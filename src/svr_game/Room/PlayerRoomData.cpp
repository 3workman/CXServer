#include "stdafx.h"
#include "PlayerRoomData.h"
#include "Room.h"
#include "../Player/Player.h"

void PlayerRoomData::ExitRoom(Player& player)
{
    if (CRoom* p = CRoom::FindByUniqueId(m_RoomId))
    {
        p->ExitRoom(player);
    }
}