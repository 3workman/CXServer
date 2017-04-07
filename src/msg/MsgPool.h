/***********************************************************************
* @ 消息池
* @ brief
    1、转接网络层buffer中的数据，缓存，以待主逻辑循环处理
    2、直接处理网络buffer的数据，那就是在IO线程做逻辑了，很可能出现性能风险

* @ Notice
    1、Insert()接口，供网络层调用，是多线程的，所以消息池必须线程安全
    2、避免野指针：队列中缓存了Player*，应在主逻辑Handle()后才做 delete player

* @ author zhoumf
* @ date 2016-12-12
************************************************************************/
#pragma once
#include "tool\Mempool.h"

class Player;
struct stMsg;
class MsgPool {
    typedef void(Player::*MsgFunc)(stMsg&);

    CPoolPage               _pool;
    std::map<int, MsgFunc>  _func;
    SafeQueue< std::pair<Player*, stMsg*> >  _queue; //Notice：为避免缓存指针野掉，主循环HandleMsg之后，处理登出逻辑
public:
    static MsgPool& Instance(){ static MsgPool T; return T; }
    MsgPool();

    void Insert(Player* player, void* pData, DWORD size); //Notice：须考虑线程安全
    void Handle(); //主循环，每帧调一次
};
#define sMsgPool MsgPool::Instance()


/*
// Player.h
struct stMsg;
#undef Msg_Declare
#undef Msg_Realize
#define Msg_Declare(typ, n) void HandleMsg_##typ(stMsg&);
#define Msg_Realize(typ) void Player::HandleMsg_##typ(stMsg& req)

//.cpp
Msg_Realize(C2S_Echo)
{
    TestMsg& msg = (TestMsg&)req;
    char* str = msg.data;
    SendMsg(msg, msg.size());
    printf("Echo: %s\n", str);
}
*/