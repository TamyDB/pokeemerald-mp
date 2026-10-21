// Minimal overworld relay. No game state, ROM, saves or gameplay authority.
#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "net/net_client.h"
#include "net/enet/enet.h"

struct Client
{
    ENetPeer *peer;
    u32 id, connectedAt, rateStarted, sequence;
    unsigned int positions;
    u8 ready, hasPosition;
    PositionPacket position; // Keep wire representation for forwarding.
};

static struct Client sClients[NET_MAX_PLAYERS];
static u32 sNextId = 1;
static volatile sig_atomic_t sRunning = 1;

static void Stop(int sig) { (void)sig; sRunning = 0; }

static int Send(ENetPeer *peer, u8 channel, const void *data, size_t size)
{
    ENetPacket *packet = enet_packet_create(data, size,
        channel == NET_CHANNEL_RELIABLE ? ENET_PACKET_FLAG_RELIABLE : 0);
    if (packet == NULL) return -1;
    if (enet_peer_send(peer, channel, packet) != 0)
    {
        enet_packet_destroy(packet);
        return -1;
    }
    return 0;
}

static void Membership(ENetPeer *peer, u8 type, u32 id)
{
    NetLeavePacket message;
    message.type = type;
    message.playerId = ENET_HOST_TO_NET_32(id);
    // A control queue failure invalidates the peer's membership view.
    if (Send(peer, NET_CHANNEL_RELIABLE, &message, sizeof(message)) != 0)
        enet_peer_disconnect_later(peer, 1);
}

static void Position(ENetPeer *peer, const struct Client *source)
{
    NetRemotePositionPacket message;
    message.type = PACKET_S2C_POSITION;
    message.playerId = ENET_HOST_TO_NET_32(source->id);
    message.position = source->position;
    Send(peer, NET_CHANNEL_UNRELIABLE, &message, sizeof(message));
}

static void Drop(struct Client *client)
{
    unsigned int i;
    if (client->ready)
    {
        for (i = 0; i < NET_MAX_PLAYERS; i++)
            if (sClients[i].ready && &sClients[i] != client)
                Membership(sClients[i].peer, PACKET_S2C_LEAVE, client->id);
        printf("leave id=%u\n", client->id);
    }
    if (client->peer != NULL)
    {
        client->peer->data = NULL;
        enet_peer_disconnect_now(client->peer, 0);
    }
    memset(client, 0, sizeof(*client));
}

static void Receive(struct Client *client, const ENetEvent *event)
{
    unsigned int i;
    ENetPacket *packet = event->packet;
    if (!client->ready)
    {
        NetHelloPacket hello;
        NetWelcomePacket welcome;
        if (event->channelID != NET_CHANNEL_RELIABLE || packet->dataLength != sizeof(hello))
            return;
        memcpy(&hello, packet->data, sizeof(hello));
        if (hello.type != PACKET_C2S_HELLO || ENET_NET_TO_HOST_16(hello.version) != NET_PROTOCOL_VERSION)
        {
            Drop(client);
            return;
        }
        welcome.type = PACKET_S2C_WELCOME;
        welcome.version = ENET_HOST_TO_NET_16(NET_PROTOCOL_VERSION);
        welcome.playerId = ENET_HOST_TO_NET_32(client->id);
        if (Send(client->peer, NET_CHANNEL_RELIABLE, &welcome, sizeof(welcome)) != 0)
        {
            Drop(client);
            return;
        }
        client->ready = 1;
        for (i = 0; i < NET_MAX_PLAYERS; i++)
            if (sClients[i].ready && &sClients[i] != client)
            {
                Membership(client->peer, PACKET_S2C_JOIN, sClients[i].id);
                Membership(sClients[i].peer, PACKET_S2C_JOIN, client->id);
                if (sClients[i].hasPosition)
                    Position(client->peer, &sClients[i]);
            }
        printf("join id=%u\n", client->id);
    }
    else if (event->channelID == NET_CHANNEL_UNRELIABLE && packet->dataLength == sizeof(PositionPacket))
    {
        PositionPacket position;
        u32 sequence, delta;
        s16 x, y;
        memcpy(&position, packet->data, sizeof(position));
        x = (s16)ENET_NET_TO_HOST_16((u16)position.x);
        y = (s16)ENET_NET_TO_HOST_16((u16)position.y);
        if (position.type != PACKET_C2S_POSITION || x < 0 || y < 0 || x > 32760 || y > 32760
         || position.elevation > 15 || position.facingDirection < 1 || position.facingDirection > 4
         || position.animState > 1)
            return;
        sequence = ENET_NET_TO_HOST_32(position.sequenceId);
        delta = sequence - client->sequence;
        if (client->hasPosition && (delta == 0 || delta >= 0x80000000u)) return;
        if ((u32)(enet_time_get() - client->rateStarted) >= 1000)
        {
            client->rateStarted = enet_time_get();
            client->positions = 0;
        }
        if (++client->positions > 40) return;
        client->sequence = sequence;
        client->position = position;
        client->hasPosition = 1;
        for (i = 0; i < NET_MAX_PLAYERS; i++)
            if (sClients[i].ready && &sClients[i] != client)
                Position(sClients[i].peer, client);
    }
}

