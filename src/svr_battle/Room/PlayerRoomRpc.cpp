#include "stdafx.h"
#include "Room.h"
#include "PlayerRoomData.h"
#include "Buffer/NetPack.h"
#include "Player/Player.h"

static CRoom* g_test_room = NULL;

Rpc_Realize(rpc_battle_create_room)
{
    req >> m_Room.m_posX >> m_Room.m_posY;

    CRoom* pRoom = new CRoom;
    pRoom->JoinRoom(*this);
    g_test_room = pRoom;

    ack << pRoom->GetUniqueId();
}
Rpc_Realize(rpc_battle_join_room)
{
    req >> m_Room.m_posX >> m_Room.m_posY;

    uint32 roomId = req.ReadUInt32();

    CRoom* pRoom = roomId ? CRoom::FindByUniqueId(roomId) : g_test_room;

    if (pRoom && pRoom->JoinRoom(*this))
    {
        ack << pRoom->GetUniqueId();

        // 通知新人，既有玩家信息
        const auto& lst = pRoom->GetPlayerLst();
        ack.WriteUInt8(lst.size() - 1);
        for (auto& it : lst) {
            Player* ptr = it.second;
            if (ptr == this) continue;
            ack.WriteUInt32(ptr->m_index);
            ack.WriteUInt32(ptr->m_Room.m_netId);
            ack.WriteFloat(ptr->m_Room.m_posX);
            ack.WriteFloat(ptr->m_Room.m_posY);
        }
    }
}
Rpc_Realize(rpc_battle_exit_room)
{
    if (CRoom* pRoom = CRoom::FindByUniqueId(m_Room.m_roomId))
    {
        pRoom->ExitRoom(*this);
    }
}
Rpc_Realize(rpc_battle_move_delta)
{
    req >> m_Room.m_netId;
    float deltaMoveX = req.ReadFloat();
    float deltaMoveY = req.ReadFloat();
    //m_Room.m_posX += deltaMoveX;
    //m_Room.m_posY += deltaMoveY;
    m_Room.m_posX = deltaMoveX;
    m_Room.m_posY = deltaMoveY;
}