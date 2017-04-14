#pragma once
#include "../rpc/RpcEnum.h"
#include "../rpc/RpcQueue.h"
#include "../NetLib/client/ClientLink.h"

#undef Rpc_Declare
#undef Rpc_Realize
#define Rpc_Declare(typ) void HandleRpc_##typ(NetPack&);
#define Rpc_Realize(typ) void CrossAgent::HandleRpc_##typ(NetPack& recvBuf)

class NetPack;
class CrossAgent {
    ClientLinkConfig    _config;
    ClientLink          _netLink;
    static NetPack      _backBuffer;
public:
    typedef void(CrossAgent::*_RpcFunc)(NetPack&);
    static std::map<int, _RpcFunc>      _rpc; //自己实现的rpc
    Rpc_For_Cross;
public:
    void SendMsg(const NetPack& pack);
    NetPack& BackBuffer() { return _backBuffer; }
    int CallRpc(const char* name, const ParseRpcParam& sendFun);
    void CallRpc(const char* name, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun);
public:
    CrossAgent();
    static CrossAgent& Instance(){ static CrossAgent T; return T; }
    void RunClientIOCP();
};
#define sCrossAgent CrossAgent::Instance()
