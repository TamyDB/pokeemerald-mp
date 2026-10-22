// Deterministic ENet boundary test: no socket, clock delays or game rendering.
#include <assert.h>
#include <stdlib.h>
#include "global.h"
#include "net/net_client.h"
#include "net/enet/enet.h"

struct ObjectEvent gObjectEvents[OBJECT_EVENTS_COUNT];
struct PlayerAvatar gPlayerAvatar;
static struct SaveBlock1 sSave;
struct SaveBlock1 *gSaveBlock1Ptr = &sSave;
static ENetHost sHost;
static ENetPeer sPeer;
static ENetEvent sEvent;
static u32 sNow, sSent, sDestroyed, sSynced, sRemoved, sShutdown;
static u8 sChannel;
static u32 sFlags;
static unsigned char sData[128];
static size_t sSize;
static PositionPacket sPosition;
static int sServiceError;
static unsigned int sQueued;

int enet_initialize(void) { return 0; }
void enet_deinitialize(void) { sShutdown++; }
u32 enet_time_get(void) { return sNow; }
int enet_address_set_host_ip(ENetAddress *address, const char *name)
{
    (void)address;
    return strcmp(name, "127.0.0.1") == 0 ? 0 : -1;
}
ENetHost *enet_host_create(const ENetAddress *address, size_t peers, size_t channels, u32 incoming, u32 outgoing)
{
    assert(address == NULL && peers == 1 && channels == 2);
    (void)incoming; (void)outgoing;
    return &sHost;
}
void enet_host_destroy(ENetHost *host) { assert(host == &sHost); }
ENetPeer *enet_host_connect(ENetHost *host, const ENetAddress *address, size_t channels, u32 version)
{
    assert(host == &sHost && address->port == 25001 && channels == 2 && version == NET_PROTOCOL_VERSION);
    return &sPeer;
}
void enet_host_flush(ENetHost *host) { assert(host == &sHost); }
void enet_peer_disconnect_now(ENetPeer *peer, u32 data) { assert(peer == &sPeer); (void)data; }
ENetPacket *enet_packet_create(const void *data, size_t size, u32 flags)
{
    ENetPacket *packet = calloc(1, sizeof(*packet));
    assert(packet != NULL);
    packet->data = malloc(size ? size : 1);
    assert(packet->data != NULL);
    memcpy(packet->data, data, size);
    packet->dataLength = size;
    packet->flags = flags;
    return packet;
}
void enet_packet_destroy(ENetPacket *packet)
{
    free(packet->data);
    free(packet);
    sDestroyed++;
}
int enet_peer_send(ENetPeer *peer, u8 channel, ENetPacket *packet)
{
    assert(peer == &sPeer && packet->dataLength <= sizeof(sData));
    sChannel = channel;
    sFlags = packet->flags;
    sSize = packet->dataLength;
    memcpy(sData, packet->data, sSize);
    sSent++;
    enet_packet_destroy(packet); // Model ENet owning successfully queued packets.
    return 0;
}
int enet_host_service(ENetHost *host, ENetEvent *event, u32 timeout)
{
    assert(host == &sHost && timeout == 0);
    if (sServiceError) return -1;
    if (!sQueued) return 0;
    *event = sEvent;
    sQueued--;
    return 1;
}
void NetSyncReset(void) {}
void NetSyncUpdate(void) {}
void NetSyncPosition(u32 id, const PositionPacket *position)
{
    assert(id == 42);
    sPosition = *position;
    sSynced++;
}
void NetSyncRemovePlayer(u32 id) { assert(id == 42); sRemoved++; }

static void Receive(u8 channel, const void *data, size_t size)
{
    sEvent.type = ENET_EVENT_TYPE_RECEIVE;
    sEvent.channelID = channel;
    sEvent.packet = enet_packet_create(data, size, 0);
    sQueued = 1;
    UpdateNetworkClient();
}

