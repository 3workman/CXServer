#include "stdafx.h"
#include "UdpServer.h"
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "UdpClientAgent.h"
#include "config_net.h"

UdpServer::UdpServer(const NetCfgServer& info)
: _config(info)
{

}
bool UdpServer::Start(BindLinkFunc bindPlayer, HandleMsgFunc handleClientMsg, ReportErrorFunc reportErrorMsg) {
    _BindLinkAndPlayer = bindPlayer;
    _HandleClientMsg = handleClientMsg;
    _ReportErrorMsg = reportErrorMsg;

    const char* password = _config.kPassword;
    int passwordLength = strlen(password);

    m_rakPeer = RakNet::RakPeerInterface::GetInstance();
    m_rakPeer->SetTimeoutTime(10000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
    m_rakPeer->SetIncomingPassword(password, passwordLength);
    m_rakPeer->SetMaximumIncomingConnections(_config.dwMaxLink);
    m_rakPeer->SetOccasionalPing(true);
    m_rakPeer->SetUnreliableTimeout(1000);

    RakNet::SocketDescriptor socketDescriptors;
    socketDescriptors.port = _config.wPort;
    socketDescriptors.socketFamily = AF_INET;

    bool bOk = m_rakPeer->Startup(_config.dwMaxLink, &socketDescriptors, 1) == RakNet::RAKNET_STARTED;
    if (!bOk) {
        printf("Server failed to start.  Terminating.\n");
        return false;
    } else {
        printf("Udp server success.\n");
    }
    return true;
}
void UdpServer::Stop() {
    m_rakPeer->Shutdown(1000);
    RakNet::RakPeerInterface::DestroyInstance(m_rakPeer);
}

UdpClientAgent* UdpServer::FindClientAgent(const RakNet::RakNetGUID& guid) {
    auto search = m_clientList.find(guid);
    if (search != m_clientList.end()) {
        return search->second;
    }
    return nullptr;
}
UdpClientAgent* UdpServer::AddClientAgent(const RakNet::RakNetGUID& guid) {
    UdpClientAgent* clientAgent = new UdpClientAgent(this);
    m_clientList[guid] = clientAgent;
    return clientAgent;
}
void UdpServer::RemoveClientAgent(const RakNet::RakNetGUID& guid) {
    UdpClientAgent* clientAgent = FindClientAgent(guid);
    if (clientAgent) {
        m_clientList.erase(guid);
        delete clientAgent;
    }
}
void UdpServer::CloseLink(const RakNet::SystemAddress& addr)
{
    m_rakPeer->CloseConnection(addr, true);
}
void UdpServer::OnLinkClosed(const RakNet::RakNetGUID& guid)
{
    if (UdpClientAgent* clientAgent = FindClientAgent(guid)) {
        _ReportErrorMsg(clientAgent->m_player, 0, 0, 0);
        clientAgent->m_player = NULL;
        m_clientList.erase(guid);
        delete clientAgent;
    }
}
bool UdpServer::SendMsg(const RakNet::SystemAddress& clientAddr, const void* pMsg, int size)
{
    return m_rakPeer->Send((const char*)pMsg, size, LOW_PRIORITY, RELIABLE_ORDERED, 0, clientAddr, false) > 0;
}
void UdpServer::Update() {
    for (RakNet::Packet* packet = m_rakPeer->Receive(); packet; m_rakPeer->DeallocatePacket(packet), packet = m_rakPeer->Receive()) {
        _HandlePacket(packet);
    }
}
void UdpServer::_HandlePacket(RakNet::Packet* packet) {
    RakNet::MessageID raknetMsgId = packet->data[0];
    switch (raknetMsgId) {
    case ID_NEW_INCOMING_CONNECTION: {
        // 新连接建立
        RakNet::RakNetGUID guid = packet->guid;
        if (UdpClientAgent* clientAgent = AddClientAgent(guid)) {
            clientAgent->m_guid = guid;
            clientAgent->m_addr = packet->systemAddress;
        }
        printf("New Client Connnect(%d)  cnt(%d) ...\n", guid.ToUint32(guid), m_clientList.size());
    } break;
    case ID_DISCONNECTION_NOTIFICATION: {
        // 连接主动断开，自己调CloseConnection()一定收到本消息
        printf("A client has disconnected ... \n");
        OnLinkClosed(packet->guid);
    } break;
    case ID_CONNECTION_LOST: {
        // 对端丢失
        printf("A client lost connection ... \n");
        OnLinkClosed(packet->guid);
    } break;
    default: {
        if (raknetMsgId > ID_USER_PACKET_ENUM) {
            if (UdpClientAgent* clientAgent = FindClientAgent(packet->guid))
                clientAgent->RecvMsg(packet->data, packet->length);
            else
                printf("No such client agnet \n");
        } else {
            printf("...RaknetMsgId(%d)\n", raknetMsgId);
        }
    } break;
    }
}