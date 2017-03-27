#pragma once
#include "RakNetTypes.h"
#include "tool\Mempool.h"

class NetPack;
class UdpClientAgent 
{
    Pool_Obj_Define(UdpClientAgent, 5000)

    // 连接建立，记录对端udp信息
    RakNet::SystemAddress   m_addr;
    RakNet::RakNetGUID      m_guid;

    // 登录svr，记录用户信息
    uint64                  m_userid;
    std::string             m_name;
public:
    void HandlePacket(NetPack& recvBuf);
    void SendMsg(const NetPack& msg);

    // message handler
};