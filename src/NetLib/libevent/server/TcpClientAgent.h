#pragma once
#include "tool/Mempool.h"
#include "Buffer/buffer.h"

class TcpServer;
class TcpClientAgent {
    Pool_Obj_Define(TcpClientAgent, 5000)

    TcpServer* const        m_pMgr;
    void*                   m_player = NULL;
    struct bufferevent*     m_bev = NULL;
    net::Buffer             m_recvBuf;

public:
    TcpClientAgent(TcpServer* p, struct bufferevent* bev);
    void CloseLink();
    void RecvMsg(const void* pMsg, uint16 size);
    void SendMsg(const void* pMsg, uint16 size);
};
