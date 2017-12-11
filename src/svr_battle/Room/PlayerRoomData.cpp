#include "stdafx.h"
#include "PlayerRoomData.h"
#include "Room.h"
#include "Player/Player.h"
#include "room_generated.h"

PlayerRoomData::~PlayerRoomData()
{
    if (CRoom* p = CRoom::FindByUniqueId(m_roomId))
    {
        p->ExitRoom(m_player);
    }
}
void PlayerRoomData::NotifyClientJoinRoom()
{
    if (CRoom* pRoom = CRoom::FindByUniqueId(m_roomId))
    {
        pRoom->JoinRoom(m_player); //��̨�����ȼӽ���������������ͬʱ�����䣬��bug����֪���˴˼���

        m_player.CallRpc(rpc_client_stop_wait_and_load_battle_scene, [&](NetPack& buf){

            auto& build = m_player.SendBuild();
            std::vector<flatbuffers::Offset<flat::PlayerBaseData>> vec;
            for (auto& it : pRoom->GetPlayerLst()) {
                Player* ptr = it.second;
                auto& data = flat::CreatePlayerBaseData(build,
                    ptr->m_pid,
                    build.CreateString(ptr->m_name),
                    ptr->m_index,
                    ptr->m_room.m_teamId
                    );
                vec.push_back(data);
            }
            auto& obj = flat::CreateRoomPlayers(build, pRoom->GetUniqueId(), build.CreateVector(vec));
            build.Finish(obj);

            // ֪ͨClient�رյȴ����棬����ս��������������Ϻ�ظ�svr
            // �·���������ҵ����ݣ���client����Զ�̾���(�����Լ�)
            //buf << p->GetUniqueId();
            //buf.WriteUInt8(p->GetPlayerLst().size());
            //for (auto& it : p->GetPlayerLst()) {
            //    Player* ptr = it.second;
            //    buf.WriteUInt32(ptr->m_pid);
            //    buf.WriteString(ptr->m_name);
            //    buf.WriteUInt32(ptr->m_index);
            //    buf.WriteUInt8(ptr->m_Room.m_teamId);
            //}
        });
    }
}
