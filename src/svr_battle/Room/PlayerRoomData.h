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
    uint8  m_teamId = 0;//�жԹ�ϵ
public:
    PlayerRoomData(Player& ref) : m_player(ref)
    {
        // Ҫͬ����client�ı������������SendData�����㷢��ʱ��һ�����
        m_SendData << m_netId;
        m_SendData << m_posX;
        m_SendData << m_posY;
    }
    ~PlayerRoomData();

    void NotifyClientJoinRoom();
};
