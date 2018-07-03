/***********************************************************************
* @ 消息池
* @ brief
    1、转接网络层buffer中的数据，缓存，以待主逻辑循环处理
    2、直接处理网络buffer的数据，那就是在IO线程做逻辑了，很可能出现性能风险

* @ Notice
    1、Insert()接口，供网络层调用，是多线程的，所以消息池必须线程安全
    2、避免野指针：队列中缓存了Player*，应在主逻辑Handle()后才做 delete player

    3、“同名rpc混乱”：client rpc server且有回包；若server那边也有个同名rpc client，那client就不好区分底层收到的包，是自己rpc的回复，还是对方主动rpc
    4、远程调用其它模块的rpc，应是本模块未声明实现的。避免同名rpc的混乱
    5、rpc底层的回包，设计成带类型的（主动发、被动回），能解决“同名rpc混乱”问题，但觉用处不大，就先免掉

* @ author zhoumf
* @ date 2016-12-12
************************************************************************/
#pragma once
#include "tool/SafeQueue.h"
#include "Buffer/NetPack.h"
#include "generate_rpc_enum.h"

typedef std::function<void(NetPack&)> ParseRpcParam;

template <typename Typ> // 类型Typ须含有：_rpcfunc列表，SendMsg()
class RpcQueue {
    typedef std::pair<Typ*, NetPack*> RpcPair;

    std::map<uint64, ParseRpcParam> m_response;  //rpc远端的回复
    SafeQueue<RpcPair>          m_queue; //Notice：为避免缓存指针野掉，主循环HandleMsg之后，处理登出逻辑
    NetPack m_SendBuffer;
    NetPack m_BackBuffer;
public:
    static RpcQueue& Instance(){ static RpcQueue T; return T; }

    flatbuffers::FlatBufferBuilder SendBuilder;
    flatbuffers::FlatBufferBuilder BackBuilder;

    void Insert(Typ* pObj, const void* pData, uint size)
    {
        assert(NetPack::GetOpCode(pData));
        m_queue.push(std::make_pair(pObj, new NetPack(pData, size)));
    }
    void Update() //主循环，每帧调一次
    {
        RpcPair data;
        while (m_queue.pop(data)) {
            _Handle(data.first, *data.second);
            delete data.second;
        }
    }
    void _Handle(Typ* pObj, NetPack& buf)
    {
        uint16 opCode = buf.OpCode(); if (opCode >= ARRAY_SIZE(Typ::_rpcfunc)) { assert(0); return; }
        if (auto func = Typ::_rpcfunc[opCode]) {
            m_BackBuffer.ResetHead(buf);
            (pObj->*func)(buf, m_BackBuffer);
            if(m_BackBuffer.OpCode()) SendBackBuffer(pObj);
        } else {
            auto it = m_response.find(buf.GetReqKey());
            assert(it != m_response.end());
            if (it != m_response.end()) {
                it->second(buf);
                m_response.erase(it);
            }
        }
    }
    void SendBackBuffer(Typ* pObj)
    {
        assert(m_BackBuffer.OpCode() > 0 && "send rpc relay repeatedly");
        m_BackBuffer.Absorb(BackBuilder);
        if (m_BackBuffer.BodySize()) pObj->SendMsg(m_BackBuffer);
        m_BackBuffer.Clear();
    }
    void RegistResponse(uint64 reqKey, const ParseRpcParam& func)
    {
        m_response[reqKey] = func; //后来的应该覆盖之前的
        //m_response.insert(make_pair(reqKey, func));
    }
    uint64 _CallRpc(Typ* pObj, RpcEnum rid, const ParseRpcParam& func)
    {
        // make sure body is cleared, to avoid nested buffer writes
        assert(m_SendBuffer.OpCode() == 0 && "opcode overwriten, probably nested rpc buffer write");
        
        static uint32 _auto_req_idx = 0;
        // Server and Client have the same Rpc
        assert(Typ::_rpcfunc[rid] == NULL);

        m_SendBuffer.OpCode(rid);
        m_SendBuffer.ReqIdx(++_auto_req_idx);
        auto ret = m_SendBuffer.GetReqKey();

        func(m_SendBuffer);
        m_SendBuffer.Absorb(SendBuilder);
        pObj->SendMsg(m_SendBuffer);
        m_SendBuffer.Clear();
        return ret;
    }
};