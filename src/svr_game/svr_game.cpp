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

std::map<int, Player*> g_playerLst;
void BroadCast(NetPack& msg, Player* except = NULL)
{
    for (auto& it : g_playerLst)
    {
        if (it.second != except)
        {
            it.second->SendPack(msg);
        }
    }
}

void BindPlayerLink(void*& refPlayer, ServLink* p, void* pMsg, DWORD size)
{
    NetPack msg(pMsg, size);
    switch (msg.GetOpcode()){
    case C2S_Login: {
        Player* ptr = new Player(p);
        refPlayer = ptr;
        {
            // 广播，有人加入游戏
            NetPack msgSend(1);
            msgSend.WriteInt32(ptr->m_index);
            BroadCast(msgSend);

            // 通知新人，既有玩家位置信息
            NetPack msgSend2(1);
            msgSend2.WriteUInt8(g_playerLst.size());
            for (auto& it : g_playerLst)
            {
                msgSend2 << it.second->m_positionX << it.second->m_positionY;
            }
            ptr->SendPack(msgSend2);
        }
        g_playerLst[ptr->m_index] = ptr;
    } break;
    case C2S_ReConnect: {
        uint playerIdx = msg.ReadInt32();
        if (Player* player = Player::FindByIdx(playerIdx)) {
            player->SetServLink(p);
            refPlayer = player;
            g_playerLst[player->m_index] = player;
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
    cout << "———————————— RunServerIOCP ————————————" << endl;
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

        NetPack msg(101);
        for (auto& it : g_playerLst) {
            msg.ClearBody();
            msg << it.second->m_positionX << it.second->m_positionY;
            BroadCast(msg, it.second);
        }

        Sleep(34);
    }

	system("pause");
	return 0;
}