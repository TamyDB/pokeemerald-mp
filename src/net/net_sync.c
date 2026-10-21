#include "global.h"
#include "fieldmap.h"
#include "event_object_movement.h"
#include "net/net_client.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"

// Reserve local IDs outside the range used by ordinary map objects.
#define NET_LOCAL_ID_BASE 240

struct RemotePlayer
{
    u8 active;
    u8 objectEventId;
    u32 playerId;
    PositionPacket position;
};

static struct RemotePlayer sPlayers[NET_MAX_PLAYERS];

static u8 OwnsObject(unsigned int slot)
{
    struct RemotePlayer *remote = &sPlayers[slot];
    struct ObjectEvent *object;
    if (!remote->active || remote->objectEventId >= OBJECT_EVENTS_COUNT)
        return FALSE;
    object = &gObjectEvents[remote->objectEventId];
    return object->active && !object->isPlayer
        && object->localId == NET_LOCAL_ID_BASE + slot
        && object->mapGroup == remote->position.mapGroup
        && object->mapNum == remote->position.mapNum;
}

static void RemoveSprite(unsigned int slot)
{
    if (OwnsObject(slot))
        RemoveNetworkObjectEvent(sPlayers[slot].objectEventId);
    sPlayers[slot].objectEventId = OBJECT_EVENTS_COUNT;
}

void NetSyncReset(void)
{
    unsigned int i;
    for (i = 0; i < NET_MAX_PLAYERS; i++)
    {
        RemoveSprite(i);
        sPlayers[i].active = FALSE;
    }
}

void NetSyncRemovePlayer(u32 playerId)
{
    unsigned int i;
    for (i = 0; i < NET_MAX_PLAYERS; i++)
    {
        if (sPlayers[i].active && sPlayers[i].playerId == playerId)
        {
            RemoveSprite(i);
            sPlayers[i].active = FALSE;
        }
    }
}

void NetSyncPosition(u32 playerId, const PositionPacket *position)
{
    unsigned int i, freeSlot = NET_MAX_PLAYERS;
    struct RemotePlayer *remote;
    for (i = 0; i < NET_MAX_PLAYERS; i++)
    {
        if (!sPlayers[i].active)
            freeSlot = i;
        else if (sPlayers[i].playerId == playerId)
            break;
    }
    if (i == NET_MAX_PLAYERS)
    {
        if (freeSlot == NET_MAX_PLAYERS)
            return;
        i = freeSlot;
        sPlayers[i].objectEventId = OBJECT_EVENTS_COUNT;
    }
    remote = &sPlayers[i];
    // Serial arithmetic accepts rollover and rejects duplicates/older packets.
    if (remote->active)
    {
        u32 delta = position->sequenceId - remote->position.sequenceId;
        if (delta == 0 || delta >= 0x80000000u)
            return;
        if (position->mapGroup != remote->position.mapGroup || position->mapNum != remote->position.mapNum)
            RemoveSprite(i);
    }
    remote->active = TRUE;
    remote->playerId = playerId;
    remote->position = *position;
}

void NetSyncUpdate(void)
{
    unsigned int i;
    if (gSaveBlock1Ptr == NULL)
        return;
    for (i = 0; i < NET_MAX_PLAYERS; i++)
    {
        struct RemotePlayer *remote = &sPlayers[i];
        struct ObjectEvent *object;
        u8 existing;
        if (!remote->active)
            continue;
        if (remote->position.mapGroup != gSaveBlock1Ptr->location.mapGroup
         || remote->position.mapNum != gSaveBlock1Ptr->location.mapNum)
        {
            RemoveSprite(i);
            continue;
        }
        if (!OwnsObject(i))
        {
            remote->objectEventId = OBJECT_EVENTS_COUNT;
            // Never claim a map object that already uses the reserved ID.
            if (!TryGetObjectEventIdByLocalIdAndMap(NET_LOCAL_ID_BASE + i,
                    remote->position.mapNum, remote->position.mapGroup, &existing))
                continue;
            remote->objectEventId = SpawnSpecialObjectEventParameterized(
                OBJ_EVENT_GFX_BRENDAN_NORMAL, MOVEMENT_TYPE_NONE, NET_LOCAL_ID_BASE + i,
                remote->position.x + MAP_OFFSET, remote->position.y + MAP_OFFSET,
                remote->position.elevation);
            if (!OwnsObject(i))
                continue; // Retry next frame if the Object Event pool is full.
        }
        object = &gObjectEvents[remote->objectEventId];
        object->currentElevation = remote->position.elevation;
        object->previousElevation = remote->position.elevation;
        if (object->currentCoords.x != remote->position.x + MAP_OFFSET
         || object->currentCoords.y != remote->position.y + MAP_OFFSET)
            MoveObjectEventToMapCoords(object, remote->position.x + MAP_OFFSET, remote->position.y + MAP_OFFSET);
        ObjectEventTurn(object, remote->position.facingDirection);
    }
}
