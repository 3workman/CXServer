#include "stdafx.h"
#include "UdpClientAgent.h"
#include "UdpServer.h"
#include "Buffer\NetPack.h"
#include "..\rpc\RpcEnum.h"

void UdpClientAgent::HandlePacket(NetPack& recvBuf) {

    switch (recvBuf.GetOpcode()) {
    case rpc_echo: {
        std::string str = recvBuf.ReadString();
        printf("Echo: %s\n", str.c_str());
        SendMsg(recvBuf);
        if (str == "-1") sUdpServer.CloseLink(m_guid); //≤‚ ‘«øÃﬂœ¬œﬂ
    } break;
    default:
        sUdpServer.CloseLink(m_guid);
        break;
    }
}

void UdpClientAgent::SendMsg(const NetPack& msg) 
{
    sUdpServer.SendMsg(m_addr, msg);
}