/***********************************************************************
* @ ��Ϣ��
* @ brief
    1��ת�������buffer�е����ݣ����棬�Դ����߼�ѭ������
    2��ֱ�Ӵ�������buffer�����ݣ��Ǿ�����IO�߳����߼��ˣ��ܿ��ܳ������ܷ���

* @ Notice
    1��Insert()�ӿڣ����������ã��Ƕ��̵߳ģ�������Ϣ�ر����̰߳�ȫ
    2������Ұָ�룺�����л�����Player*��Ӧ�����߼�Handle()����� delete player

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
    SafeQueue< std::pair<Player*, stMsg*> >  _queue; //Notice��Ϊ���⻺��ָ��Ұ������ѭ��HandleMsg֮�󣬴���ǳ��߼�
public:
    static MsgPool& Instance(){ static MsgPool T; return T; }
    MsgPool();

    void Insert(Player* player, void* pData, DWORD size); //Notice���뿼���̰߳�ȫ
    void Handle(); //��ѭ����ÿ֡��һ��
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