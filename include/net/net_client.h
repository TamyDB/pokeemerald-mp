#ifndef GUARD_NET_CLIENT_H
#define GUARD_NET_CLIENT_H

#include "gba/types.h"

#define NET_CHANNEL_UNRELIABLE 0
#define NET_CHANNEL_RELIABLE   1
#define NET_PROTOCOL_VERSION   2
#define NET_MAX_PLAYERS        8
#define NET_POSITION_INTERVAL  50 // milliseconds, 20 Hz

enum NetPacketType
{
    PACKET_C2S_HELLO = 1,
    PACKET_S2C_WELCOME,
    PACKET_C2S_POSITION,
    PACKET_S2C_POSITION,
    PACKET_S2C_LEAVE,
    PACKET_S2C_JOIN,
};

// All multibyte fields on the wire use network byte order.
typedef struct __attribute__((packed))
{
    u8 type;
    u32 sequenceId;
    u8 mapGroup;
    u8 mapNum;
    s16 x;
    s16 y;
    u8 elevation;
    u8 facingDirection;
    u8 animState; // 0: idle, 1: moving (reserved for interpolation)
} PositionPacket;

typedef struct __attribute__((packed))
{
    u8 type;
    u16 version;
} NetHelloPacket;

typedef struct __attribute__((packed))
{
    u8 type;
    u16 version;
    u32 playerId;
} NetWelcomePacket;

typedef struct __attribute__((packed))
{
    u8 type;
    u32 playerId;
    PositionPacket position;
} NetRemotePositionPacket;

typedef struct __attribute__((packed))
{
    u8 type;
    u32 playerId;
} NetLeavePacket;

// JOIN and LEAVE share the same layout; reliable membership gates positions.
typedef NetLeavePacket NetJoinPacket;

// Numeric IPv4 only: avoids blocking DNS. Call from the game thread.
u8 InitNetworkClient(const char *serverAddress, u16 port);
void UpdateNetworkClient(void);
void SendPlayerPosition(void);
void ShutdownNetworkClient(void);

// Internal sync interface; positions here are in host byte order.
void NetSyncPosition(u32 playerId, const PositionPacket *position);
void NetSyncRemovePlayer(u32 playerId);
void NetSyncReset(void);
void NetSyncUpdate(void);

#endif
