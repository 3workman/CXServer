#include "stdafx.h"
#include "UdpClientAgent.h"
#include "UdpServer.h"
#include "RakPeerInterface.h"

UdpClientAgent::UdpClientAgent(UdpServer& svr)
    : m_pMgr(svr)
    , m_rakPeer(svr.m_rakPeer)
{

}
void UdpClientAgent::CloseLink()
{
    printf("Udp CloseLink(%xd)... \n", m_guid.ToUint32(m_guid));
    m_pMgr.CloseLink(m_guid);
    m_player = NULL;
}
void UdpClientAgent::RecvMsg(const void* pMsg, int size)
{
    if (m_player == NULL)
    {
        if (!m_pMgr._BindLinkAndPlayer(m_player, this, pMsg, size))
        {
            CloseLink();
        }
    }
    m_pMgr._HandleClientMsg(m_player, pMsg, size);
}
void UdpClientAgent::SendMsg(const void* pMsg, int size)
{
    m_rakPeer->Send((const char*)pMsg, size, HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_addr, false);
}
void UdpClientAgent::SendUdpMsg(const void* pMsg, int size)
{
    m_rakPeer->Send((const char*)pMsg, size, HIGH_PRIORITY, UNRELIABLE, 1, m_addr, false);
}
void UdpClientAgent::SendReliablyMsg(const void* pMsg, int size)
{
    m_rakPeer->Send((const char*)pMsg, size, HIGH_PRIORITY, RELIABLE, 2, m_addr, false);
}
