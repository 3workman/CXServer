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
#include "tool\SafeQueue.h"

class Player;
class NetPack;
class RpcQueue {
    typedef void(Player::*RpcFunc)(NetPack&);

    std::map<int, RpcFunc>  _func;
    SafeQueue< std::pair<Player*, NetPack*> >  _queue; //Notice��Ϊ���⻺��ָ��Ұ������ѭ��HandleMsg֮�󣬴���ǳ��߼�
public:
    static RpcQueue& Instance(){ static RpcQueue T; return T; }
    RpcQueue();

    void Insert(Player* player, void* pData, DWORD size); //Notice���뿼���̰߳�ȫ
    void Handle(); //��ѭ����ÿ֡��һ��
};
#define sRpcQueue RpcQueue::Instance()
