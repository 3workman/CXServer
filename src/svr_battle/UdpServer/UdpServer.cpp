#include "stdafx.h"
#include "UdpServer.h"
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "UdpClientAgent.h"
#include "Buffer\NetPack.h"

bool UdpServer::Start() {
    char* password = "fuck";
    int passwordLength = strlen(password);
    unsigned short port = 8989;
    unsigned short maxClients = 10;

    m_rakPeer = RakNet::RakPeerInterface::GetInstance();
    m_rakPeer->SetTimeoutTime(10000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
    m_rakPeer->SetIncomingPassword(password, passwordLength);
    m_rakPeer->SetMaximumIncomingConnections(maxClients);
    m_rakPeer->SetOccasionalPing(true);
    m_rakPeer->SetUnreliableTimeout(1000);

    RakNet::SocketDescriptor socketDescriptors[2];
    socketDescriptors[0].port = port;
    socketDescriptors[0].socketFamily = AF_INET; // Test out IPV4
    socketDescriptors[1].port = port;
    socketDescriptors[1].socketFamily = AF_INET6; // Test out IPV6

    bool bOk = m_rakPeer->Startup(maxClients, socketDescriptors, 2) == RakNet::RAKNET_STARTED;
    if (!bOk) {
        printf("Failed to start dual IPV4 and IPV6 ports. Trying IPV4 only.\n");
        bool bOk = m_rakPeer->Startup(maxClients, socketDescriptors, 1) == RakNet::RAKNET_STARTED;
        if (!bOk) {
            printf("Server failed to start.  Terminating.\n");
            return false;
        } else {
            printf("IPV4 only success.\n");
        }
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
    UdpClientAgent* clientAgent = new UdpClientAgent;
    if (clientAgent) {
        m_clientList.insert(std::make_pair(guid, clientAgent));
    }
    return clientAgent;
}
void UdpServer::RemoveClientAgent(const RakNet::RakNetGUID& guid) {
    UdpClientAgent* clientAgent = FindClientAgent(guid);
    if (clientAgent) {
        m_clientList.erase(guid);
        delete clientAgent;
    }
}
void UdpServer::CloseLink(const RakNet::RakNetGUID& guid)
{
    m_rakPeer->CloseConnection(guid, true);
}
void UdpServer::OnLinkClosed(const RakNet::RakNetGUID& guid)
{
    RemoveClientAgent(guid);
}
bool UdpServer::SendMsg(const RakNet::SystemAddress& clientAddr, const NetPack& msg) 
{
    return m_rakPeer->Send((const char*)msg.Buffer(), msg.Size(), LOW_PRIORITY, RELIABLE_ORDERED, 0, clientAddr, false) > 0;
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
        UdpClientAgent* clientAgent = FindClientAgent(guid);
        if (clientAgent == nullptr) {
            if (clientAgent = AddClientAgent(guid)) {
                clientAgent->m_guid = guid;
                clientAgent->m_addr = packet->systemAddress;
            }
        }
        printf("New Client Connnect(%d) ...\n", m_clientList.size());
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
            RakNet::RakNetGUID guid = packet->guid;
            printf("guid %s\n", guid.ToString());

            if (UdpClientAgent* clientAgent = FindClientAgent(guid)) {
                NetPack* ptr = new NetPack(packet->data, packet->length);
                clientAgent->HandlePacket(*ptr);
                delete ptr;
            }
            else
                printf("No such client agnet \n");
        }
        printf("...RaknetMsgId(%d)\n", raknetMsgId);
    } break;
    }
}