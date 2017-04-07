#pragma once
#include "def\ConstDef.h"
#include "tool\Mempool.h"

class Player;
class PlayerRoomData {
    Pool_Obj_Define(PlayerRoomData, MAX_PLAYER_COUNT);
public:
    uint32 m_RoomId;
    uint32 m_netId;
    float  m_posX;
    float  m_posY;
public:
};
