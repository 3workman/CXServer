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
        pRoom->JoinRoom(m_player); //后台必须先加进来，否则两个人同时进房间，有bug：不知道彼此加入

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

            // 通知Client关闭等待界面，载入战斗场景，载入完毕后回复svr
            // 下发房间内玩家的数据，给client创建远程镜像(包含自己)
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
