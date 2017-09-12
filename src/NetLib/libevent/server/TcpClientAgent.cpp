#include "stdafx.h"
#include "TcpClientAgent.h"
#include "TcpServer.h"
#include "event2/bufferevent.h"

TcpClientAgent::TcpClientAgent(TcpServer* p, struct bufferevent* bev)
    : m_pMgr(p)
    , m_bev(bev)
    , m_recvBuf(4096)
{

}
void TcpClientAgent::CloseLink()
{
    bufferevent_free(m_bev);
    m_pMgr->_ReportErrorMsg(m_player, 0, 0);
    m_player = NULL;
    m_recvBuf.clear();
}
void TcpClientAgent::RecvMsg(const void* pMsg, uint16 size)
{
    if (m_player == NULL)
    {
        if (!m_pMgr->_BindLinkAndPlayer(m_player, this, pMsg, size))
        {
            CloseLink();
        }
    }
    m_pMgr->_HandleClientMsg(m_player, pMsg, size);
}
void TcpClientAgent::SendMsg(const void* pMsg, uint16 size)
{
    bufferevent_write(m_bev, &size, sizeof(size));
    bufferevent_write(m_bev, pMsg, size);
}