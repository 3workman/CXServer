#include "stdafx.h"
#include "Player/Player.h"
#include "CrossAgent.h"
#ifdef _WIN32
#include "iocp/client/ClientLink.h"
#else
#include "handy/client/TcpClient.h"
#endif
#include "Timer/TimerWheel.h"
#include "Room/Room.h"
#include "Log/LogFile.h"
#include "Room/PlayerRoomData.h"

using namespace std;
std::map<int, CrossAgent::_RpcFunc> CrossAgent::_rpc;

void CrossAgent::RunClient()
{
    _netLink->CreateLinkAndConnect([&]() {
        _OnConnect();
    }, [&](const void* p, int size) {
        sRpcCross.Insert(this, p, size);
    });

    //// 等待ConnectEx三次握手完成的回调，之后才能发数据
    //while (!_netLink->IsClose() && !_netLink->IsConnect()) Sleep(200);
}
void CrossAgent::_OnConnect()
{
    //Notice: 这里不能用CallRpc，多线程呐~
    _netLink->SendMsg(&_connId, sizeof(_connId)); //第一条消息：上报connId
    NetPack regMsg(16);
    regMsg.OpCode(sRpcCross.RpcNameToId("rpc_regist"));
    regMsg << "battle" << (uint32)1;
    SendMsg(regMsg);
}

CrossAgent::CrossAgent()
{
    _netLink = new TcpClient(_config);

    if (_rpc.empty())
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpc[sRpcCross.RpcNameToId(#typ)] = &CrossAgent::HandleRpc_##typ;
        Rpc_For_Cross;
    }
}
CrossAgent::~CrossAgent()
{
    delete _netLink;
}
uint64 CrossAgent::CallRpc(const char* name, const ParseRpcParam& sendFun)
{
    return sRpcCross._CallRpc(name, sendFun, std::bind(&CrossAgent::SendMsg, this, std::placeholders::_1));
}
void CrossAgent::CallRpc(const char* name, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun)
{
    uint64 reqKey = CallRpc(name, sendFun);

    sRpcCross.RegistResponse(reqKey, recvFun);
}
void CrossAgent::SendMsg(const NetPack& pack)
{
    _netLink->SendMsg(pack.contents(), pack.size());
}

//////////////////////////////////////////////////////////////////////////
// rpc
Rpc_Realize(rpc_echo)
{
    string str = req.ReadString();
    printf("Echo: %s\n", str.c_str());

    //NetPack& backBuffer = BackBuffer();
    //backBuffer << str;
}
Rpc_Realize(rpc_svr_accept)
{
    _connId = req.ReadUInt32();
}
Rpc_Realize(rpc_battle_handle_player_data)//回复<pid>列表
{
    uint8 cnt = req.ReadUInt8();
    ack << Player::GetPlayerCnt() + cnt;
    ack << cnt;
    std::vector<Player*> lst; lst.reserve(cnt);
    for (uint i = 0; i < cnt; ++i) {
        uint32 pid = req.ReadUInt32();
        Player* player = Player::FindByPid(pid);
        if (player == NULL) player = new Player(pid);

        //svr_game --- Rpc_Battle_Begin，更新玩家战斗数据
        player->m_name = req.ReadString();

        ack << pid;

        if (player->m_Room.m_roomId) {
            LOG_TRACK("PlayerId(%d) is already in room(%d)", pid, player->m_Room.m_roomId);
            continue;
        }

        //一段时间client没连上来，防止等待加入中途出错(强杀进程)，内存泄露
        //【Bug】可能在定时器期间，玩家登录又离线，所以还需判断player是否已被delete
        sTimerMgr.AddTimer([=]{
            if (!player->m_isLogin && player->m_pid) delete player;
        }, 10);

        lst.push_back(player);
    }
    bool isJoin = false;
    for (auto& it : CRoom::RoomList) {
        if (it.second->TryToJoinWaitLst(lst)) {
            isJoin = true;
            break;
        }
    }
    if (!isJoin) {
        CRoom* pRoom = new CRoom;
        isJoin = pRoom->TryToJoinWaitLst(lst);
        assert(isJoin);
    }
}
