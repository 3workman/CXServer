#pragma once
#include "tool/Mempool.h"
#include "Buffer/buffer.h"
#include "uv.h"

class TcpServer;
class TcpClientAgent {
    Pool_Obj_Define(TcpClientAgent, 5000)

    TcpServer* const        m_pMgr;
    void*                   m_player = NULL;
    net::Buffer             m_recvBuf;
    net::Buffer             m_sendBuf;

    uv_stream_t*            m_handle;
    cMutex                  _csLock;

public:
    TcpClientAgent(TcpServer* svr, uv_stream_t* p);
    void CloseLink();
    void RecvMsg(const void* pMsg, uint16 size);
    void SendMsg(const void* pMsg, uint16 size);
};
