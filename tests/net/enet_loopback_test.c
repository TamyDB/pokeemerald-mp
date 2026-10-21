// Exercise the vendored transport with real loopback UDP on an ephemeral port.
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "net/enet/enet.h"

int main(void)
{
    ENetAddress address = {ENET_HOST_ANY, 0};
    ENetHost *server, *client;
    ENetPeer *peer;
    ENetEvent event;
    enet_uint32 start;
    unsigned int received = 0;
    unsigned int connected = 0;
    const char *messages[2] = {"position", "hello"};

    assert(enet_initialize() == 0);
    server = enet_host_create(&address, 1, 2, 0, 0);
    if (server == NULL)
    {
        fprintf(stderr, "Could not bind loopback test UDP socket\n");
        enet_deinitialize();
        return 1;
    }
    assert(enet_socket_get_address(server->socket, &address) == 0);
    assert(enet_address_set_host_ip(&address, "127.0.0.1") == 0);
    client = enet_host_create(NULL, 1, 2, 0, 0);
    assert(client != NULL);
    peer = enet_host_connect(client, &address, 2, 1);
    assert(peer != NULL);
    start = enet_time_get();
    while (received != 3 && (enet_uint32)(enet_time_get() - start) < 5000)
    {
        int result = enet_host_service(client, &event, 0);
        assert(result >= 0);
        if (result > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
        {
            unsigned int i;
            connected++;
            for (i = 0; i < 2; i++)
            {
                ENetPacket *packet = enet_packet_create(messages[i], strlen(messages[i]) + 1,
                    i == 1 ? ENET_PACKET_FLAG_RELIABLE : 0);
                assert(packet != NULL && enet_peer_send(peer, i, packet) == 0);
            }
        }
        else if (result > 0 && event.type == ENET_EVENT_TYPE_RECEIVE)
            enet_packet_destroy(event.packet);
        result = enet_host_service(server, &event, 0);
        assert(result >= 0);
        if (result > 0 && event.type == ENET_EVENT_TYPE_RECEIVE)
        {
            assert(event.channelID < 2);
            assert(event.packet->dataLength == strlen(messages[event.channelID]) + 1);
            assert(memcmp(event.packet->data, messages[event.channelID], event.packet->dataLength) == 0);
            if (event.channelID == 1)
                assert(event.packet->flags & ENET_PACKET_FLAG_RELIABLE);
            received |= 1u << event.channelID;
            enet_packet_destroy(event.packet);
        }
    }
    assert(connected == 1 && received == 3);
    enet_peer_disconnect_now(peer, 0);
    enet_host_destroy(client);
    enet_host_destroy(server);
    enet_deinitialize();
    puts("ENet loopback: both channels passed");
    return 0;
}