int main(int argc, char **argv)
{
    ENetAddress address = {ENET_HOST_ANY, 25001};
    ENetHost *host;
    u32 lastSnapshot;
    char *end;
    long port;
    unsigned int i;
    if (argc > 2) return 2;
    if (argc == 2)
    {
        port = strtol(argv[1], &end, 10);
        if (*argv[1] == '\0' || *end != '\0' || port < 1 || port > 65535) return 2;
        address.port = port;
    }
    if (enet_initialize() != 0) return 1;
    host = enet_host_create(&address, NET_MAX_PLAYERS, 2, 0, 0);
    if (host == NULL) { fprintf(stderr, "Cannot bind UDP %u\n", address.port); enet_deinitialize(); return 1; }
    host->maximumPacketSize = 1024;
    host->maximumWaitingData = 64 * 1024;
    signal(SIGINT, Stop);
    signal(SIGTERM, Stop);
    setvbuf(stdout, NULL, _IOLBF, 0);
    printf("ENet relay v%u UDP %u capacity=%u\n", NET_PROTOCOL_VERSION, address.port, NET_MAX_PLAYERS);
    lastSnapshot = enet_time_get();
    while (sRunning)
    {
        ENetEvent event;
        unsigned int count;
        const struct timespec pause = {0, 1000000};
        for (count = 0; count < 128; count++)
        {
            int result = enet_host_service(host, &event, 0);
            struct Client *client;
            if (result < 0) { sRunning = 0; break; }
            if (result == 0) break;
            client = event.peer->data;
            if (event.type == ENET_EVENT_TYPE_CONNECT)
            {
                for (i = 0; i < NET_MAX_PLAYERS && sClients[i].peer != NULL; i++) {}
                if (i == NET_MAX_PLAYERS || event.data != NET_PROTOCOL_VERSION || sNextId == 0)
                    enet_peer_disconnect_now(event.peer, 1);
                else
                {
                    client = &sClients[i];
                    client->peer = event.peer;
                    client->id = sNextId++; // Never reuse an ID within a server session.
                    client->connectedAt = enet_time_get();
                    event.peer->data = client;
                }
            }
            else if (event.type == ENET_EVENT_TYPE_RECEIVE)
            {
                if (client != NULL) Receive(client, &event);
                enet_packet_destroy(event.packet);
            }
            else if (event.type == ENET_EVENT_TYPE_DISCONNECT && client != NULL)
                Drop(client);
        }
        for (i = 0; i < NET_MAX_PLAYERS; i++)
            if (sClients[i].peer != NULL && !sClients[i].ready
             && (u32)(enet_time_get() - sClients[i].connectedAt) >= 10000)
                Drop(&sClients[i]);
        // Retransmit current snapshots: a position may arrive before its reliable JOIN.
        if ((u32)(enet_time_get() - lastSnapshot) >= NET_POSITION_INTERVAL)
        {
            unsigned int j;
            for (i = 0; i < NET_MAX_PLAYERS; i++)
                if (sClients[i].ready && sClients[i].hasPosition)
                    for (j = 0; j < NET_MAX_PLAYERS; j++)
                        if (i != j && sClients[j].ready) Position(sClients[j].peer, &sClients[i]);
            lastSnapshot = enet_time_get();
        }
        enet_host_flush(host);
        nanosleep(&pause, NULL);
    }
    for (i = 0; i < NET_MAX_PLAYERS; i++) if (sClients[i].peer != NULL) Drop(&sClients[i]);
    enet_host_destroy(host);
    enet_deinitialize();
    return 0;
}
