#include "stdafx.h"
#include "Room.h"
#include "..\Player\Player.h"
#include "PlayerRoomData.h"
#include "Service\ServiceMgr.h"
#include "Buffer\NetPack.h"

DWORD _Service_Sync_Position(void* p){
    CRoom* pRoom = (CRoom*)p;
    pRoom->SyncPlayerPosition();
    return 67;
}

CRoom::CRoom()
{
    _CreateUniqueId();
    ServiceMgr::Register(Service_Sync_Position, this);
}
CRoom* CRoom::CreateRoom(Player& player)
{
    CRoom* pRoom = new CRoom();
    pRoom->JoinRoom(player);
    return pRoom;
}
void CRoom::DestroyRoom()
{
    ServiceMgr::UnRegister(Service_Sync_Position, this);
    delete this;
}
bool CRoom::JoinRoom(Player& player)
{
    // 广播，有人加入游戏
    for (auto& it : m_players) {
        Player* ptr = it.second;
        ptr->CallRpc("rpc_notify_player_join_room", [&](NetPack& buf){
            buf.WriteUInt32(player.m_index);
            buf.WriteUInt32(player.m_RoomData->m_netId);
            buf.WriteFloat(player.m_RoomData->m_posX);
            buf.WriteFloat(player.m_RoomData->m_posY);
        });
    }
    m_players[player.m_index] = &player;
    player.m_RoomData->m_RoomId = GetUniqueId();
    return true;
}
bool CRoom::ExitRoom(Player& player)
{
    m_players.erase(player.m_index);
    player.m_RoomData->m_RoomId = 0;

    // 广播，有人退出
    for (auto& it : m_players) {
        Player* ptr = it.second;
        ptr->CallRpc("rpc_notify_player_exit_room", [&](NetPack& buf){
            buf.WriteUInt32(player.m_index);
            buf.WriteUInt32(player.m_RoomData->m_netId);
        });
    }
    if (m_players.empty()) DestroyRoom();
    return true;
}
void CRoom::SyncPlayerPosition()
{
    for (auto& it : m_players) {
        it.second->CallRpc("rpc_sync_position", [&](NetPack& buf){
            buf.WriteUInt8(m_players.size());
            for (auto& itr : m_players) {
                Player* ptr = itr.second;
                buf.WriteUInt32(ptr->m_index);
                buf.WriteUInt32(ptr->m_RoomData->m_netId);
                buf.WriteFloat(ptr->m_RoomData->m_posX);
                buf.WriteFloat(ptr->m_RoomData->m_posY);
            }
        });
    }
}