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
    cout << "������������������������ RunClientIOCP ������������������������" << endl;

    ClientLink::InitWinsock();
    link.CreateLinkAndConnect(HandleServerMsg);

    while (!link.IsClose() && !link.IsConnect()) Sleep(1000); // �ȴ�ConnectEx����������ɵĻص���֮����ܷ�����
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
        // ��������һ�����ݣ���ʱ�����������˵�AcceptExDoneIOCallback
        // ���Խ����ʾ���ͻ��˽���connect�����������ݣ����ᴥ��������DoneIO�ص�
        // ��ʵ�������ǻᷢ���ݵģ�����ֻConnect
        // �ͻ���connect���������ֳɹ����ڶԶ˱����롰��������������С�����δ���û����̽ӹܣ���client����Ѿ��ܷ�������
        NetPack msg(rpc_login, 0);
        g_link.SendMsg(msg.Buffer(), msg.Size());
    }*/

    cout << "�����뷢������..." << endl;
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