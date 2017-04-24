#include "stdafx.h"
#include "CrossAgent.h"

std::map<int, CrossAgent::_RpcFunc> CrossAgent::_rpc;

static CrossAgent* g_tmp_cross = NULL;
void HandleServerMsg(void* p, int size)
{
    sRpcCross.Insert(g_tmp_cross, p, size);
}
void CrossAgent::RunClientIOCP()
{
    g_tmp_cross = this;
    _netLink.CreateLinkAndConnect(HandleServerMsg);

    while (!_netLink.IsClose() && !_netLink.IsConnect()) Sleep(200); // 等待ConnectEx三次握手完成的回调，之后才能发数据
}
CrossAgent::CrossAgent()
: _netLink(_config)
{
    if (_rpc.empty())
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpc[sRpcCross.RpcNameToId(#typ)] = &CrossAgent::HandleRpc_##typ;
        Rpc_For_Cross;
    }

    _config.wServerPort = 7003; //go cross
}
uint64 CrossAgent::CallRpc(const char* name, const ParseRpcParam& sendFun)
{
    return sRpcCross._CallRpc(name, sendFun, std::bind(&CrossAgent::SendMsg, this, std::placeholders::_1));
}
void CrossAgent::CallRpc(const char* name, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun)
{
    uint64 reqKey = CallRpc(name, sendFun);

    sRpcCross.RegistResponse(reqKey, recvFun);
}
void CrossAgent::SendMsg(const NetPack& pack)
{
    _netLink.SendMsg(pack.Buffer(), pack.Size());
}

//////////////////////////////////////////////////////////////////////////
// rpc
Rpc_Realize(rpc_echo)
{
    string str = recvBuf.ReadString();
    printf("Echo: %s\n", str.c_str());

    //NetPack& backBuffer = BackBuffer();
    //backBuffer << str;
}
