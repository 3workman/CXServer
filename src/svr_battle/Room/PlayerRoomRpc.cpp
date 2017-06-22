#include "stdafx.h"
#include "Room.h"
#include "PlayerRoomData.h"
#include "Buffer/NetPack.h"
#include "Player/Player.h"

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