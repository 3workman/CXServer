#pragma once
#include "../rpc/RpcEnum.h"
#include "../rpc/RpcQueue.h"
#include "../NetLib/config_net.h"

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
    uint32          m_pid = 0;
    bool            m_isLogin = false;

    typedef void(CPlayer::*_RpcFunc)(NetPack&, NetPack&);
    static std::map<int, _RpcFunc>      _rpc; //自己实现的rpc
    Rpc_For_Client;
public:
    void SendMsg(const NetPack& pack);
    uint64 CallRpc(const char* name, const ParseRpcParam& sendFun);
    void CallRpc(const char* name, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun);
public:
    CPlayer();
    ~CPlayer();
    void UpdateNet();
};