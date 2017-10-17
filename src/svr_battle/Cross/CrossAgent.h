#pragma once
#include "RpcEnum.h"
#include "RpcQueue.h"
#include "config_net.h"

#ifdef _WIN32
class ClientLink;   typedef ClientLink  TcpClient;
#else
class TcpClient;
#endif

#undef Rpc_Declare
#undef Rpc_Realize
#define Rpc_Declare(typ) void HandleRpc_##typ(NetPack&, NetPack&);
#define Rpc_Realize(typ) void CrossAgent::HandleRpc_##typ(NetPack& req, NetPack& ack)

class CrossAgent {
    NetCfgClient    _config;
    TcpClient*      _netLink = NULL;
    uint32          _connId = 0;
public:
    typedef void(CrossAgent::*_RpcFunc)(NetPack&, NetPack&);
    static  _RpcFunc          _rpc[RpcEnumCnt]; //自己实现的rpc
    Rpc_For_Cross;
public:
    void    SendMsg(const NetPack& pack);
    uint64  CallRpc(RpcEnum rid, const ParseRpcParam& sendFun);
    void    CallRpc(RpcEnum rid, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun);
public:
    static CrossAgent& Instance() { static CrossAgent T; return T; }
    CrossAgent();
    ~CrossAgent();
    void RunClient();
private:
    void _OnConnect();
};
#define sCrossAgent CrossAgent::Instance()
