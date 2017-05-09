#pragma once
#include "def/ConstDef.h"
#include "tool/Mempool.h"
#include "../Player/PlayerDataSend.h"

class PlayerRoomData {
    Pool_Obj_Define(PlayerRoomData, MAX_PLAYER_COUNT);
public:
    PlayerDataSend m_SendData;
    uint32 m_RoomId;
    uint32 m_netId;
    float  m_posX;
    float  m_posY;
public:
    PlayerRoomData() {
        // 要同步给client的变量，按序放入SendData，方便发送时，一键打包
        m_SendData << m_netId;
        m_SendData << m_posX;
        m_SendData << m_posY;
    }
};
