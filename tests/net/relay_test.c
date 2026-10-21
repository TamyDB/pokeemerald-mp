#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "global.h"
#include "net/net_client.h"
#include "net/enet/enet.h"

struct ObjectEvent gObjectEvents[OBJECT_EVENTS_COUNT];
struct PlayerAvatar gPlayerAvatar;
static struct SaveBlock1 sSave;
struct SaveBlock1 *gSaveBlock1Ptr = &sSave;
static u32 sRemoteId, sReceived, sLeft;
static PositionPacket sLast;

void NetSyncReset(void) {}
void NetSyncUpdate(void) {}
void NetSyncPosition(u32 id, const PositionPacket *position)
{
    sRemoteId = id;
    sLast = *position;
    sReceived++;
}
void NetSyncRemovePlayer(u32 id) { if (id == sRemoteId) sLeft++; }

struct Probe
{
    ENetHost *host;
    ENetPeer *peer;
    u32 id, joinId, positionId;
    PositionPacket position;
    unsigned int positions;
};

static void Start(struct Probe *probe, const ENetAddress *address)
{
    memset(probe, 0, sizeof(*probe));
    probe->host = enet_host_create(NULL, 1, 2, 0, 0);
    assert(probe->host != NULL);
    probe->peer = enet_host_connect(probe->host, address, 2, NET_PROTOCOL_VERSION);
    assert(probe->peer != NULL);
}

static void Poll(struct Probe *probe)
{
    ENetEvent event;
    int result;
    while ((result = enet_host_service(probe->host, &event, 0)) > 0)
    {
        if (event.type == ENET_EVENT_TYPE_CONNECT)
        {
            NetHelloPacket hello = {PACKET_C2S_HELLO, ENET_HOST_TO_NET_16(NET_PROTOCOL_VERSION)};
            ENetPacket *packet = enet_packet_create(&hello, sizeof(hello), ENET_PACKET_FLAG_RELIABLE);
            assert(packet != NULL && enet_peer_send(probe->peer, 1, packet) == 0);
        }
        else if (event.type == ENET_EVENT_TYPE_RECEIVE)
        {
            u8 type = event.packet->data[0];
            if (type == PACKET_S2C_WELCOME)
            {
                NetWelcomePacket welcome;
                assert(event.channelID == 1 && event.packet->dataLength == sizeof(welcome));
                memcpy(&welcome, event.packet->data, sizeof(welcome));
                assert(ENET_NET_TO_HOST_16(welcome.version) == NET_PROTOCOL_VERSION);
                probe->id = ENET_NET_TO_HOST_32(welcome.playerId);
            }
            else if (type == PACKET_S2C_JOIN)
            {
                NetJoinPacket join;
                assert(event.channelID == 1 && event.packet->dataLength == sizeof(join));
                memcpy(&join, event.packet->data, sizeof(join));
                probe->joinId = ENET_NET_TO_HOST_32(join.playerId);
            }
            else if (type == PACKET_S2C_POSITION)
            {
                NetRemotePositionPacket remote;
                assert(event.channelID == 0 && event.packet->dataLength == sizeof(remote));
                memcpy(&remote, event.packet->data, sizeof(remote));
                probe->positionId = ENET_NET_TO_HOST_32(remote.playerId);
                probe->position = remote.position;
                probe->positions++;
            }
            enet_packet_destroy(event.packet);
        }
    }
    assert(result >= 0);
    enet_host_flush(probe->host);
}

static void Pump(struct Probe *a, struct Probe *b, u32 duration)
{
    u32 start = enet_time_get();
    const struct timespec pause = {0, 1000000};
    do
    {
        UpdateNetworkClient();
        if (a != NULL) Poll(a);
        if (b != NULL) Poll(b);
        nanosleep(&pause, NULL);
    } while ((u32)(enet_time_get() - start) < duration);
}

static void SendPosition(struct Probe *probe, u32 sequence, s16 x, u8 channel)
{
    PositionPacket position = {PACKET_C2S_POSITION, ENET_HOST_TO_NET_32(sequence), 1, 2,
        (s16)ENET_HOST_TO_NET_16((u16)x), (s16)ENET_HOST_TO_NET_16(21), 3, 2, 0};
    ENetPacket *packet = enet_packet_create(&position, sizeof(position), channel == 1 ? ENET_PACKET_FLAG_RELIABLE : 0);
    assert(packet != NULL && enet_peer_send(probe->peer, channel, packet) == 0);
    enet_host_flush(probe->host);
}

int main(int argc, char **argv)
{
    ENetAddress address;
    struct Probe a, b;
    u32 oldId, start;
    assert(argc == 3);
    address.port = (u16)strtoul(argv[2], NULL, 10);
    assert(enet_initialize() == 0 && enet_address_set_host_ip(&address, argv[1]) == 0);
    sSave.location.mapGroup = 1;
    sSave.location.mapNum = 2;
    sSave.pos.x = 10;
    sSave.pos.y = 20;
    gObjectEvents[0].active = TRUE;
    gObjectEvents[0].facingDirection = 1;
    assert(InitNetworkClient(argv[1], address.port));
    Start(&a, &address);
    start = enet_time_get();
    do { Pump(&a, NULL, 10); } while ((!a.id || !a.positions || !a.joinId) && (u32)(enet_time_get() - start) < 10000);
    assert(a.id != 0 && a.joinId != 0 && a.positionId != a.id);
    assert(ENET_NET_TO_HOST_16((u16)a.position.x) == 10); // Actual game's C2S serializer -> relay.
    SendPosition(&a, 100, 12, 0);
    start = enet_time_get();
    do { Pump(&a, NULL, 10); } while (!sReceived && (u32)(enet_time_get() - start) < 5000);
    assert(sReceived && sRemoteId == a.id && sLast.x == 12 && sLast.sequenceId == 100);
    SendPosition(&a, 99, 99, 0);
    SendPosition(&a, 101, 88, 1); // Correct packet, wrong channel.
    SendPosition(&a, 101, -1, 0); // Malformed coordinate.
    Pump(&a, NULL, 300);
    assert(sLast.sequenceId == 100 && sLast.x == 12);
    // Late joiner receives cached positions and reliable membership.
    Start(&b, &address);
    start = enet_time_get();
    do { Pump(&a, &b, 10); } while ((!b.id || !b.positions || !b.joinId) && (u32)(enet_time_get() - start) < 10000);
    assert(b.id != 0 && b.id != a.id && b.positions && b.joinId);
    oldId = a.id;
    enet_peer_disconnect(a.peer, 0);
    start = enet_time_get();
    do { Pump(&a, &b, 10); } while (!sLeft && (u32)(enet_time_get() - start) < 5000);
    assert(sLeft == 1);
    enet_host_destroy(a.host);
    Start(&a, &address);
    start = enet_time_get();
    do { Pump(&a, &b, 10); } while (!a.id && (u32)(enet_time_get() - start) < 10000);
    assert(a.id && a.id != oldId);
    SendPosition(&a, 1, 14, 0);
    start = enet_time_get();
    do { Pump(&a, &b, 10); } while (sRemoteId != a.id && (u32)(enet_time_get() - start) < 5000);
    assert(sRemoteId == a.id && sLast.x == 14);
    ShutdownNetworkClient();
    enet_peer_disconnect_now(a.peer, 0);
    enet_peer_disconnect_now(b.peer, 0);
    enet_host_destroy(a.host);
    enet_host_destroy(b.host);
    enet_deinitialize();
    puts("Relay integration passed: game client, membership, positions, stale/invalid packets, late join, leave, reconnect");
    return 0;
}
