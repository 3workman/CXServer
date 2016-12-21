#include "stdafx.h"
#include "..\NetLib\client\ClientLink.h"
#include "..\msg\MsgEnum.h"
#include "..\msg\TestMsg.h"


ClientLinkConfig config;
ClientLink g_link(config);

void HandleServerMsg(void* p, DWORD size)
{
    stMsg* pMsg = (stMsg*)p;
    if (pMsg->msgId == Echo)
    {
        printf("Echo: %s\n", ((char*)pMsg) + 4);
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

    TestMsg msg; msg.msgId = Echo;
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