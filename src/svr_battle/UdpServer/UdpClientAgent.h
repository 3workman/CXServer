#pragma once
#include "RakNetTypes.h"
#include "tool\Mempool.h"

class NetPack;
class UdpClientAgent 
{
    Pool_Obj_Define(UdpClientAgent, 5000)

    // ���ӽ�������¼�Զ�udp��Ϣ
    RakNet::SystemAddress   m_addr;
    RakNet::RakNetGUID      m_guid;

    // ��¼svr����¼�û���Ϣ
    uint64                  m_userid;
    std::string             m_name;
public:
    void HandlePacket(NetPack& recvBuf);
    void SendMsg(const NetPack& msg);

    // message handler
};