#pragma once
#include "RpcEnum.h"
#include "RpcQueue.h"
#include "config_net.h"

class NetPack;
class UdpClient;
class CPlayer : boost::noncopyable {
    NetCfgClient    _netCfg;
    UdpClient*      _netLink = NULL;
public:
    uint            m_index = 0;
    bool            m_isLogin = false;

    typedef void(CPlayer::*_RpcFunc)(NetPack&, NetPack&);
    static  _RpcFunc       _rpcfunc[RpcEnumCnt]; //自己实现的rpc
    static  RpcQueue<CPlayer>& _rpc;
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