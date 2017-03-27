#include "stdafx.h"
#include "..\NetLib\server\ServLinkMgr.h"
#include "..\NetLib\server\define.h"
#include "..\msg\MsgPool.h"
#include "..\rpc\RpcQueue.h"
#include "Player\Player.h"
#include "Service\ServiceMgr.h"
#include "Timer\TimerWheel.h"
#include "tool\GameApi.h"
#include "..\msg\LoginMsg.h"
#include "Buffer\NetPack.h"


void BindPlayerLink(void*& refPlayer, ServLink* p, void* pMsg, DWORD size)
{
    NetPack msg(pMsg, size);
    switch (msg.GetOpcode()){
    case C2S_Login: {
        refPlayer = new Player(p);
    } break;
    case C2S_ReConnect: {
        uint playerIdx = msg.ReadInt32();
        if (Player* player = Player::FindByIdx(playerIdx)) {
            player->SetServLink(p);
            refPlayer = player;
        }
    } break;
    default: assert(0); break;
    }
}
void HandleClientMsg(void* player, void* pMsg, DWORD size)
{
    //sMsgPool.Insert((Player*)player, pMsg, size);
    sRpcQueue.Insert((Player*)player, pMsg, size);
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

        //sMsgPool.Handle();
        sRpcQueue.Handle();

        Sleep(100);
    }

	system("pause");
	return 0;
}