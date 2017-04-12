#include "stdafx.h"
#include "../NetLib/client/ClientLink.h"
#include "../NetLib/UdpClient/UdpClient.h"
#include "Buffer/NetPack.h"
#include "../rpc/RpcEnum.h"
#include <functional>
#include "tool/thread.h"
#include "../rpc/RpcQueue.h"
#include "Csv/CSVparser.hpp"


ClientLinkConfig config;
//ClientLink g_link(config);
UdpClient  g_link;

SafeQueue<NetPack*>  g_queue;

Thread* g_thread;

void HandleServerMsg(void* p, int size)
{
    g_queue.push(new NetPack(p, size));
}
void RunClientIOCP(ClientLink& link)
{
    cout << "———————————— RunClientIOCP ————————————" << endl;

    ClientLink::InitWinsock();
    link.CreateLinkAndConnect(HandleServerMsg);

    while (!link.IsClose() && !link.IsConnect()) Sleep(1000); // 等待ConnectEx三次握手完成的回调，之后才能发数据
}

static void AssistLoop(LPVOID pParam)
{
    while (cv_status::timeout == g_thread->WaitKillEvent(50))
    {
        g_link.Update();
    }
}
void RunClientUdp()
{
    g_link.Start(HandleServerMsg);

    g_thread = new Thread;
    g_thread->RunThread(AssistLoop, 0);

    while (!g_link.IsClose() && !g_link.IsConnect()) Sleep(1000);
}

std::map<std::string, int> g_rpc_table;
std::map<int, RecvRpcParam> g_RecvHandle;
int CallRpc(const char* name, const SendRpcParam& func)
{
    int opCodeId = g_rpc_table[name];
    assert(opCodeId > 0);
    static NetPack msg(0);
    msg.ClearBody();
    msg.SetOpCode(opCodeId);;
    func(msg);
    g_link.SendMsg(msg.Buffer(), msg.Size());
    return opCodeId;
}
void CallRpc(const char* name, const SendRpcParam& fun1, const RecvRpcParam& fun2)
{
    int opCodeId = CallRpc(name, fun1);

    g_RecvHandle.insert(make_pair(opCodeId, fun2));
}
void UpdateRecvQueue()
{
    NetPack* pData;
    if (g_queue.pop(pData)) {
        auto it = g_RecvHandle.find(pData->GetOpcode());
        if (it != g_RecvHandle.end()) it->second(*pData);
        delete pData;
    }
}


int _tmain(int argc, _TCHAR* argv[])
{
    csv::Parser file = csv::Parser("../data/csv/rpc.csv");
    uint cnt = file.rowCount();
    for (uint i = 0; i < cnt; ++i) {
        csv::Row& row = file[i];
        g_rpc_table[row["name"]] = atoi(row["id"].c_str());
    }


    RunClientUdp();
    ON_SCOPE_EXIT([]{ g_link.Stop(); });
    CallRpc("rpc_login", [&](NetPack& buf){
        buf.WriteUInt32(1);
    });
    CallRpc("rpc_create_room", [&](NetPack& buf){
        buf.WriteFloat(1);
        buf.WriteFloat(1);
    });

    //CallRpc("rpc_logout", [&](NetPack& buf){
    //});
    //g_link.CloseLink();

/*
    RunClientIOCP(g_link);
    ON_SCOPE_EXIT([]{ g_link.CloseClient(0); });
    {
        // 立即发送一条数据，及时触发服务器端的AcceptExDoneIOCallback
        // 测试结果显示：客户端仅仅connect但不发送数据，不会触发服务器DoneIO回调
        // 真实环境下是会发数据的，不会只Connect
        // 客户端connect，三次握手成功后，在对端被放入“呼入连接请求队列”，尚未被用户进程接管，但client这边已经能发数据了
        NetPack msg(rpc_login, 0);
        g_link.SendMsg(msg.Buffer(), msg.Size());
    }*/

    cout << "请输入发送内容..." << endl;
    char tmpStr[32] = { 0 };
    while (true)
    {
        cin >> tmpStr;

        CallRpc("rpc_echo", [&](NetPack& buf){
            buf.WriteString(tmpStr); // buf << tmpStr;
        }, 
            [](NetPack& recvBuf){
            printf("Echo: %s\n", recvBuf.ReadString().c_str());
        });

        Sleep(2000);
        UpdateRecvQueue();
    }

    system("pause");
    return 0;
}