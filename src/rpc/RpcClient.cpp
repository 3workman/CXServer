#include "stdafx.h"
#include "RpcClient.h"
#ifdef _WIN32
#include "iocp/client/ClientLink.h"
#else
//#include "handy/client/TcpClient.h"
#include "libevent/client/TcpClient.h"
#endif

RpcClient::_RpcFunc  RpcClient::_rpcfunc[RpcEnumCnt] = { 0 };
RpcQueue<RpcClient>& RpcClient::_rpc = RpcQueue<RpcClient>::Instance();

void RpcClient::RunClient()
{
    _netLink->CreateLinkAndConnect([&]() {
        _OnConnect();
    }, [&](const void* p, int size) {
        _rpc.Insert(this, p, size);
    });
}
void RpcClient::_OnConnect() //Notice：IO线程调用的，不是主逻辑线程，有线程安全风险
{
    //这里不能用CallRpc，它不是线程安全的
    _netLink->SendMsg(&_connId, sizeof(_connId)); //第一条消息：上报connId
    NetPack regMsg(16);
    regMsg.OpCode(rpc_regist);
    NetMeta::G_Local_Meta->DataToBuf(regMsg);
    SendMsg(regMsg);
}

RpcClient::RpcClient()
{
    _netLink = new TcpClient(_config);

#undef Rpc_Declare
#define Rpc_Declare(typ) _rpcfunc[typ] = &RpcClient::HandleRpc_##typ;
    Rpc_Declare(rpc_svr_accept)
}
RpcClient::~RpcClient()
{
    delete _netLink;
}
uint64 RpcClient::CallRpc(RpcEnum rid, const ParseRpcParam& sendFun)
{
    return _rpc._CallRpc(rid, sendFun, std::bind(&RpcClient::SendMsg, this, std::placeholders::_1));
}
void RpcClient::CallRpc(RpcEnum rid, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun)
{
    uint64 reqKey = CallRpc(rid, sendFun);

    _rpc.RegistResponse(reqKey, recvFun);
}
void RpcClient::SendMsg(const NetPack& pack)
{
    _netLink->SendMsg(pack.contents(), pack.size());
}

//////////////////////////////////////////////////////////////////////////
// rpc
#undef Rpc_Realize
#define Rpc_Realize(typ) void RpcClient::HandleRpc_##typ(NetPack& req, NetPack& ack)

Rpc_Realize(rpc_svr_accept)
{
    _connId = req.ReadUInt32();
}