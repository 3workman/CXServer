#pragma once
#include "def/ConstDef.h"
#include "tool/Mempool.h"
#include "Player/PlayerDataSend.h"

class Player;
class PlayerRoomData {
    Pool_Obj_Define(PlayerRoomData, MAX_PLAYER_COUNT);
public:
    Player& m_player;
    PlayerDataSend m_SendData;
    uint32 m_roomId = 0;
    uint32 m_netId;
    float  m_posX;
    float  m_posY;
    uint8  m_teamId = 0;//敌对关系
public:
    PlayerRoomData(Player& ref) : m_player(ref)
    {
        // 要同步给client的变量，按序放入SendData，方便发送时，一键打包
        m_SendData << m_netId;
        m_SendData << m_posX;
        m_SendData << m_posY;
    }
    ~PlayerRoomData();

    void NotifyClientJoinRoom();
};
