#include "stdafx.h"
#include "Room.h"
#include "PlayerRoomData.h"
#include "Service/ServiceMgr.h"
#include "Buffer/NetPack.h"
#include "Player/Player.h"

static const uint Room_Player_Min = 2;
static const uint Room_Player_Max = 20;

uint _Service_Sync_Position(void* p){
    CRoom* pRoom = (CRoom*)p;
    pRoom->SyncPlayerPosition();
    return 67;
}

std::map<uint32, CRoom*> CRoom::RoomList;

CRoom::CRoom(uint8 teamCnt /* = 2 */) : m_kTeamCntMax(teamCnt)
{
    ServiceMgr::Register(Service_Sync_Position, this);

    RoomList[m_unique_id] = this;
}
void CRoom::DestroyRoom()
{
    ServiceMgr::UnRegister(Service_Sync_Position, this);
    RoomList.erase(m_unique_id);
    delete this;
}
bool CRoom::JoinRoom(Player& player)
{
    //assert(m_players.find(player.m_index) == m_players.end());
    // 广播，有人加入游戏
    for (auto& it : m_players) {
        Player* ptr = it.second;
        ptr->CallRpc("rpc_notify_player_join_room", [&](NetPack& buf){
            buf.WriteUInt32(player.m_index);
            buf.WriteUInt32(player.m_Room->m_netId);
            buf.WriteFloat(player.m_Room->m_posX);
            buf.WriteFloat(player.m_Room->m_posY);
        });
    }
    m_players[player.m_index] = &player;
    player.m_Room->m_roomId = GetUniqueId();
    return true;
}
bool CRoom::ExitRoom(Player& player)
{
    m_players.erase(player.m_index);
    player.m_Room->m_roomId = 0;

    // 广播，有人退出
    for (auto& it : m_players) {
        Player* ptr = it.second;
        ptr->CallRpc("rpc_notify_player_exit_room", [&](NetPack& buf){
            buf.WriteUInt32(player.m_index);
            buf.WriteUInt32(player.m_Room->m_netId);
        });
    }
    if (m_players.empty()) DestroyRoom();
    return true;
}
Player* CRoom::FindBattlePlayer(uint idx)
{
    auto it = m_players.find(idx);
    if (it == m_players.end()) return nullptr;
    return it->second;
}
void CRoom::ForEachTeammate(uint8 teamId, std::function<void(Player&)>& func)
{
    for (auto& it : m_players) {
        Player* ptr = it.second;
        if (teamId == ptr->m_Room->m_teamId)
        {
            func(*ptr);
        }
    }
}
void CRoom::ForEachPlayer(std::function<void(Player&)>& func)
{
    for (auto& it : m_players) func(*it.second);
}

bool CRoom::TryToJoinWaitLst(const std::vector<Player*>& lst)
{
    //房间空位不够
    if (m_players.size() + lst.size() > Room_Player_Max) return false;

    //超过单战队人数限制
    const uint OneTeamPlayerMax = Room_Player_Max / m_kTeamCntMax;
    if (lst.size() > OneTeamPlayerMax) return false;

    std::map<uint8, uint> teamInfos;
    for (auto& it : m_players) {
        teamInfos[it.second->m_Room->m_teamId] += 1;
    }
    for (auto& it : m_waitLst) {
        teamInfos[it.first] += it.second.size();
    }
    //新建战队否
    if (teamInfos.size() < m_kTeamCntMax) {
        uint8 newTeamId = teamInfos.empty() ? 1 : teamInfos.rbegin()->first + 1;
        teamInfos[newTeamId] = lst.size();
        m_waitLst[newTeamId] = std::move(lst);
        _FlushWaitLst(teamInfos);
        return true;
    }
    //优先加人数最少的战队
    uint min = 100; uint8 minId = 0;
    for (auto& it : teamInfos) {
        if (it.second < min) {
            min = it.second;
            minId = it.first;
        }
    }
    if (teamInfos[minId] + lst.size() <= OneTeamPlayerMax) {
        teamInfos[minId] += lst.size();
        auto& refLst = m_waitLst[minId];
        for (auto& it : lst) refLst.push_back(it);
        _FlushWaitLst(teamInfos);
        return true;
    }
    return false;
}
void CRoom::_FlushWaitLst(const std::map<uint8, uint>& teamInfos)
{
    if (teamInfos.size() < 2) return;

    uint min = 100, max = 0;
    for (auto& it : teamInfos) {
        if (it.second < min) min = it.second;
        if (it.second > max) max = it.second;
    }
    if (max - min <= 1) {
        //设置玩家的m_roomId，登录时，若自己m_roomId有效，开启“进入战斗场景流程”
        //已登录的，直接开启“进入战斗场景流程”
        /* 进入战斗场景流程：
            1、通知Client关闭等待界面，载入战斗场景
            2、完毕后上报svr_battle，此时尚不能操作
            3、svr_battle收到载入完毕消息，真正将玩家加入房间，进场景，再回复client
            4、client操作解锁
        */
        for (auto& it : m_waitLst) {
            for (auto& player : it.second) {
                player->m_Room->m_roomId = m_unique_id;
                player->m_Room->m_teamId = it.first;
                if (player->m_isLogin) {
                    player->m_Room->NotifyClientJoinRoom();
                }
            }
        }
    }
}

void CRoom::SyncPlayerPosition()
{
    for (auto& it : m_players) {
        it.second->CallRpc("rpc_sync_position", [&](NetPack& buf){
            buf.WriteUInt8(m_players.size());
            for (auto& itr : m_players) {
                Player* ptr = itr.second;
                buf.WriteUInt32(ptr->m_index);
                ptr->m_Room->m_SendData.ToBuf(buf);
            }
        });
    }
}
