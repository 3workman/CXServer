#include "stdafx.h"
#include "../NetLib/server/ServLinkMgr.h"
#include "../NetLib/server/define.h"
#include "../rpc/RpcQueue.h"
#include "Player/Player.h"
#include "Service/ServiceMgr.h"
#include "Timer/TimerWheel.h"
#include "tool/GameApi.h"
#include "Buffer/NetPack.h"
#include "Log/LogFile.h"

//��TODO:BUG��IOCP���������ص��������Ƕ��̵߳��õģ�Ӧ��ת����Ϣ�������̣߳��ɺ��ߴ���
bool BindPlayerLink(void*& refPlayer, NetLink* p, const void* pMsg, int size)
{
    NetPack msg(pMsg, size);
    switch (msg.OpCode()) {
    case 1: {
        //TODO:��¼�����Ȳ�ѯ<account, player>����ʡ�Զ����ݿ�
        Player* player = new Player();
        player->SetNetLink(p);
        player->m_isLogin = true;
        refPlayer = player;
    } return true;
    case 2: {
        uint idx; uint64 pid; //������client�������еġ��ڴ�������pid��
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
    sRpcClient.Insert((Player*)player, pMsg, size);
}
void ReportErrorMsg(void* pUser, int InvalidEnum, int nErrorCode, int nParam)
{
    if (Player* player = (Player*)pUser)
    {
        player->SetNetLink(NULL);

        //TODO:������Ը��ݴ����������ִ������磺�İ���ֱ���ߵ�
        if (player->m_isLogin) {
            player->m_isLogin = false;
            sTimerMgr.AddTimer([=]{ if (!player->m_isLogin) delete player; }, 30); //�ڼ������������
        } else
            delete player;
    }
}
void RunServerIOCP(ServLinkMgr& mgr)
{
    cout << "������������������������ RunServerIOCP ������������������������" << endl;
    ServLinkMgr::InitWinsock();
    mgr.CreateServer(BindPlayerLink, HandleClientMsg, ReportErrorMsg);
}
int main(int argc, char* argv[])
{
    LogFile log("log\\game", LogFile::TRACK, true);
    _LOG_MAIN_(log);

    ServerConfig config;
    ServLinkMgr mgr(config);
    RunServerIOCP(mgr);

    uint timeOld(0), timeNow = GetTickCount();
    while (true) {
        timeOld = timeNow;
        timeNow = GetTickCount();

        GameApi::RefreshTimeNow();
        ServiceMgr::RunAllService(timeNow - timeOld, timeNow);
        sTimerMgr.Refresh(timeNow - timeOld, timeNow);
        sRpcClient.Update();

        Sleep(33);
    }

	system("pause");
	return 0;
}