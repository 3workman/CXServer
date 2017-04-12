/***********************************************************************
* @ ��Ϣ��
* @ brief
    1��ת�������buffer�е����ݣ����棬�Դ����߼�ѭ������
    2��ֱ�Ӵ�������buffer�����ݣ��Ǿ�����IO�߳����߼��ˣ��ܿ��ܳ������ܷ���

* @ Notice
    1��Insert()�ӿڣ����������ã��Ƕ��̵߳ģ�������Ϣ�ر����̰߳�ȫ
    2������Ұָ�룺�����л�����Player*��Ӧ�����߼�Handle()����� delete player

    3����ͬ��rpc���ҡ���client rpc server���лذ�����server�Ǳ�Ҳ�и�ͬ��rpc client����client�Ͳ������ֵײ��յ��İ������Լ�rpc�Ļظ������ǶԷ�����rpc
    4��Զ�̵�������ģ���rpc��Ӧ�Ǳ�ģ��δ����ʵ�ֵġ�����ͬ��rpc�Ļ���
    5��rpc�ײ�Ļذ�����Ƴɴ����͵ģ��������������أ����ܽ����ͬ��rpc���ҡ����⣬�����ô����󣬾������

* @ author zhoumf
* @ date 2016-12-12
************************************************************************/
#pragma once
#include "tool/SafeQueue.h"
class Player;
class NetPack;

typedef std::function<void(NetPack&)> SendRpcParam;
typedef std::function<void(NetPack&)> RecvRpcParam;

class RpcQueue {
    typedef void(Player::*RpcFunc)(NetPack&);
    typedef std::pair<Player*, NetPack*> RpcPair;

    std::map<int, RpcFunc>      _rpc;       //�Լ�ʵ�ֵ�rpc
    std::map<int, RecvRpcParam>  _response;  //rpcԶ�˵Ļظ�
    SafeQueue<RpcPair>          _queue; //Notice��Ϊ���⻺��ָ��Ұ������ѭ��HandleMsg֮�󣬴���ǳ��߼�
public:
    static RpcQueue& Instance(){ static RpcQueue T; return T; }
    RpcQueue();

    void Insert(Player* player, const void* pData, uint size); //Notice���뿼���̰߳�ȫ
    void Update(); //��ѭ����ÿ֡��һ��
    void _Handle(Player* player, NetPack& buf);
    void RegistResponse(int opCode, const RecvRpcParam& func);
    static int RpcNameToId(const char* name);
};
#define sRpcQueue RpcQueue::Instance()