int main(void)
{
    NetWelcomePacket welcome = {PACKET_S2C_WELCOME, ENET_HOST_TO_NET_16(NET_PROTOCOL_VERSION), ENET_HOST_TO_NET_32(7)};
    NetRemotePositionPacket remote = {0};
    NetLeavePacket leave = {PACKET_S2C_LEAVE, ENET_HOST_TO_NET_32(42)};
    NetJoinPacket join = {PACKET_S2C_JOIN, ENET_HOST_TO_NET_32(42)};
    PositionPacket sent;
    u32 count;

    assert(sizeof(PositionPacket) == 14 && sizeof(NetRemotePositionPacket) == 19);
    assert(!InitNetworkClient(NULL, 25001));
    assert(!InitNetworkClient("hostname", 25001));
    assert(!InitNetworkClient("127.0.0.1", 0));
    assert(InitNetworkClient("127.0.0.1", 25001));
    sSave.pos.x = 1234;
    sSave.pos.y = 321;
    sSave.location.mapGroup = 1;
    sSave.location.mapNum = 2;
    gObjectEvents[0].active = TRUE;
    gObjectEvents[0].facingDirection = 3;
    SendPlayerPosition();
    assert(sSent == 0); // No positions before application handshake.
    sEvent.type = ENET_EVENT_TYPE_CONNECT;
    sQueued = 1;
    UpdateNetworkClient();
    assert(sSent == 1 && sChannel == 1 && sFlags == ENET_PACKET_FLAG_RELIABLE);
    assert(sSize == sizeof(NetHelloPacket) && sData[0] == PACKET_C2S_HELLO);
    Receive(0, &welcome, sizeof(welcome));
    assert(sSent == 1); // Wrong channel cannot complete handshake.
    Receive(1, &welcome, sizeof(welcome));
    assert(sSent == 2 && sChannel == 0 && sFlags == 0);
    memcpy(&sent, sData, sizeof(sent));
    assert(ENET_NET_TO_HOST_16((u16)sent.x) == 1234);
    assert(ENET_NET_TO_HOST_16((u16)sent.y) == 321);
    assert(ENET_NET_TO_HOST_32(sent.sequenceId) == 1);
    sNow = 49;
    UpdateNetworkClient();
    assert(sSent == 2);
    sNow = 50;
    UpdateNetworkClient();
    assert(sSent == 3);

    remote.type = PACKET_S2C_POSITION;
    remote.playerId = ENET_HOST_TO_NET_32(42);
    remote.position.type = PACKET_C2S_POSITION;
    remote.position.sequenceId = ENET_HOST_TO_NET_32(123);
    remote.position.x = (s16)ENET_HOST_TO_NET_16(1024);
    remote.position.y = (s16)ENET_HOST_TO_NET_16(256);
    remote.position.facingDirection = 2;
    Receive(1, &remote, sizeof(remote));
    Receive(0, &remote, sizeof(remote) - 1);
    assert(sSynced == 0);
    Receive(0, &remote, sizeof(remote));
    assert(sSynced == 0); // Positions cannot introduce membership.
    Receive(1, &join, sizeof(join));
    Receive(0, &remote, sizeof(remote));
    assert(sSynced == 1 && sPosition.x == 1024 && sPosition.y == 256 && sPosition.sequenceId == 123);
    remote.position.facingDirection = 255;
    Receive(0, &remote, sizeof(remote));
    assert(sSynced == 1);
    remote.position.facingDirection = 2;
    remote.playerId = welcome.playerId;
    Receive(0, &remote, sizeof(remote));
    assert(sSynced == 1); // Do not render our own echoed position.
    count = sDestroyed;
    Receive(0, &remote, 0);
    assert(sDestroyed == count + 1);
    Receive(0, &leave, sizeof(leave));
    assert(sRemoved == 0);
    Receive(1, &leave, sizeof(leave));
    assert(sRemoved == 1);
    remote.playerId = ENET_HOST_TO_NET_32(42);
    Receive(0, &remote, sizeof(remote));
    assert(sSynced == 1); // Delayed UDP must not resurrect a departed peer.
    sEvent.type = ENET_EVENT_TYPE_DISCONNECT;
    sQueued = 1;
    UpdateNetworkClient();
    count = sSent;
    SendPlayerPosition();
    assert(sSent == count);
    ShutdownNetworkClient(); // Safe to call twice.

    assert(InitNetworkClient("127.0.0.1", 25001));
    count = sShutdown;
    sNow += 10000;
    UpdateNetworkClient();
    assert(sShutdown == count + 1);
    assert(InitNetworkClient("127.0.0.1", 25001));
    count = sShutdown;
    sServiceError = 1;
    UpdateNetworkClient();
    assert(sShutdown == count + 1);
    sServiceError = 0;
    unsetenv("POKE_NET_SERVER");
    unsetenv("POKE_NET_PORT");
    assert(!InitNetworkClientFromEnvironment());
    setenv("POKE_NET_SERVER", "127.0.0.1", 1);
    assert(!InitNetworkClientFromEnvironment());
    setenv("POKE_NET_PORT", "65536", 1);
    assert(!InitNetworkClientFromEnvironment());
    setenv("POKE_NET_PORT", "0", 1);
    assert(!InitNetworkClientFromEnvironment());
    setenv("POKE_NET_PORT", "-1", 1);
    assert(!InitNetworkClientFromEnvironment());
    setenv("POKE_NET_PORT", "25001", 1);
    assert(InitNetworkClientFromEnvironment());
    ShutdownNetworkClient();
    unsetenv("POKE_NET_SERVER");
    unsetenv("POKE_NET_PORT");
    return 0;
}
