#include "stdafx.h"
#include "..\Player\Player.h"
#include "Room.h"
#include "PlayerRoomData.h"
#include "Buffer\NetPack.h"

static CRoom* g_test_room = NULL;

Rpc_Realize(rpc_create_room)
{
    recvBuf >> m_RoomData->m_posX >> m_RoomData->m_posY;

    if (CRoom* pRoom = CRoom::CreateRoom(*this))
    {
        g_test_room = pRoom;

        NetPack& backBuffer = BackBuffer();
        backBuffer << pRoom->GetUniqueId();
    }
}
Rpc_Realize(rpc_join_room)
{
    recvBuf >> m_RoomData->m_posX >> m_RoomData->m_posY;

    uint32 roomId = recvBuf.ReadUInt32();

    CRoom* pRoom = roomId ? CRoom::FindByUniqueId(roomId) : g_test_room;

    if (pRoom && pRoom->JoinRoom(*this))
    {
        NetPack& backBuffer = BackBuffer();
        backBuffer << pRoom->GetUniqueId();

        // 通知新人，既有玩家信息
        const auto& lst = pRoom->GetPlayerLst();
        backBuffer.WriteUInt8(lst.size() - 1);
        for (auto& it : lst) {
            Player* ptr = it.second;
            if (ptr == this) continue;
            backBuffer.WriteUInt32(ptr->m_index);
            backBuffer.WriteUInt32(ptr->m_RoomData->m_netId);
            backBuffer.WriteFloat(ptr->m_RoomData->m_posX);
            backBuffer.WriteFloat(ptr->m_RoomData->m_posY);
        }
    }
}
Rpc_Realize(rpc_exit_room)
{
    if (CRoom* pRoom = CRoom::FindByUniqueId(m_RoomData->m_RoomId))
    {
        pRoom->ExitRoom(*this);
    }
}
Rpc_Realize(rpc_move_delta)
{
    float deltaMoveX = recvBuf.ReadFloat();
    float deltaMoveY = recvBuf.ReadFloat();
    //m_RoomData->m_posX += deltaMoveX;
    //m_RoomData->m_posY += deltaMoveY;
    m_RoomData->m_posX = deltaMoveX;
    m_RoomData->m_posY = deltaMoveY;
}