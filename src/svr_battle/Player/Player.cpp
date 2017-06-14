#include "stdafx.h"
#include "../NetLib/server/ServLink.h"
#include "../NetLib/UdpServer/UdpClientAgent.h"
#include "Player.h"
#include "Buffer/NetPack.h"
#include "../svr_battle/Room/PlayerRoomData.h"

std::map<int, Player::_RpcFunc> Player::_rpc;
std::map<uint32, Player*> Player::PlayerList;

Player::Player(uint32 pid)
    : m_pid(pid)
    , m_Room(*new PlayerRoomData(*this))
{
    PlayerList[pid] = this;

    if (_rpc.empty())
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpc[sRpcClient.RpcNameToId(#typ)] = &Player::HandleRpc_##typ;
        Rpc_For_Player;
    }
}
Player::~Player()
{
    delete &m_Room;

    PlayerList.erase(m_pid); m_pid = 0;
}
void Player::SetNetLink(NetLink* p)
{
    if (_clientNetLink)
    {
        _clientNetLink->CloseLink();
    }
    _clientNetLink = p;
}
void Player::SendMsg(const NetPack& pack)
{
    //Notice：断线重连期间，连接无效
    if (_clientNetLink) {
        _clientNetLink->SendMsg(pack.Buffer(), pack.Size());
    }
}
uint64 Player::CallRpc(const char* name, const ParseRpcParam& sendFun)
{
    return sRpcClient._CallRpc(name, sendFun, std::bind(&Player::SendMsg, this, std::placeholders::_1));
}
void Player::CallRpc(const char* name, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun)
{
    uint64 reqKey = CallRpc(name, sendFun);
    sRpcClient.RegistResponse(reqKey, recvFun);
}
Player* Player::FindByPid(uint32 pid)
{
    auto it = PlayerList.find(pid);
    if (it == PlayerList.end()) return NULL;
    return it->second;
}

//////////////////////////////////////////////////////////////////////////
// rpc
Rpc_Realize(rpc_login)
{
    printf("rpc_login\n");
    req >> m_index >> m_pid;

    if (m_Room.m_roomId > 0)
    {
        m_Room.NotifyClientJoinRoom();
    }
}
Rpc_Realize(rpc_logout)
{
    printf("rpc_logout\n");
    m_isLogin = false;
}
Rpc_Realize(rpc_reconnect)
{
    printf("rpc_reconnect\n");

    //TODO:到这里已经重连成功了，通知client
}
Rpc_Realize(rpc_echo)
{
    string str = req.ReadString();
    const char* pstr = str.c_str();
    LOG_TRACK("Echo: %s\n", pstr);

    ack << str;
}
