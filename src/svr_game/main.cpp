#include "stdafx.h"
#include "..\NetLib\server\ServLinkMgr.h"
#include "..\NetLib\server\define.h"
#include "Player\MsgPool.h"
#include "Player\Player.h"
#include "Service\ServiceMgr.h"
#include "Timer\TimerWheel.h"
#include "tool\GameApi.h"
#include "..\msg\LoginMsg.h"


void BindPlayerLink(void*& refPlayer, ServLink* p, void* pMsg)
{
    switch (((stMsg*)pMsg)->msgId){
    case C2S_Login:
    {
        refPlayer = new Player(p);
    }
    break;
    case C2S_ReConnect:
    {
        ReConnectMsg* msg = (ReConnectMsg*)pMsg;
        if (Player* player = Player::FindByIdx(msg->playerIdx))
        {
            player->SetServLink(p);
        }
    }
    break;
    default: assert(0); break;
    }
}
void HandleClientMsg(void* player, void* pMsg, DWORD size)
{
    sMsgPool.Insert((Player*)player, (stMsg*)pMsg, size);
}
void ReportErrorMsg(void* player, int InvalidEnum, int nErrorCode, int nParam)
{
    if (player)
    {
    }
}
void RunServerIOCP(ServLinkMgr& mgr)
{
    cout << "！！！！！！！！！！！！ RunServerIOCP ！！！！！！！！！！！！" << endl;
    ServLinkMgr::InitWinsock();
    mgr.CreateServer(BindPlayerLink, HandleClientMsg, ReportErrorMsg);
}
int _tmain(int argc, _TCHAR* argv[])
{
    ServerConfig config;
    ServLinkMgr mgr(config);
    RunServerIOCP(mgr);

    DWORD timeOld(0), timeNow = GetTickCount();
    while (true) {
        timeOld = timeNow;
        timeNow = GetTickCount();

        ServiceMgr::RunAllService(timeNow - timeOld, timeNow);
        sTimerMgr.Refresh(timeNow);
        GameApi::RefreshTimeNow();

        sMsgPool.Handle();

        Sleep(100);
    }

	system("pause");
	return 0;
}