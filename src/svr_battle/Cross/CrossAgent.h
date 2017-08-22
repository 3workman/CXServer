#pragma once
#include "../rpc/RpcEnum.h"
#include "../rpc/RpcQueue.h"
#include "config_net.h"

#undef Rpc_Declare
#undef Rpc_Realize
#define Rpc_Declare(typ) void HandleRpc_##typ(NetPack&, NetPack&);
#define Rpc_Realize(typ) void CrossAgent::HandleRpc_##typ(NetPack& req, NetPack& ack)

class ClientLink;
class CrossAgent {
    NetCfgClient    _config;
    ClientLink*     _netLink = NULL;
    uint32          _connId = 0;
public:
    typedef void(CrossAgent::*_RpcFunc)(NetPack&, NetPack&);
    static std::map<int, _RpcFunc>      _rpc; //自己实现的rpc
    Rpc_For_Cross;
public:
    void SendMsg(const NetPack& pack);
    uint64 CallRpc(const char* name, const ParseRpcParam& sendFun);
    void CallRpc(const char* name, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun);
public:
    CrossAgent();
    ~CrossAgent();
    static CrossAgent& Instance(){ static CrossAgent T; return T; }
    void RunClientIOCP();
};
#define sCrossAgent CrossAgent::Instance()
