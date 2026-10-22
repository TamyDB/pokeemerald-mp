#include <stdlib.h>
#include <stdio.h>
#include "global.h"
#include "net/net_client.h"
#include "net/enet/enet.h"

#define NET_CONNECT_TIMEOUT 10000
#define NET_EVENT_BUDGET 64

static ENetHost *sHost;
static ENetPeer *sPeer;
static u8 sReady;
static u32 sPlayerId;
static u32 sSequence;
static u32 sLastPosition;
static u32 sConnectStarted;
static u32 sMembers[NET_MAX_PLAYERS];
static u8 sMemberActive[NET_MAX_PLAYERS];

static u8 FindMember(u32 playerId)
{
    u8 i;
    for (i = 0; i < NET_MAX_PLAYERS; i++)
        if (sMemberActive[i] && sMembers[i] == playerId)
            return i;
    return NET_MAX_PLAYERS;
}

static void SendPacket(u8 channel, const void *data, size_t size, u32 flags)
{
    ENetPacket *packet = enet_packet_create(data, size, flags);
    if (packet != NULL && enet_peer_send(sPeer, channel, packet) < 0)
        enet_packet_destroy(packet);
}

u8 InitNetworkClient(const char *serverAddress, u16 port)
{
    ENetAddress address;

    ShutdownNetworkClient();
    if (serverAddress == NULL || port == 0 || enet_initialize() != 0)
        return FALSE;
    address.port = port;
    if (enet_address_set_host_ip(&address, serverAddress) != 0)
    {
        enet_deinitialize();
        return FALSE;
    }
    sHost = enet_host_create(NULL, 1, 2, 0, 0);
    if (sHost == NULL)
    {
        enet_deinitialize();
        return FALSE;
    }
    sHost->maximumPacketSize = 1024;
    sHost->maximumWaitingData = 64 * 1024;
    sPeer = enet_host_connect(sHost, &address, 2, NET_PROTOCOL_VERSION);
    if (sPeer == NULL)
    {
        ShutdownNetworkClient();
        return FALSE;
    }
    fprintf(stderr, "NET: connecting to %s:%u\n", serverAddress, port);
    sSequence = 0;
    sConnectStarted = enet_time_get();
    return TRUE; // Connection and application handshake complete asynchronously.
}

// Environment is populated by the launcher from an ignored local configuration.
u8 InitNetworkClientFromEnvironment(void)
{
    const char *address = getenv("POKE_NET_SERVER");
    const char *portText = getenv("POKE_NET_PORT");
    char *end;
    unsigned long port;
    if (address == NULL && portText == NULL)
        return FALSE; // Offline launch.
    if (address == NULL || *address == '\0' || portText == NULL || *portText == '\0'
     || strspn(portText, "0123456789") != strlen(portText))
    {
        fprintf(stderr, "NET: configure POKE_NET_SERVER and POKE_NET_PORT locally\n");
        return FALSE;
    }
    port = strtoul(portText, &end, 10);
    if (*end != '\0' || port == 0 || port > 65535)
    {
        fprintf(stderr, "NET: invalid port (expected 1-65535)\n");
        return FALSE;
    }
    return InitNetworkClient(address, (u16)port);
}

