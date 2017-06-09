#include "stdafx.h"
#include "PlayerRoomData.h"
#include "Room.h"
#include "Player/Player.h"

PlayerRoomData::~PlayerRoomData()
{
    if (CRoom* p = CRoom::FindByUniqueId(m_roomId))
    {
        p->ExitRoom(m_player);
    }
}
void PlayerRoomData::NotifyClientJoinRoom()
{
    m_player.CallRpc("rpc_stop_wait_and_load_battle_scene", [&](NetPack& buf){
        // 通知Client关闭等待界面，载入战斗场景，载入完毕后回复svr；此时尚不能操作
    });
}
void PlayerRoomData::OnClientJoinRoomOK()
{
    if (CRoom* p = CRoom::FindByUniqueId(m_roomId))
    {
        if (p->JoinRoom(m_player))
        {
            m_player.CallRpc("rpc_svr_join_room_ok", [&](NetPack& buf){
                // 真正将玩家加入房间，进场景，再回复client；client方可操作
            });
        }
    }
}