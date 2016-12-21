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
    cout << "———————————— RunClientIOCP ————————————" << endl;

    ClientLink::InitWinsock();
    link.CreateLinkAndConnect(HandleServerMsg);

    while (!link.IsConnect()) Sleep(1000); // 等待ConnectEx三次握手完成的回调，之后才能发数据
}
int _tmain(int argc, _TCHAR* argv[])
{
    RunClientIOCP(g_link);

    TestMsg msg; msg.msgId = Echo;
    // 立即发送一条数据，及时触发服务器端的AcceptExDoneIOCallback
    // 测试结果显示：客户端仅仅connect但不发送数据，不会触发服务器DoneIO回调
    // 真实环境下是会发数据的，不会只Connect
    // 客户端connect，三次握手成功后，在对端被放入“呼入连接请求队列”，尚未被用户进程接管，但client这边已经能发数据了
    strcpy_s(msg.data, "道友，你可听过醉寒江么？");
    g_link.SendMsg(&msg, msg.size());
    cout << "请输入发送内容..." << endl;

    while (true)
    {
        cin >> msg.data;
        //link.CloseClient(0); //测试关闭连接
        g_link.SendMsg(&msg, msg.size());
    }

    g_link.CloseClient(0);
	system("pause");
	return 0;
}