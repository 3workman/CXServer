#include "stdafx.h"
#include "Player/Player.h"
#include "CrossAgent.h"
#include "../NetLib/client/ClientLink.h"
#include "Timer/TimerWheel.h"
#include "Room/Room.h"
#include "Room/PlayerRoomData.h"
#include "Log/LogFile.h"

std::map<int, CrossAgent::_RpcFunc> CrossAgent::_rpc;

void CrossAgent::RunClientIOCP()
{
    _netLink->SetOnConnect([&](){
        //Notice: ���ﲻ����CallRpc�����߳���~
        SendMsg(_first_buf);
        NetPack regMsg(16);
        regMsg.OpCode(sRpcCross.RpcNameToId("rpc_regist"));
        regMsg << "battle" << (uint32)1;
        SendMsg(regMsg);
    });
    _netLink->CreateLinkAndConnect([&](void* p, int size){
        sRpcCross.Insert(this, p, size);
    });
    // �ȴ�ConnectEx����������ɵĻص���֮����ܷ�����
    while (!_netLink->IsClose() && !_netLink->IsConnect()) Sleep(200);
}
CrossAgent::CrossAgent() : _netLink(new ClientLink(_config))
{
    if (_rpc.empty())
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpc[sRpcCross.RpcNameToId(#typ)] = &CrossAgent::HandleRpc_##typ;
        Rpc_For_Cross;
    }

    _first_buf << uint32(0);
    _config.wServerPort = 7003; //TODO: go cross
}
CrossAgent::~CrossAgent()
{
    _netLink->SetReConnect(false);
    _netLink->CloseLink(0);
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
    _netLink->SendMsg(pack.Buffer(), pack.Size());
}

//////////////////////////////////////////////////////////////////////////
// rpc
Rpc_Realize(rpc_echo)
{
    string str = recvBuf.ReadString();
    printf("Echo: %s\n", str.c_str());

    //NetPack& backBuffer = BackBuffer();
    //backBuffer << str;
}
Rpc_Realize(rpc_svr_accept)
{
    auto connId = recvBuf.ReadUInt32();
    _first_buf.SetPos(0, connId);
}
Rpc_Realize(rpc_handle_battle_data)
{
    NetPack& backBuffer = BackBuffer(); //�ظ�<pid, roomId, teamId>�б�

    uint8 cnt = recvBuf.ReadUInt8();
    vector<Player*> lst; lst.reserve(cnt);
    backBuffer << cnt;
    for (uint8 i = 0; i < cnt; ++i) {
        //svr_game --- Rpc_Battle_Begin
        uint32 pid = recvBuf.ReadUInt32();

        if (Player* player = Player::FindByPid(pid)) {
            LOG_TRACK("PlayerId(%d) is already in room", pid);
            backBuffer << pid << player->m_index;
            continue;
        }

        //����svr_game������ս�����ݣ��������
        Player* player = new Player();
        player->m_pid = pid;
        Player::PlayerList[pid] = player;

        //����ȴ�ƥ��ʱ�䡿�ڣ�clientû����������ֹ�ȴ�������;����(ǿɱ����)���ڴ�й¶
        //��Bug�������ڶ�ʱ���ڼ䣬��ҵ�¼�����ߣ����Ի����ж�player�Ƿ��ѱ�delete
        sTimerMgr.AddTimer([=]{
            if (!player->m_isLogin && player->m_Room) delete player;
        }, 60);

        lst.push_back(player);
        backBuffer << pid << player->m_index;
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
