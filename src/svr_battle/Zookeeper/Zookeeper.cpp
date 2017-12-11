#include "stdafx.h"
#include "Cross/CrossAgent.h"
#include "Zookeeper.h"

Zookeeper::Zookeeper()
{
    if (!_rpcfunc[rpc_svr_accept])
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpcfunc[typ] = (RpcClient::_RpcFunc)&Zookeeper::HandleRpc_##typ;
        Rpc_For_Zookeeper;
    }

    auto ptr = NetMeta::GetMeta("zookeeper");
    _config.svrIp = ptr->ip.c_str();
    _config.svrPort = ptr->tcp_port;
}

void Zookeeper::_OnConnect()
{
    RpcClient::_OnConnect();

    //Notice：CallRpc非线程安全的，但Zookeeper连接只会调一次，实际没竞态
    CallRpc(rpc_zoo_register, [&](NetPack& buf) {
        buf.WriteString(NetMeta::G_Local_Meta->module);
        buf.WriteInt32(NetMeta::G_Local_Meta->svr_id);
    }, [&](NetPack& recvBuf) {
        //rpc回复都是由主线程处理的，这里无竞态
        NetMeta meta;
        int cnt = recvBuf.ReadInt32();
        for (int i = 0; i < cnt; ++i) {
            meta.BufToData(recvBuf);
            NetMeta::AddMeta(meta);

            if (meta.module == "cross") {
                auto ptr = std::make_shared<CrossAgent>();
                m_nodes.push_back(ptr);
                ptr->RunClient();
            }
        }
    });
}

//////////////////////////////////////////////////////////////////////////
// rpc
#undef Rpc_Realize
#define Rpc_Realize(typ) void Zookeeper::HandleRpc_##typ(NetPack& req, NetPack& ack)

Rpc_Realize(rpc_svr_node_join)//有服务器节点加入，连接之
{
    NetMeta meta;
    meta.BufToData(req);
    NetMeta::AddMeta(meta);

    if (meta.module == "cross") {
        auto ptr = std::make_shared<CrossAgent>();
        m_nodes.push_back(ptr);
        ptr->RunClient();
    }
}
