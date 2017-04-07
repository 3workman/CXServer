#include "stdafx.h"
#include "UdpClientAgent.h"
#include "UdpServer.h"

UdpClientAgent::UdpClientAgent(UdpServer* p)
    : m_pMgr(p)
{

}
void UdpClientAgent::CloseLink()
{
    m_pMgr->CloseLink(m_addr);
}
void UdpClientAgent::RecvMsg(const void* pMsg, int size)
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
void UdpClientAgent::SendMsg(const void* pMsg, int size)
{
    m_pMgr->SendMsg(m_addr, pMsg, size);
}