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
#elif defined(_USE_LIBUV)
#include "libuv/server/TcpServer.h"
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

bool BindPlayerLink(void*& refPlayer, NetLinkPtr p, const void* pMsg, int size) //io线程调用
{
    NetPack msg(pMsg, size);
    switch (msg.OpCode()) {
    case rpc_battle_login:
    case rpc_battle_reconnect: {
        uint32 pid = msg.ReadUInt32();
        Player* player = Player::FindByPid(pid);
        if (player == NULL) player = new Player(pid); //TODO:zhoumf:外网找不到应主动断开
        player->ResetNetLink(p);
        player->m_isLogin = true;
        refPlayer = player;
    } return true;
    default: assert(0); return false;
    }
}

void HandleClientMsg(void* user, const void* pMsg, int size) //io线程调用
{
    //若玩家不在房间，除固定几条消息外，其它不予处理
    Player* player = (Player*)user;
    if (player->m_roomId == 0 || player->GetMyObj() == nullptr) {
        switch (NetPack::GetOpCode(pMsg)) {
        case rpc_battle_login:
        case rpc_battle_logout:
        case rpc_battle_reconnect:
        case rpc_battle_join_room:
        case rpc_battle_direct_join_room:
            break;
        default:return;
        }
    }
    //printf("recv msg:%d\n", NetPack::GetOpCode(pMsg));
    Player::_rpc.Insert(player, pMsg, size);
}

void ReportErrorMsg(void* pUser, int errEnum, int errCode) //io线程调用
{
    if (Player* player = (Player*)pUser) {
        NetPack msg(32); msg.OpCode(rpc_net_error);
        msg.WriteInt32(errEnum);
        msg.WriteInt32(errCode);
        Player::_rpc.Insert(player, msg.contents(), (uint)msg.size());
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
    //testing::InitGoogleTest(&argc, argv);
    //if (RUN_ALL_TESTS() == 0)
    //    printf("All tests pased\n");
    //else
    //    printf("some tests failed\n");

    NetCfgServer& cfg = NetCfgServer::Instance(); //网络，初始化
#ifdef _USE_RAKNET
    UdpServer svr(cfg);
#elif defined(_USE_IOCP)
    ServLinkMgr svr(cfg);
#elif defined(_USE_HANDY) || defined(_USE_LIBEVENT) || defined(_USE_LIBUV)
    TcpServer svr(cfg);
#endif
    svr.Start(BindPlayerLink, HandleClientMsg, ReportErrorMsg);
    printf("server success.\n");

    //sZookeeper.RunClient(); //注册到Zookeeper

    uint64 timeOld(0), timeNow = GameApi::TimeMS();
    while (true) {
        timeOld = timeNow; timeNow = GameApi::TimeMS();
        uint time_elapse = uint(timeNow - timeOld);
        
        GameApi::RefreshTimeSecond();
        ServiceMgr::RunAllService(time_elapse, timeNow);
        sTimerMgr.Refresh(time_elapse, timeNow);

#ifdef _USE_RAKNET
        svr.Update(); //网络收包
#endif
        Player::_rpc.Update();
        RpcClient::_rpc.Update();

        uint tickDiff = uint(GameApi::TimeMS() - timeNow);
        if(tickDiff < 20) RakSleep(20 - tickDiff); // TODO: bug

        Input::CheckKeyboardInput();
    }
    return 0;
}
