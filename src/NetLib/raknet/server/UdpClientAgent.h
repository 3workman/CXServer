#pragma once
#include "RakNetTypes.h"
#include "tool/Mempool.h"

class UdpServer;
class UdpClientAgent {
    Pool_Obj_Define(UdpClientAgent, 5000)

    RakNet::SystemAddress   m_addr;
    RakNet::RakNetGUID      m_guid;
    UdpServer* const        m_pMgr;
    void*                   m_player = NULL; //收到玩家第一条消息(登录)，从角色内存池(有上限5000)中取个，与Link绑定
    RakNet::RakPeerInterface* const m_rakPeer;

public:
    UdpClientAgent(UdpServer* p);
    void CloseLink();
    void RecvMsg(const void* pMsg, int size);
    void SendMsg(const void* pMsg, int size);
};