#pragma once
#include "RpcEnum.h"
#include "RpcQueue.h"
#include "config_net.h"

#undef Rpc_Declare
#undef Rpc_Realize
#define Rpc_Declare(typ) void HandleRpc_##typ(NetPack&, NetPack&);
#define Rpc_Realize(typ) void Player::HandleRpc_##typ(NetPack& req, NetPack& ack)

#define sRpcClientPlayer RpcQueue<CPlayer>::Instance()

class NetPack;
class UdpClient;
class CPlayer : boost::noncopyable {
    static NetCfgClient _netCfg;
    UdpClient*      _netLink = NULL;
public:
    uint            m_index = 0;
    bool            m_isLogin = false;

    typedef void(CPlayer::*_RpcFunc)(NetPack&, NetPack&);
    static  _RpcFunc       _rpc[rpc_enum_cnt]; //自己实现的rpc
    Rpc_For_Client;
public:
    void    SendMsg(const NetPack& pack);
    uint64  CallRpc(RpcEnum rid, const ParseRpcParam& sendFun);
    void    CallRpc(RpcEnum rid, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun);
public:
    CPlayer();
    ~CPlayer();
    void UpdateNet();

private:
    void RunClientNet();
};