#include <assert.h>
#include "global.h"
#include "net/net_client.h"
#include "event_object_movement.h"

struct ObjectEvent gObjectEvents[OBJECT_EVENTS_COUNT];
static struct SaveBlock1 sSave;
struct SaveBlock1 *gSaveBlock1Ptr = &sSave;
static unsigned int sRemoved;

void RemoveNetworkObjectEvent(u8 id)
{
    assert(id < OBJECT_EVENTS_COUNT);
    gObjectEvents[id].active = FALSE;
    sRemoved++;
}

bool8 TryGetObjectEventIdByLocalIdAndMap(u8 local, u8 map, u8 group, u8 *id)
{
    unsigned int i;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
        if (gObjectEvents[i].active && gObjectEvents[i].localId == local
         && gObjectEvents[i].mapNum == map && gObjectEvents[i].mapGroup == group)
        {
            *id = i;
            return FALSE;
        }
    return TRUE;
}

u8 SpawnSpecialObjectEventParameterized(u8 gfx, u8 movement, u8 local, s16 x, s16 y, u8 elevation)
{
    unsigned int i;
    (void)gfx;
    (void)movement;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
        if (!gObjectEvents[i].active)
        {
            struct ObjectEvent *object = &gObjectEvents[i];
            memset(object, 0, sizeof(*object));
            object->active = TRUE;
            object->localId = local;
            object->mapNum = sSave.location.mapNum;
            object->mapGroup = sSave.location.mapGroup;
            object->currentCoords.x = x;
            object->currentCoords.y = y;
            object->currentElevation = elevation;
            return i;
        }
    return OBJECT_EVENTS_COUNT;
}

void MoveObjectEventToMapCoords(struct ObjectEvent *object, s16 x, s16 y)
{
    object->currentCoords.x = x;
    object->currentCoords.y = y;
}

void ObjectEventTurn(struct ObjectEvent *object, u8 direction)
{
    object->facingDirection = direction;
}

int main(void)
{
    PositionPacket position = {PACKET_C2S_POSITION, 0xfffffffeu, 1, 2, 10, 20, 3, 1, 0};
    assert(sizeof(PositionPacket) == 14);
    sSave.location.mapGroup = 1;
    sSave.location.mapNum = 2;
    NetSyncReset();
    NetSyncPosition(42, &position);
    NetSyncUpdate();
    assert(gObjectEvents[0].active && gObjectEvents[0].currentCoords.x == 17);
    position.sequenceId = 0xfffffffdu;
    position.x = 99;
    NetSyncPosition(42, &position);
    NetSyncUpdate();
    assert(gObjectEvents[0].currentCoords.x == 17);
    position.sequenceId = 1; // Wraparound is newer.
    position.x = 11;
    NetSyncPosition(42, &position);
    NetSyncUpdate();
    assert(gObjectEvents[0].currentCoords.x == 18);
    sSave.location.mapNum = 3;
    NetSyncUpdate();
    assert(!gObjectEvents[0].active && sRemoved == 1);
    sSave.location.mapNum = 2;
    NetSyncUpdate();
    assert(gObjectEvents[0].active);
    // Map reload reuses the slot for a local NPC: never remove that NPC.
    gObjectEvents[0].localId = 1;
    NetSyncRemovePlayer(42);
    assert(gObjectEvents[0].active && sRemoved == 1);
    NetSyncReset();
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
    // Full object pool: retry when a slot becomes available.
    memset(gObjectEvents, 0xff, sizeof(gObjectEvents));
    position.sequenceId = 1;
    NetSyncPosition(50, &position);
    NetSyncUpdate();
    gObjectEvents[0].active = FALSE;
    NetSyncUpdate();
    assert(gObjectEvents[0].active && gObjectEvents[0].currentCoords.x == 18);
    // Duplicate sequence must not move the sprite.
    position.x = 22;
    NetSyncPosition(50, &position);
    NetSyncUpdate();
    assert(gObjectEvents[0].currentCoords.x == 18);
    NetSyncRemovePlayer(50);
    assert(!gObjectEvents[0].active);
    NetSyncReset();
    return 0;
}
