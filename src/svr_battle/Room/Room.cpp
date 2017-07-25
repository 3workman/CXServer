#include "stdafx.h"
#include "Room.h"
#include "PlayerRoomData.h"
#include "Service/ServiceMgr.h"
#include "Buffer/NetPack.h"
#include "Player/Player.h"

static const uint Room_Player_Min = 1;
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

    // ս���������������ڵ����
    for (auto& it : m_waitLst) {
        for (auto& player : it.second) {
            player->m_Room.m_roomId = 0;
        }
    }
    for (auto& it : m_players) {
        it.second->m_Room.m_roomId = 0;
    }
    delete this;
}
bool CRoom::JoinRoom(Player& player)
{
    // �㲥�����˼�����Ϸ
    for (auto& it : m_players) {
        it.second->CallRpc("rpc_client_notify_player_join_room", [&](NetPack& buf){
            buf.WriteUInt32(player.m_pid);
            buf.WriteString(player.m_name);
            buf.WriteUInt32(player.m_index);
            buf.WriteUInt8(player.m_Room.m_teamId);
            buf.WriteUInt32(player.m_Room.m_netId);
            buf.WriteFloat(player.m_Room.m_posX);
            buf.WriteFloat(player.m_Room.m_posY);
        });
    }
    m_players[player.m_index] = &player;
    player.m_Room.m_roomId = GetUniqueId();
    return true;
}
bool CRoom::ExitRoom(Player& player)
{
    CancelWait(player);

    m_players.erase(player.m_index);
    player.m_Room.m_roomId = 0;

    // �㲥�������˳�
    for (auto& it : m_players) {
        it.second->CallRpc("rpc_client_notify_player_exit_room", [&](NetPack& buf){
            buf.WriteUInt32(player.m_pid);
            buf.WriteUInt32(player.m_index);
            buf.WriteUInt32(player.m_Room.m_netId);
        });
    }
    if (m_players.empty() && m_waitLst.empty()) DestroyRoom();
    return true;
}
void CRoom::CancelWait(Player& player)
{
    for (auto& it : m_waitLst) {
        for (auto itt = it.second.begin(); itt != it.second.end(); ++itt) {
            if ((*itt)->m_index == player.m_index) {
                it.second.erase(itt);
                return;
            }
        }
    }
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
        if (teamId == ptr->m_Room.m_teamId)
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
    //�����λ����
    if (m_players.size() + lst.size() > Room_Player_Max) return false;

    //������ս����������
    const uint OneTeamPlayerMax = Room_Player_Max / m_kTeamCntMax;
    if (lst.size() > OneTeamPlayerMax) return false;

    std::map<uint8, uint> teamInfos;
    for (auto& it : m_players) {
        teamInfos[it.second->m_Room.m_teamId] += 1;
    }
    for (auto& it : m_waitLst) {
        teamInfos[it.first] += it.second.size();
    }
    //�½�ս�ӷ�
    if (teamInfos.size() < m_kTeamCntMax) {
        uint8 newTeamId = teamInfos.empty() ? 1 : teamInfos.rbegin()->first + 1;
        teamInfos[newTeamId] = lst.size();
        for (auto& it : lst) it->m_Room.m_roomId = m_unique_id;
        m_waitLst[newTeamId] = std::move(lst);
        _FlushWaitLst(teamInfos);
        return true;
    }
    //���ȼ��������ٵ�ս��
    uint min = 100; uint8 minId = 0;
    for (auto& it : teamInfos) {
        if (it.second < min) {
            min = it.second;
            minId = it.first;
        }
    }
    if (teamInfos[minId] + lst.size() <= OneTeamPlayerMax) {
        teamInfos[minId] += lst.size();
        for (auto& it : lst) {
            it->m_Room.m_roomId = m_unique_id;
            m_waitLst[minId].push_back(it);
        }
        _FlushWaitLst(teamInfos);
        return true;
    }
    return false;
}
void CRoom::_FlushWaitLst(const std::map<uint8, uint>& teamInfos)
{
    //if (teamInfos.size() < 2) return;//TODO:zhoumf:��ʱ�ſ�

    uint min = 100, max = 0, total = 0;
    for (auto& it : teamInfos) {
        if (it.second < min) min = it.second;
        if (it.second > max) max = it.second;
        total += it.second;
    }
    if (total >= Room_Player_Min && max - min <= 1) {
        //������ҵ�m_roomId����¼ʱ�����Լ�m_roomId��Ч������������ս���������̡�
        //�ѵ�¼�ģ�ֱ�ӿ���������ս���������̡�
        /* ����������ս���������̣�
            1��֪ͨClient�رյȴ����棬����ս������
            2����Ϻ��ϱ�svr_battle����ʱ�в��ܲ���
            3��svr_battle�յ����������Ϣ����������Ҽ��뷿�䣬���������ٻظ�client
            4��client��������
        */
        for (auto& it : m_waitLst) {
            for (auto& player : it.second) {
                player->m_Room.m_teamId = it.first;
                player->m_Room.m_canJoinRoom = true;
                if (player->m_isLogin) { player->m_Room.NotifyClientJoinRoom(); }
            }
        }
        m_waitLst.clear();
    }
}
void CRoom::SyncPlayerPosition()
{
    for (auto& it : m_players) {
        it.second->CallRpc("rpc_client_sync_position", [&](NetPack& buf){
            buf.WriteUInt8((uint8)m_players.size());
            for (auto& itr : m_players) {
                Player* ptr = itr.second;
                buf.WriteUInt32(ptr->m_index);
                ptr->m_Room.m_SendData.ToBuf(buf);
            }
        });
    }
}
