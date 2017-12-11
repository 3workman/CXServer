#include "stdafx.h"
#include "Room.h"
#include "PlayerRoomData.h"
#include "Buffer/NetPack.h"
#include "Player/Player.h"

//////////////////////////////////////////////////////////////////////////
// rpc
Rpc_Realize(rpc_battle_exit_room)
{
    if (CRoom* pRoom = CRoom::FindByUniqueId(m_room.m_roomId))
    {
        pRoom->ExitRoom(*this);
    }
}