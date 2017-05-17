#include "stdafx.h"
#include "Player/Player.h"
#include "RakSleep.h"
#include "tool/thread.h"
#include "../NetLib/UdpClient/UdpClient.h"

void test_svr_battle(int playerCnt);

int main(int argc, char* argv[])
{
    LogFile log("log\\client", LogFile::DEBUG, true);
    _LOG_MAIN_(log);

    //test_svr_battle(200);

    Player player;
    player.CallRpc("rpc_login", [&](NetPack& buf){
        buf.WriteUInt32(1);
    });
    player.CallRpc("rpc_create_room", [&](NetPack& buf){
        buf.WriteFloat(1);
        buf.WriteFloat(1);
    });
    //CallRpc("rpc_logout", [&](NetPack& buf){
    //});

/*
    RunClientIOCP(g_link);
    ON_SCOPE_EXIT([&]{ g_link.CloseClient(0); });
    {
        // 立即发送一条数据，及时触发服务器端的AcceptExDoneIOCallback
        // 测试结果显示：客户端仅仅connect但不发送数据，不会触发服务器DoneIO回调
        // 真实环境下是会发数据的，不会只Connect
        // 客户端connect，三次握手成功后，在对端被放入“呼入连接请求队列”，尚未被用户进程接管，但client这边已经能发数据了
        NetPack msg(rpc_login, 0);
        g_link.SendMsg(msg.Buffer(), msg.Size());
    }
*/

    char tmpStr[32] = { 0 };
    while (true)
    {
        player.UpdateNet();

        cout << "请输入发送内容..." << endl;
        cin >> tmpStr;
        player.CallRpc("rpc_echo", [&](NetPack& buf){
            buf.WriteString(tmpStr); // buf << tmpStr;
        }, 
            [](NetPack& recvBuf){
            printf("Echo: %s\n", recvBuf.ReadString().c_str());
        });

        RakSleep(2000);
    }

    system("pause");
    return 0;
}

Thread* g_thread;
void test_svr_battle(int playerCnt)
{
    std::vector<Player> vec(playerCnt);

    char strbuff[1024] = { '\0' };
    while (true)
    {
        sRpcClientPlayer.Update();

        for (auto& it : vec)
        {
            it.UpdateNet();

            if (!it.m_isLogin) continue;

            it.CallRpc("rpc_echo", [&](NetPack& buf){
                int idx = sprintf(strbuff, "test svr_battle : PlayerIdx(%d)", it.m_index);
                string str(strbuff, idx);
                buf.WriteString(str);
            },
                [](NetPack& recvBuf){
                printf("Echo: %s\n", recvBuf.ReadString().c_str());
            });
        }

        RakSleep(33);
    }
}
