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
    if (CRoom* p = CRoom::FindByUniqueId(m_roomId))
    {
        p->JoinRoom(m_player); //��̨�����ȼӽ���������������ͬʱ�����䣬��bug����֪���˴˼���

        m_player.CallRpc("rpc_client_stop_wait_and_load_battle_scene", [&](NetPack& buf){
            // ֪ͨClient�رյȴ����棬����ս��������������Ϻ�ظ�svr
            // �·���������ҵ����ݣ���client����Զ�̾���(�����Լ�)
            buf << p->GetUniqueId();
            buf.WriteUInt8(p->GetPlayerLst().size());
            for (auto& it : p->GetPlayerLst()) {
                Player* ptr = it.second;
                buf.WriteUInt32(ptr->m_pid);
                buf.WriteString(ptr->m_name);
                buf.WriteUInt32(ptr->m_index);
                buf.WriteUInt8(ptr->m_Room.m_teamId);
            }
        });
    }
}