static void ReceivePacket(const ENetEvent *event)
{
    const ENetPacket *packet = event->packet;
    u8 type;

    if (packet->dataLength == 0)
        return;
    type = packet->data[0];
    if (type == PACKET_S2C_WELCOME && !sReady
     && event->channelID == NET_CHANNEL_RELIABLE
     && packet->dataLength == sizeof(NetWelcomePacket))
    {
        NetWelcomePacket welcome;
        memcpy(&welcome, packet->data, sizeof(welcome));
        if (ENET_NET_TO_HOST_16(welcome.version) != NET_PROTOCOL_VERSION)
        {
            enet_peer_disconnect_now(sPeer, 0);
            return;
        }
        sPlayerId = ENET_NET_TO_HOST_32(welcome.playerId);
        sReady = TRUE;
        fprintf(stderr, "NET: handshake complete, player=%u protocol=%u\n", sPlayerId, NET_PROTOCOL_VERSION);
        sLastPosition = enet_time_get() - NET_POSITION_INTERVAL;
    }
    else if (sReady && type == PACKET_S2C_POSITION
          && event->channelID == NET_CHANNEL_UNRELIABLE
          && packet->dataLength == sizeof(NetRemotePositionPacket))
    {
        NetRemotePositionPacket remote;
        memcpy(&remote, packet->data, sizeof(remote));
        remote.playerId = ENET_NET_TO_HOST_32(remote.playerId);
        remote.position.sequenceId = ENET_NET_TO_HOST_32(remote.position.sequenceId);
        remote.position.x = (s16)ENET_NET_TO_HOST_16((u16)remote.position.x);
        remote.position.y = (s16)ENET_NET_TO_HOST_16((u16)remote.position.y);
        if (remote.playerId != sPlayerId && FindMember(remote.playerId) < NET_MAX_PLAYERS
         && remote.position.type == PACKET_C2S_POSITION
         && remote.position.facingDirection >= 1 && remote.position.facingDirection <= 4
         && remote.position.elevation <= 15 && remote.position.animState <= 1
         && remote.position.x >= 0 && remote.position.x <= 32760
         && remote.position.y >= 0 && remote.position.y <= 32760)
            NetSyncPosition(remote.playerId, &remote.position);
    }
    else if (sReady && type == PACKET_S2C_JOIN
          && event->channelID == NET_CHANNEL_RELIABLE
          && packet->dataLength == sizeof(NetJoinPacket))
    {
        NetJoinPacket join;
        u32 id;
        u8 i;
        memcpy(&join, packet->data, sizeof(join));
        id = ENET_NET_TO_HOST_32(join.playerId);
        if (id != sPlayerId && FindMember(id) == NET_MAX_PLAYERS)
            for (i = 0; i < NET_MAX_PLAYERS; i++)
                if (!sMemberActive[i])
                {
                    sMemberActive[i] = TRUE;
                    sMembers[i] = id;
                    break;
                }
    }
    else if (sReady && type == PACKET_S2C_LEAVE
          && event->channelID == NET_CHANNEL_RELIABLE
          && packet->dataLength == sizeof(NetLeavePacket))
    {
        NetLeavePacket leave;
        memcpy(&leave, packet->data, sizeof(leave));
        {
            u32 id = ENET_NET_TO_HOST_32(leave.playerId);
            u8 slot = FindMember(id);
            if (slot < NET_MAX_PLAYERS)
            {
                sMemberActive[slot] = FALSE;
                NetSyncRemovePlayer(id);
            }
        }
    }
}

void UpdateNetworkClient(void)
{
    ENetEvent event;
    unsigned int count;
    int result;

    if (sHost == NULL)
        return;
    for (count = 0; count < NET_EVENT_BUDGET; count++)
    {
        result = enet_host_service(sHost, &event, 0);
        if (result < 0)
        {
            ShutdownNetworkClient();
            return;
        }
        if (result == 0)
            break;
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
        {
            NetHelloPacket hello;
            hello.type = PACKET_C2S_HELLO;
            hello.version = ENET_HOST_TO_NET_16(NET_PROTOCOL_VERSION);
            SendPacket(NET_CHANNEL_RELIABLE, &hello, sizeof(hello), ENET_PACKET_FLAG_RELIABLE);
            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
            ReceivePacket(&event);
            enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            fprintf(stderr, "NET: disconnected\n");
            ShutdownNetworkClient();
            return;
        default:
            break;
        }
    }
    if (!sReady && (u32)(enet_time_get() - sConnectStarted) >= NET_CONNECT_TIMEOUT)
    {
        fprintf(stderr, "NET: connection/handshake timed out\n");
        ShutdownNetworkClient();
        return;
    }
    NetSyncUpdate();
    if (sReady && (u32)(enet_time_get() - sLastPosition) >= NET_POSITION_INTERVAL)
        SendPlayerPosition();
    enet_host_flush(sHost);
}

void SendPlayerPosition(void)
{
    PositionPacket position;
    struct ObjectEvent *player;

    if (!sReady || gSaveBlock1Ptr == NULL || gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT)
        return;
    player = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (!player->active)
        return;
    position.type = PACKET_C2S_POSITION;
    position.sequenceId = ENET_HOST_TO_NET_32(++sSequence);
    position.mapGroup = gSaveBlock1Ptr->location.mapGroup;
    position.mapNum = gSaveBlock1Ptr->location.mapNum;
    position.x = (s16)ENET_HOST_TO_NET_16((u16)gSaveBlock1Ptr->pos.x);
    position.y = (s16)ENET_HOST_TO_NET_16((u16)gSaveBlock1Ptr->pos.y);
    position.elevation = player->currentElevation;
    position.facingDirection = player->facingDirection;
    position.animState = player->singleMovementActive;
    SendPacket(NET_CHANNEL_UNRELIABLE, &position, sizeof(position), 0);
    sLastPosition = enet_time_get();
}

void ShutdownNetworkClient(void)
{
    if (sPeer != NULL)
        enet_peer_disconnect_now(sPeer, 0);
    if (sHost != NULL)
    {
        enet_host_destroy(sHost);
        enet_deinitialize();
    }
    sHost = NULL;
    sPeer = NULL;
    sReady = FALSE;
    memset(sMemberActive, 0, sizeof(sMemberActive));
    NetSyncReset();
}
