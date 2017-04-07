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
    cout << "������������������������ RunClientIOCP ������������������������" << endl;

    ClientLink::InitWinsock();
    link.CreateLinkAndConnect(HandleServerMsg);

    while (!link.IsClose() && !link.IsConnect()) Sleep(1000); // �ȴ�ConnectEx����������ɵĻص���֮����ܷ�����
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
std::map<int, RecvRpcParam> g_RpcHandle;     //TODO:zhoumf����ģ��ʵ�ֵ�rpc���ɾ�̬�����������μ�RpcQueue::_func
std::map<int, RecvRpcParam> g_RecvHandle;    //�ҽ�Socket Link�Ļذ�������������g_RpcHandle��CallRpc()��̬�����lambda����鹹��
void CallRpc(uint16 opCode, const SendRpcParam& func)
{
    //��ͬ��rpc���ҡ���client rpc server���лذ�����server�Ǳ�Ҳ�и�ͬ��rpc client����client�Ͳ������ֵײ��յ��İ������Լ�rpc�Ļظ������ǶԷ�����rpc
    //Զ�̵�������ģ���rpc��Ӧ�Ǳ�ģ��δ����ʵ�ֵġ�����ͬ��rpc�Ļ���
    //��rpc�ײ�Ļذ�����Ƴɴ����͵ģ��������������أ����ܽ����ͬ��rpc���ҡ����⣬�����ô����󣬾������
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