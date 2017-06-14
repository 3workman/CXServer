#pragma once
#include "../rpc/RpcEnum.h"
#include "../rpc/RpcQueue.h"
#include "../NetLib/client/config_client.h"

#undef Rpc_Declare
#undef Rpc_Realize
#define Rpc_Declare(typ) void HandleRpc_##typ(NetPack&, NetPack&);
#define Rpc_Realize(typ) void CrossAgent::HandleRpc_##typ(NetPack& req, NetPack& ack)

class ClientLink;
class CrossAgent {
    ClientLinkConfig    _config;
    ClientLink*         _netLink = NULL;
    NetPack             _first_buf; // 连接建立后的第一个包，上报connId、密钥等
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
