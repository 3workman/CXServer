#include "stdafx.h"
#include "Player/Player.h"
#include "CrossAgent.h"
#include "iocp/client/ClientLink.h"
#include "Timer/TimerWheel.h"
#include "Room/Room.h"
#include "Room/PlayerRoomData.h"
#include "Log/LogFile.h"

std::map<int, CrossAgent::_RpcFunc> CrossAgent::_rpc;

void CrossAgent::RunClientIOCP()
{
    _netLink->SetOnConnect([&](){
        //Notice: ���ﲻ����CallRpc�����߳���~
        SendMsg(_first_buf); //Notice: ����Go�Ǳߵײ�Э����Զ��������ܣ��������ӽ����ĵ�һ����
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
CrossAgent::CrossAgent()
{
    _config.svrPort = 7003; //TODO: svr_cross: ip & port
    _netLink = new ClientLink(_config);

    if (_rpc.empty())
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpc[sRpcCross.RpcNameToId(#typ)] = &CrossAgent::HandleRpc_##typ;
        Rpc_For_Cross;
    }

    _first_buf << uint32(0);
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
    _netLink->SendMsg(pack.contents(), (uint16)pack.size());
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
    auto connId = req.ReadUInt32();
    _first_buf.SetPos(0, connId);
}
Rpc_Realize(rpc_battle_handle_player_data) //�ظ�<pid>�б�
{
    uint8 cnt = req.ReadUInt8();
    ack << cnt;
    vector<Player*> lst; lst.reserve(cnt);
    for (uint i = 0; i < cnt; ++i) {
        uint32 pid = req.ReadUInt32();
        Player* player = Player::FindByPid(pid);
        if (player == NULL) player = new Player(pid);

        //svr_game --- Rpc_Battle_Begin���������ս������
        player->m_name = req.ReadString();

        ack << pid;

        if (player->m_Room.m_roomId) {
            LOG_TRACK("PlayerId(%d) is already in room(%d)", pid, player->m_Room.m_roomId);
            continue;
        }

        //һ��ʱ��clientû����������ֹ�ȴ�������;����(ǿɱ����)���ڴ�й¶
        //��Bug�������ڶ�ʱ���ڼ䣬��ҵ�¼�����ߣ����Ի����ж�player�Ƿ��ѱ�delete
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
