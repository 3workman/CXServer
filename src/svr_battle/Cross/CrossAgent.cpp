#include "stdafx.h"
#include "CrossAgent.h"
#include "Player/Player.h"
#include "Timer/TimerWheel.h"
#include "Room/Room.h"
#include "Log/LogFile.h"

CrossAgent::CrossAgent()
{
    if (!_rpcfunc[rpc_battle_handle_player_data])
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpcfunc[typ] = (RpcClient::_RpcFunc)&CrossAgent::HandleRpc_##typ;
        Rpc_For_Cross;
    }

    auto ptr = NetMeta::GetMeta("cross");
    _config.svrIp = ptr->ip.c_str();
    _config.svrPort = ptr->tcp_port;
}

//////////////////////////////////////////////////////////////////////////
// rpc
#undef Rpc_Realize
#define Rpc_Realize(typ) void CrossAgent::HandleRpc_##typ(NetPack& req, NetPack& ack)

Rpc_Realize(rpc_battle_handle_player_data)//回复<pid>列表
{
    uint8 cnt = req.ReadUInt8();
    std::vector<Player*> vec; vec.reserve(cnt);

    ack.WriteUInt32((uint32)(Player::GetPlayerCnt() + cnt));
    ack.WriteString(NetCfgServer::Instance().ip);
    ack.WriteUInt16(NetCfgServer::Instance().wPort);

    for (uint i = 0; i < cnt; ++i) {
        uint32 pid = req.ReadUInt32();
        Player* player = Player::FindByPid(pid);
        if (player == NULL) player = new Player(pid);

        //svr_game --- Rpc_Battle_Begin，更新玩家战斗数据
        player->m_name = req.ReadString();
        player->m_svrId = req.ReadInt32();
        player->m_dbData.BufToData(req);

        ack << pid;

        if (player->m_room.m_roomId) {
            LOG_TRACK("PlayerId(%d) is already in room(%d)", pid, player->m_room.m_roomId);
            continue;
        }

        //一段时间client没连上来，防止等待加入中途出错(强杀进程)，内存泄露
        //【Bug】可能在定时器期间，玩家登录又离线，所以还需判断player是否已被delete
        sTimerMgr.AddTimer([=] {if (!player->m_isLogin && player->m_pid) delete player; }, 30);

        vec.push_back(player);
    }

    ack.WriteUInt8(vec.size());
    for (auto& it : vec) ack.WriteUInt32(it->m_pid);

    bool isJoin = false;
    for (auto& it : CRoom::RoomList) {
        if (it.second->TryToJoinWaitLst(vec)) {
            isJoin = true;
            break;
        }
    }
    if (!isJoin) {
        CRoom* pRoom = new CRoom;
        isJoin = pRoom->TryToJoinWaitLst(vec);
        assert(isJoin);
    }
}
