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
#include "Buffer/NetPack.h"
#include "Csv/CSVparser.hpp"

typedef std::function<void(NetPack&)> ParseRpcParam;
typedef std::function<void(const NetPack&)> SendMsgFunc;

template <typename Typ> // ����Typ�뺬�У�_rpc�б�SendMsg()
class RpcQueue {
    typedef std::pair<Typ*, NetPack*> RpcPair;

    std::map<int, ParseRpcParam> _response;  //rpcԶ�˵Ļظ�
    SafeQueue<RpcPair>          _queue; //Notice��Ϊ���⻺��ָ��Ұ������ѭ��HandleMsg֮�󣬴���ǳ��߼�
public:
    static RpcQueue& Instance(){ static RpcQueue T; return T; }
    RpcQueue() { LoadRpcCsv(); }

    void Insert(Typ* pObj, const void* pData, uint size)
    {
        _queue.push(std::make_pair(pObj, new NetPack(pData, size)));
    }
    void Update() //��ѭ����ÿ֡��һ��
    {
        RpcPair data;
        if (_queue.pop(data))
        {
            _Handle(data.first, *data.second);

            delete data.second;
        }
    }
    void _Handle(Typ* pObj, NetPack& buf)
    {
        uint16 opCode = buf.GetOpcode();

        auto it = Typ::_rpc.find(opCode);
        if (it != Typ::_rpc.end()) {
            NetPack& backBuffer = pObj->BackBuffer();
            backBuffer.ClearBody();
            backBuffer.SetOpCode(buf.GetOpcode());
            backBuffer.SetFromType(buf.GetFromType());

            (pObj->*(it->second))(buf);

            if (backBuffer.BodyBytes()) pObj->SendMsg(backBuffer);
        }
        else
        {
            auto it = _response.find(opCode);
            assert(it != _response.end());
            if (it != _response.end()) it->second(buf);
        }
    }
    void RegistResponse(int opCode, const ParseRpcParam& func)
    {
        assert(Typ::_rpc.find(opCode) == Typ::_rpc.end());

        _response.insert(make_pair(opCode, func));
    }
    int RpcNameToId(const char* name)
    {
        auto it = _rpc_table.find(name);
        if (it == _rpc_table.end()) {
            assert(0);
            return 0;
        }
        return it->second;
    }
    int _CallRpc(const char* name, const ParseRpcParam& func, const SendMsgFunc& doSend)
    {
        int opCodeId = RpcNameToId(name);
        assert(opCodeId > 0);
        static NetPack msg(0);
        msg.ClearBody();
        msg.SetOpCode(opCodeId);
        func(msg);
        doSend(msg);
        return opCodeId;
    }

private:
    std::map<std::string, int> _rpc_table;
    void LoadRpcCsv()
    {
        csv::Parser file = csv::Parser("../data/csv/rpc.csv");
        uint cnt = file.rowCount();
        for (uint i = 0; i < cnt; ++i) {
            csv::Row& row = file[i];
            _rpc_table[row["name"]] = atoi(row["id"].c_str());
        }
    }
};
#define sRpcClient RpcQueue<Player>::Instance()
#define sRpcCross RpcQueue<CrossAgent>::Instance()
