#pragma once
#include "Config/ConstDef.h"
#include "tool/Mempool.h"

class Player;
class PlayerRoomData {
    Pool_Obj_Define(PlayerRoomData, MAX_PLAYER_COUNT);
public:
    Player& m_player;
    uint32 m_roomId = 0;
    uint8  m_teamId = 0;//µÐ¶Ô¹ØÏµ
    bool   m_canJoinRoom = false;
    RoleType m_roleType = ROLE_HERO;
    //shared<GameObject> gameObject;
public:
    PlayerRoomData(Player& ref) : m_player(ref)
    {
    }
    ~PlayerRoomData();

    void NotifyClientJoinRoom();
};
