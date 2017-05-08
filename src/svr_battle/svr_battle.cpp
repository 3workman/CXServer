#include "stdafx.h"
#include "../NetLib/UdpServer/UdpServer.h"
#include "RakSleep.h"
#include "Service/ServiceMgr.h"
#include "Timer/TimerWheel.h"
#include "tool/GameApi.h"
#include "Buffer/NetPack.h"
#include "../svr_game/Player/Player.h"
#include "Log/LogFile.h"
#include "Cross/CrossAgent.h"

bool BindPlayerLink(void*& refPlayer, UdpClientAgent* p, const void* pMsg, int size)
{
    NetPack msg(pMsg, size);
    switch (msg.OpCode()){
    case 1: {
        //TODO:登录，可先查询<account, player>，可省略读数据库
        Player* player = new Player();
        player->SetNetLink(p);
        player->m_isLogin = true;
        refPlayer = player;
    } return true;
    case 2: {
        uint idx; uint64 pid;
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
    NetPack msg(pMsg, size);
    sRpcClient._Handle((Player*)player, msg);
}
void ReportErrorMsg(void* pUser, int InvalidEnum, int nErrorCode, int nParam)
{
    if (Player* player = (Player*)pUser)
    {
        player->SetNetLink(NULL);

        //TODO:这里可以根据错误类型区分处理，比如：改包的直接踢掉
        if (player->m_isLogin) {
            player->m_isLogin = false;
            sTimerMgr.AddTimer([=]{
                if (!player->m_isLogin)
                    delete player;
            }, 10); //期间允许断线重连
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
    sCrossAgent.CallRpc("rpc_regist", [](NetPack& buf){
        buf << "battle" << (uint32)1;
    });

    UdpServer upd;
    upd.Start(BindPlayerLink, HandleClientMsg, ReportErrorMsg);

    uint timeOld(0), timeNow = GetTickCount();
    while (true) {
        timeOld = timeNow;
        timeNow = GetTickCount();

        ServiceMgr::RunAllService(timeNow - timeOld, timeNow);
        sTimerMgr.Refresh(timeNow - timeOld, timeNow);
        GameApi::RefreshTimeNow();

        sRpcCross.Update();
        upd.Update();
        RakSleep(30);
    }
    upd.Stop();
	return 0;
}