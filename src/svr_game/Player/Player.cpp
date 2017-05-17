#include "stdafx.h"
#include "../NetLib/server/ServLink.h"
#include "../NetLib/UdpServer/UdpClientAgent.h"
#include "Player.h"
#include "Buffer/NetPack.h"
#include "../Room/PlayerRoomData.h"

std::map<int, Player::_RpcFunc> Player::_rpc;

Player::Player()
{
    if (_rpc.empty())
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpc[sRpcClient.RpcNameToId(#typ)] = &Player::HandleRpc_##typ;
        Rpc_For_Player;
    }
    m_RoomData = new PlayerRoomData();
}
Player::~Player()
{
    m_RoomData->ExitRoom(*this);

    delete m_RoomData;
}
void Player::SetNetLink(NetLink* p)
{
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

//////////////////////////////////////////////////////////////////////////
// rpc
Rpc_Realize(rpc_login)
{
    printf("rpc_login\n");
    recvBuf >> m_RoomData->m_netId;

    NetPack& backBuffer = BackBuffer();
    backBuffer << m_index;
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
    string str = recvBuf.ReadString();
    const char* pstr = str.c_str();
    LOG_TRACK("Echo: %s\n", pstr);

    NetPack& backBuffer = BackBuffer();
    backBuffer << str;
}
