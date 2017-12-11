#include "stdafx.h"
#include "Player/Player.h"
#ifdef _USE_RAKNET
#include "raknet/server/UdpServer.h"
#elif defined(_USE_IOCP)
#include "iocp/server/ServLinkMgr.h"
#elif defined(_USE_HANDY)
#include "handy/server/TcpServer.h"
#elif defined(_USE_LIBEVENT)
#include "libevent/server/TcpServer.h"
#endif
#include "Zookeeper/Zookeeper.h"
#include "RakSleep.h"
#include "Service/ServiceMgr.h"
#include "Timer/TimerWheel.h"
#include "tool/GameApi.h"
#include "Buffer/NetPack.h"
#include "Player/Player.h"
#include "Log/LogFile.h"
#include "tool/Input.h"
#include "Config/LoadCsv.hpp"

bool BindPlayerLink(void*& refPlayer, NetLinkPtr p, const void* pMsg, int size)
{
    NetPack msg(pMsg, size);
    switch (msg.OpCode()) {
    case rpc_battle_login:
    case rpc_battle_reconnect:
    {
        uint32 pid = msg.ReadUInt32();
        Player* player = Player::FindByPid(pid);
        if (player == NULL) player = new Player(pid);
        player->SetNetLink(p);
        player->m_isLogin = true;
        refPlayer = player;
    } return true;
    default: assert(0); return false;
    }
}

void HandleClientMsg(void* player, const void* pMsg, int size)
{
    //Notice：RakNet是在Update里逐个处理收包，单线程的。所以能直接调用响应函数
    //Notice：IOCP是多线程收，须先放进主循环队列，mainloop时再响应
#ifdef _USE_RAKNET
    NetPack msg(pMsg, size);
    Player::_rpc._Handle((Player*)player, msg);
#elif defined(_USE_IOCP) || defined(_USE_HANDY) || defined(_USE_LIBEVENT)
    Player::_rpc.Insert((Player*)player, pMsg, size);
#endif
}

void ReportErrorMsg(void* pUser, int InvalidEnum, int nErrorCode)
{
    if (Player* player = (Player*)pUser)
    {
        //TODO:这里可以根据错误类型区分处理，比如：改包的直接踢掉
        if (player->m_isLogin) {
            player->m_isLogin = false;
            sTimerMgr.AddTimer([=]{
                if (!player->m_isLogin) delete player;
            }, 60); //期间允许断线重连
        } else
            delete player;
    }
}

int main(int argc, char* argv[])
{
    setvbuf(stdout, NULL, _IOLBF, 2); //设置stdout的缓冲类型为行缓冲，重定向文件更友好

    LogFile log("log/battle", LogFile::TRACK);
    _LOG_MAIN_(log);

    LoadAllCsv();
    NetMeta::G_Local_Meta = NetMeta::GetMeta("battle");

    // unit test
    //TestLua2(L);
    //TestLua3(L);
    //testing::InitGoogleTest(&argc, argv);
    //if (RUN_ALL_TESTS() == 0)
    //    printf("All tests pased\n");
    //else
    //    printf("some tests failed\n");

    NetCfgServer cfg; //网络，初始化
#ifdef _USE_RAKNET
    UdpServer svr(cfg);
#elif defined(_USE_IOCP)
    ServLinkMgr svr(cfg);
#elif defined(_USE_HANDY) || defined(_USE_LIBEVENT)
    TcpServer svr(cfg);
#endif
    svr.Start(BindPlayerLink, HandleClientMsg, ReportErrorMsg);

    //sZookeeper.RunClient(); //注册到Zookeeper

    uint64 timeOld(0), timeNow = GameApi::TimeMS();
    while (true) {
        timeOld = timeNow;
        timeNow = GameApi::TimeMS();
        uint time_elapse = uint(timeNow - timeOld);
        
        GameApi::RefreshTimeSecond();
        ServiceMgr::RunAllService(time_elapse, timeNow);
        sTimerMgr.Refresh(time_elapse, timeNow);

#ifdef _USE_RAKNET  //网络，更新
        svr.Update();
#elif defined(_USE_IOCP) || defined(_USE_HANDY) || defined(_USE_LIBEVENT)
        Player::_rpc.Update();
#endif
        RpcClient::_rpc.Update();

        uint tickDiff = uint(GameApi::TimeMS() - timeNow);
        if(tickDiff < 20) RakSleep(20 - tickDiff); // TODO: bug

        Input::CheckKeyboardInput();
    }
    return 0;
}
