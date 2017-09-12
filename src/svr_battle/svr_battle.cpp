#include "stdafx.h"
#include "config_net.h"
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
    //Notice��RakNet����Update����������հ������̵߳ġ�������ֱ�ӵ�����Ӧ����
    //Notice��IOCP�Ƕ��߳��գ����ȷŽ���ѭ�����У�mainloopʱ����Ӧ
#ifdef _USE_RAKNET
    NetPack msg(pMsg, size);
    sRpcClient._Handle((Player*)player, msg);
#elif defined(_USE_IOCP) || defined(_USE_HANDY) || defined(_USE_LIBEVENT)
    sRpcClient.Insert((Player*)player, pMsg, size);
#endif
}
void ReportErrorMsg(void* pUser, int InvalidEnum, int nErrorCode, int nParam)
{
    if (Player* player = (Player*)pUser)
    {
        //TODO:������Ը��ݴ����������ִ������磺�İ���ֱ���ߵ�
        if (player->m_isLogin) {
            player->m_isLogin = false;
            sTimerMgr.AddTimer([=]{
                if (!player->m_isLogin) delete player;
            }, 60); //�ڼ������������
        } else
            delete player;
    }
}

int main(int argc, char* argv[])
{
    setvbuf(stdout, NULL, _IOLBF, 2); //����stdout�Ļ�������Ϊ�л��壬�ض����ļ����Ѻ�

    LogFile log("log/battle", LogFile::TRACK, true);
    _LOG_MAIN_(log);

    // unit test
    unittest::UnitTest::RunAllTests();


    NetCfgServer cfg; //���磬��ʼ��
#ifdef _USE_RAKNET
    UdpServer svr(cfg);
#elif defined(_USE_IOCP)
    ServLinkMgr svr(cfg);
#elif defined(_USE_HANDY) || defined(_USE_LIBEVENT)
    TcpServer svr(cfg);
#endif
    svr.Start(BindPlayerLink, HandleClientMsg, ReportErrorMsg);

    sCrossAgent.RunClient();

    uint64 timeOld(0), timeNow = GameApi::TimeMS();
    while (true) {
        timeOld = timeNow;
        timeNow = GameApi::TimeMS();
        uint time_elapse = uint(timeNow - timeOld);

        GameApi::RefreshTimeNow();
        ServiceMgr::RunAllService(time_elapse, timeNow);
        sTimerMgr.Refresh(time_elapse, timeNow);


#ifdef _USE_RAKNET  //���磬����
        svr.Update();
#elif defined(_USE_IOCP) || defined(_USE_HANDY) || defined(_USE_LIBEVENT)
        sRpcClient.Update();
#endif
        sRpcCross.Update();

        uint tickDiff = uint(GameApi::TimeMS() - timeNow);
        if (tickDiff < 20) RakSleep(20 - tickDiff); //TODO: bug

        Input::CheckKeyboardInput();
    }
	return 0;
}