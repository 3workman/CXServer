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
        // ֪ͨClient�رյȴ����棬����ս��������������Ϻ�ظ�svr����ʱ�в��ܲ���
    });
}
void PlayerRoomData::OnClientJoinRoomOK()
{
    if (CRoom* p = CRoom::FindByUniqueId(m_roomId))
    {
        if (p->JoinRoom(m_player))
        {
            m_player.CallRpc("rpc_svr_join_room_ok", [&](NetPack& buf){
                // ��������Ҽ��뷿�䣬���������ٻظ�client��client���ɲ���
            });
        }
    }
}