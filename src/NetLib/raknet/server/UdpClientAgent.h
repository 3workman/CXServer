#pragma once
#include "RakNetTypes.h"
#include "tool/Mempool.h"

class UdpServer;
class UdpClientAgent {
    Pool_Obj_Define(UdpClientAgent, 5000)

    RakNet::SystemAddress   m_addr;
    RakNet::RakNetGUID      m_guid;
    UdpServer* const        m_pMgr;
    void*                   m_player = NULL; //�յ���ҵ�һ����Ϣ(��¼)���ӽ�ɫ�ڴ��(������5000)��ȡ������Link��
    RakNet::RakPeerInterface* const m_rakPeer;

public:
    UdpClientAgent(UdpServer* p);
    void CloseLink();
    void RecvMsg(const void* pMsg, int size);
    void SendMsg(const void* pMsg, int size);
};