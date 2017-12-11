#pragma once
#include "RpcEnum.h"
#include "RpcQueue.h"
#include "config_net.h"

#ifdef _WIN32
class ClientLink;   typedef ClientLink  TcpClient;
#else
class TcpClient;
#endif

class RpcClient {
protected:
    NetCfgClient    _config;
    TcpClient*      _netLink = NULL;
    uint32          _connId = 0;
public:
    typedef void(RpcClient::*_RpcFunc)(NetPack&, NetPack&);
    static  _RpcFunc         _rpcfunc[RpcEnumCnt]; //自己实现的rpc
    static  RpcQueue<RpcClient>& _rpc;
    Rpc_Declare(rpc_svr_accept)
public:
    void    SendMsg(const NetPack& pack);
    uint64  CallRpc(RpcEnum rid, const ParseRpcParam& sendFun);
    void    CallRpc(RpcEnum rid, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun);
public:
    RpcClient();
    virtual ~RpcClient();
    void RunClient();
protected:
    virtual void _OnConnect(); //Notice：IO线程调用的，不是主逻辑线程，有线程安全风险
};