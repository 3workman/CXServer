#include "stdafx.h"
#ifdef _USE_UDP
#include "../NetLib/UdpServer/UdpServer.h"
#else
#include "../NetLib/server/define.h"
#include "../NetLib/server/ServLinkMgr.h"
#endif
#include "../NetLib/client/ClientLink.h"
#include "RakSleep.h"
#include "Service/ServiceMgr.h"
#include "Timer/TimerWheel.h"
#include "tool/GameApi.h"
#include "Buffer/NetPack.h"
#include "Player/Player.h"
#include "Log/LogFile.h"
#include "Cross/CrossAgent.h"

bool BindPlayerLink(void*& refPlayer, NetLink* p, const void* pMsg, int size)
{
    NetPack msg(pMsg, size);
    switch (msg.OpCode()){
    case 1:
    case 2:
    {
        uint32 idx, pid;
        msg >> idx >> pid;
        if (Player* player = Player::FindByIdx(idx)) {
            if (player->m_pid == pid) {
                player->SetNetLink(p);
                player->m_isLogin = true;
                refPlayer = player;
            }
        }
    } return true;
    default: assert(0); return false;
    }
}
void HandleClientMsg(void* player, const void* pMsg, int size)
{
    //Notice��RakNet����Update����������հ������̵߳ġ�������ֱ�ӵ�����Ӧ����
    //Notice��IOCP�Ƕ��߳��գ����ȷŽ���ѭ�����У�mainloopʱ����Ӧ
#ifdef _USE_UDP
    NetPack msg(pMsg, size);
    sRpcClient._Handle((Player*)player, msg);
#else
    sRpcClient.Insert((Player*)player, pMsg, size);
#endif
}
void ReportErrorMsg(void* pUser, int InvalidEnum, int nErrorCode, int nParam)
{
    if (Player* player = (Player*)pUser)
    {
        player->SetNetLink(NULL);

        //TODO:������Ը��ݴ����������ִ������磺�İ���ֱ���ߵ�
        if (player->m_isLogin) {
            player->m_isLogin = false;
            sTimerMgr.AddTimer([=]{
                if (!player->m_isLogin) delete player;
            }, 10); //�ڼ������������
        } else
            delete player;
    }
}

int main(int argc, char* argv[])
{
    LogFile log("log\\battle", LogFile::TRACK, true);
    _LOG_MAIN_(log);

    ClientLink::InitWinsock();
    sCrossAgent.RunClientIOCP();

#ifdef _USE_UDP
    UdpServer upd;
    upd.Start(BindPlayerLink, HandleClientMsg, ReportErrorMsg);
#else
    ServerConfig config;
    ServLinkMgr mgr(config);
    mgr.CreateServer(BindPlayerLink, HandleClientMsg, ReportErrorMsg);
#endif

    uint time_elapse(0), timeOld(0), timeNow = GetTickCount();
    while (true) {
        timeOld = timeNow;
        timeNow = GetTickCount();
        time_elapse = timeNow - timeOld;

        GameApi::RefreshTimeNow();
        ServiceMgr::RunAllService(time_elapse, timeNow);
        sTimerMgr.Refresh(time_elapse, timeNow);
        sRpcCross.Update();

#ifdef _USE_UDP
        upd.Update();
#else
        sRpcClient.Update();
#endif
        RakSleep(30);
    }
	return 0;
}