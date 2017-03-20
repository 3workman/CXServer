#include "stdafx.h"
#include "..\NetLib\client\ClientLink.h"
#include "..\msg\MsgEnum.h"
#include "..\msg\TestMsg.h"
#include "..\msg\LoginMsg.h"
#include "Buffer\NetPack.h"
#include "..\rpc\RpcEnum.h"
#include <functional>


ClientLinkConfig config;
ClientLink g_link(config);

SafeQueue<NetPack*>  g_queue;

/*
void HandleServerMsg(void* p, DWORD size)
{
    switch (((stMsg*)p)->msgId) {
    case C2S_Echo:
    {
        TestMsg* msg = (TestMsg*)p;
        printf("Echo: %s\n", msg->data);
    }
    break;
    default:break;
    }
}
void RunClientIOCP(ClientLink& link)
{
    cout << "������������������������ RunClientIOCP ������������������������" << endl;

    ClientLink::InitWinsock();
    link.CreateLinkAndConnect(HandleServerMsg);

    while (!link.IsConnect()) Sleep(1000); // �ȴ�ConnectEx����������ɵĻص���֮����ܷ�����
}
int _tmain(int argc, _TCHAR* argv[])
{
    RunClientIOCP(g_link);
    {
        LoginMsg msg; msg.msgId = C2S_Login;
        g_link.SendMsg(&msg, sizeof(msg));
    }
    TestMsg msg; msg.msgId = C2S_Echo;
    // ��������һ�����ݣ���ʱ�����������˵�AcceptExDoneIOCallback
    // ���Խ����ʾ���ͻ��˽���connect�����������ݣ����ᴥ��������DoneIO�ص�
    // ��ʵ�������ǻᷢ���ݵģ�����ֻConnect
    // �ͻ���connect���������ֳɹ����ڶԶ˱����롰��������������С�����δ���û����̽ӹܣ���client����Ѿ��ܷ�������
    strcpy_s(msg.data, "���ѣ������������ô��");
    g_link.SendMsg(&msg, msg.size());
    cout << "�����뷢������..." << endl;

    while (true)
    {
        cin >> msg.data;
        //link.CloseClient(0); //���Թر�����
        g_link.SendMsg(&msg, msg.size());
    }

    g_link.CloseClient(0);
	system("pause");
	return 0;
}
*/


void HandleServerMsg(void* p, DWORD size)
{
    NetPack& recvBuf = *new NetPack(p, size);
    g_queue.push(new NetPack(p, size));
}
void RunClientIOCP(ClientLink& link)
{
    cout << "������������������������ RunClientIOCP ������������������������" << endl;

    ClientLink::InitWinsock();
    link.CreateLinkAndConnect(HandleServerMsg);

    while (!link.IsConnect()) Sleep(1000); // �ȴ�ConnectEx����������ɵĻص���֮����ܷ�����
}

typedef std::function<void(NetPack&)>   WriteRpcParam;
typedef std::function<void(NetPack&)>   ReadRpcBack;
std::map<int, ReadRpcBack> g_RpcHandle;     //TODO:zhoumf����ģ��ʵ�ֵ�rpc���ɾ�̬�����������μ�RpcQueue::_func
std::map<int, ReadRpcBack> g_RecvHandle;    //�ҽ�Socket Link�Ļذ�������������g_RpcHandle��CallRpc()��̬�����lambda����鹹��
void CallRpc(uint16 opCode, WriteRpcParam func)
{
    //��ͬ��rpc���ҡ���client rpc server���лذ�����server�Ǳ�Ҳ�и�ͬ��rpc client����client�Ͳ������ֵײ��յ��İ������Լ�rpc�Ļظ������ǶԷ�����rpc
    //Զ�̵�������ģ���rpc��Ӧ�Ǳ�ģ��δ����ʵ�ֵġ�����ͬ��rpc�Ļ���
    //��rpc�ײ�Ļذ�����Ƴɴ����͵ģ��������������أ����ܽ����ͬ��rpc���ҡ����⣬�����ô����󣬾������
    assert(g_RpcHandle.find(opCode) == g_RpcHandle.end());

    NetPack msg(opCode);
    func(msg);
    g_link.SendMsg(msg.Buffer(), msg.Size());
}
void CallRpc(uint16 opCode, WriteRpcParam fun1, ReadRpcBack fun2)
{
    CallRpc(opCode, fun1);

    g_RecvHandle.insert(make_pair(opCode, fun2));
}
void HandleRecvQueue()
{
    NetPack* pData;
    if (g_queue.pop(pData))
    {
        g_RecvHandle[pData->GetOpcode()](*pData);
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    RunClientIOCP(g_link);
    {
        NetPack msg(rpc_login, 0);
        g_link.SendMsg(msg.Buffer(), msg.Size());
    }

    cout << "�����뷢������..." << endl;

    char tmpStr[32] = { 0 };
    while (true)
    {
        cin >> tmpStr;

        CallRpc(rpc_echo, [&](NetPack& buf){
            buf << tmpStr;
        }, 
            [](NetPack& recvBuf){
            printf("Echo: %s\n", recvBuf.ReadString().c_str());
        });

        Sleep(2000);
        HandleRecvQueue();
    }

    g_link.CloseClient(0);
    system("pause");
    return 0;
}