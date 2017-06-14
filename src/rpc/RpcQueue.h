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

    std::map<uint64, ParseRpcParam> m_response;  //rpcԶ�˵Ļظ�
    SafeQueue<RpcPair>          m_queue; //Notice��Ϊ���⻺��ָ��Ұ������ѭ��HandleMsg֮�󣬴���ǳ��߼�
    NetPack m_SendBuffer;
    NetPack m_BackBuffer;
public:
    static RpcQueue& Instance(){ static RpcQueue T; return T; }
    RpcQueue() { LoadRpcCsv(); }

    void Insert(Typ* pObj, const void* pData, uint size)
    {
        m_queue.push(std::make_pair(pObj, new NetPack(pData, size)));
    }
    void Update() //��ѭ����ÿ֡��һ��
    {
        RpcPair data;
        if (m_queue.pop(data)) {
            _Handle(data.first, *data.second);
            delete data.second;
        }
    }
    void _Handle(Typ* pObj, NetPack& buf)
    {
        uint16 opCode = buf.OpCode();
        auto it = Typ::_rpc.find(opCode);
        if (it != Typ::_rpc.end()) {
            m_BackBuffer.ResetHead(buf);
            (pObj->*(it->second))(buf, m_BackBuffer);
            if (m_BackBuffer.BodySize()) pObj->SendMsg(m_BackBuffer);
#ifdef _DEBUG
            LOG_TRACK("Recv Msg: %s(%d) \n", DebugRpcIdToName(opCode), opCode);
#endif
        } else {
            auto it = m_response.find(buf.GetReqKey());
            assert(it != m_response.end());
            if (it != m_response.end()) {
                it->second(buf);
                m_response.erase(it);
            }
        }
    }
    void RegistResponse(uint64 reqKey, const ParseRpcParam& func)
    {
        m_response[reqKey] = func; //������Ӧ�ø���֮ǰ��
        //m_response.insert(make_pair(reqKey, func));
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
    const char* DebugRpcIdToName(int id)
    {
        for (auto& it : _rpc_table)
        {
            if (it.second == id) return it.first.c_str();
        }
        return "nil";
    }
    uint64 _CallRpc(const char* name, const ParseRpcParam& func, const SendMsgFunc& doSend)
    {
        static uint32 _auto_req_idx = 0;
        int opCodeId = RpcNameToId(name);
        // Server and Client have the same Rpc
        assert(Typ::_rpc.find(opCodeId) == Typ::_rpc.end());
        m_SendBuffer.ClearBody();
        m_SendBuffer.OpCode(opCodeId);
        m_SendBuffer.ReqIdx(++_auto_req_idx);
        func(m_SendBuffer);
        doSend(m_SendBuffer);
        return m_SendBuffer.GetReqKey();
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
