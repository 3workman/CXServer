#include "stdafx.h"
#include "..\NetLib\client\ClientLink.h"
#include "..\NetLib\UdpClient\UdpClient.h"
#include "Buffer\NetPack.h"
#include "..\rpc\RpcEnum.h"
#include <functional>
#include "tool\thread.h"


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
    while (WAIT_TIMEOUT == g_thread->WaitKillEvent(50))
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



typedef std::function<void(NetPack&)>   SendRpcParam;
typedef std::function<void(NetPack&)>   RecvRpcParam;
std::map<int, RecvRpcParam> g_RpcHandle;     //TODO:zhoumf：本模块实现的rpc，由静态声明决定，参见RpcQueue::_func
std::map<int, RecvRpcParam> g_RecvHandle;    //挂接Socket Link的回包处理函数集，由g_RpcHandle、CallRpc()动态加入的lambda两大块构成
void CallRpc(uint16 opCode, const SendRpcParam& func)
{
    //“同名rpc混乱”：client rpc server且有回包；若server那边也有个同名rpc client，那client就不好区分底层收到的包，是自己rpc的回复，还是对方主动rpc
    //远程调用其它模块的rpc，应是本模块未声明实现的。避免同名rpc的混乱
    //讲rpc底层的回包，设计成带类型的（主动发、被动回），能解决“同名rpc混乱”问题，但觉用处不大，就先免掉
    assert(g_RpcHandle.find(opCode) == g_RpcHandle.end());

    NetPack msg(opCode);
    func(msg);
    g_link.SendMsg(msg.Buffer(), msg.Size());
}
void CallRpc(uint16 opCode, const SendRpcParam& fun1, const RecvRpcParam& fun2)
{
    CallRpc(opCode, fun1);

    g_RecvHandle.insert(make_pair(opCode, fun2));
}
void UpdateRecvQueue()
{
    NetPack* pData;
    if (g_queue.pop(pData))
    {
        auto it = g_RecvHandle.find(pData->GetOpcode());
        if (it != g_RecvHandle.end()) it->second(*pData);
        delete pData;
    }
}


int _tmain(int argc, _TCHAR* argv[])
{
    RunClientUdp();
    ON_SCOPE_EXIT([]{ g_link.Stop(); });
    CallRpc(1, [&](NetPack& buf){
        buf.WriteUInt32(1);
    });
    CallRpc(10, [&](NetPack& buf){
        buf.WriteFloat(1);
        buf.WriteFloat(1);
    });

    CallRpc(2, [&](NetPack& buf){
    });
    g_link.CloseLink();

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

        CallRpc(0, [&](NetPack& buf){
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