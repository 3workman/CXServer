#include "stdafx.h"
#include "config_net.h"
#include "Player/Player.h"
#ifdef _USE_RAKNET
#include "raknet/server/UdpServer.h"
#elif defined(_USE_IOCP)
#include "iocp/server/ServLinkMgr.h"
#elif defined(_USE_HANDY)
#include "handy/server/TcpServer.h"
#endif
#include "iocp/client/ClientLink.h"
#include "Cross/CrossAgent.h"
#include "RakSleep.h"
#include "Service/ServiceMgr.h"
#include "Timer/TimerWheel.h"
#include "tool/GameApi.h"
#include "Buffer/NetPack.h"
#include "Log/LogFile.h"
#include "tool/UnitTest.h"
#include "tool/Input.h"

bool BindPlayerLink(void*& refPlayer, NetLinkPtr p, const void* pMsg, int size)
{
    NetPack msg(pMsg, size);
    switch (msg.OpCode()){
    case 1:
    case 2:
    {
        uint32 pid = msg.ReadUInt32();
        if (Player* player = Player::FindByPid(pid)) {
            player->SetNetLink(p);
            player->m_isLogin = true;
            refPlayer = player;
            printf("Player login idx(%d) pid(%d) \n", player->m_index, pid);
        } else {
            assert(0);
            return false;
        }
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
    sRpcClient._Handle((Player*)player, msg);
#elif defined(_USE_IOCP)
    sRpcClient.Insert((Player*)player, pMsg, size);
#elif defined(_USE_HANDY)
    sRpcClient.Insert((Player*)player, pMsg, size);
#endif
}
void ReportErrorMsg(void* pUser, int InvalidEnum, int nErrorCode, int nParam)
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
    LogFile log("log/battle", LogFile::TRACK, true);
    _LOG_MAIN_(log);

    // unit test
    unittest::UnitTest::RunAllTests();

    ClientLink::InitWinsock();
    sCrossAgent.RunClientIOCP();

    NetCfgServer cfg; //网络，初始化
#ifdef _USE_RAKNET
    UdpServer upd(cfg);
    upd.Start(BindPlayerLink, HandleClientMsg, ReportErrorMsg);
#elif defined(_USE_IOCP)
    ClientLink::InitWinsock();
    //sCrossAgent.RunClientIOCP();
    ServLinkMgr mgr(cfg);
    mgr.Start(BindPlayerLink, HandleClientMsg, ReportErrorMsg);
#elif defined(_USE_HANDY)
    TcpServer svr(cfg);
    svr.Start(BindPlayerLink, HandleClientMsg, ReportErrorMsg);
#endif

    uint64 timeOld(0), timeNow = GameApi::TimeMS();
    while (true) {
        timeOld = timeNow;
        timeNow = GameApi::TimeMS();
        uint time_elapse = uint(timeNow - timeOld);

        GameApi::RefreshTimeNow();
        ServiceMgr::RunAllService(time_elapse, timeNow);
        sTimerMgr.Refresh(time_elapse, timeNow);


#ifdef _USE_RAKNET  //网络，更新
        upd.Update();
#elif defined(_USE_IOCP)
        sRpcClient.Update();
        //sRpcCross.Update();
#elif defined(_USE_HANDY)
        sRpcClient.Update();
#endif

        uint tickDiff = uint(GameApi::TimeMS() - timeNow);
        if (tickDiff < 20) RakSleep(20 - tickDiff); //TODO: bug

        Input::CheckKeyboardInput();
    }
	return 0;
}