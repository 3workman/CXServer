#include "stdafx.h"
#include "UdpClient.h"
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"

#pragma comment(lib,"Ws2_32.lib")

bool UdpClient::Start(const HandleMsgFunc& func) {
    // Pointers to the interfaces of our server and client.
    // Note we can easily have both in the same program
    m_rakPeer = RakNet::RakPeerInterface::GetInstance();
    char* ip = "127.0.0.1";
    uint16 port = 8989;
    char* password = "ChillyRoom";
    int   passwordLength = strlen(password);
    // Connecting the client is very simple.  0 means we don't care about
    // a connectionValidationInteger, and false for low priority threads
    RakNet::SocketDescriptor socketDescriptor(0, 0);
    socketDescriptor.socketFamily = AF_INET;
    m_rakPeer->Startup(8, &socketDescriptor, 1);
    m_rakPeer->SetOccasionalPing(true);

    RakNet::ConnectionAttemptResult ret = m_rakPeer->Connect(ip, port, password, passwordLength);
    RakAssert(ret == RakNet::CONNECTION_ATTEMPT_STARTED);
    m_eState = State_Connecting;

    m_HandleServerMsg = func;
    return true;
}
void UdpClient::Stop() {
    m_rakPeer->Shutdown(1000);
    RakNet::RakPeerInterface::DestroyInstance(m_rakPeer);
    m_eState = State_Close;
}
void UdpClient::CloseLink()
{
    m_rakPeer->CloseConnection(m_serverAddr, true);
}
void UdpClient::Update() {
    for (RakNet::Packet* packet = m_rakPeer->Receive(); packet; m_rakPeer->DeallocatePacket(packet), packet = m_rakPeer->Receive()) {
        _HandlePacket(packet);
    }
}
bool UdpClient::SendMsg(const void* pMsg, int size)
{
    return m_rakPeer->Send((const char*)pMsg, size, LOW_PRIORITY, RELIABLE_ORDERED, 0, m_serverAddr, false) > 0;
}
void UdpClient::_HandlePacket(RakNet::Packet* packet) {
    RakNet::MessageID raknetMsgId = packet->data[0];
    switch (raknetMsgId) {
    case ID_CONNECTION_REQUEST_ACCEPTED: {
        // 连接成功
        m_serverAddr = packet->systemAddress;
        m_eState = State_Connected;
        m_onConnect();
    } break;
    case ID_CONNECTION_ATTEMPT_FAILED: {
        // 连接失败
        printf("connect failed ... \n");
        m_eState = State_Close;
    } break;
    case ID_NO_FREE_INCOMING_CONNECTIONS: {
        // 服务器连接满了
        printf("server if full ... \n");
        m_eState = State_Close;
    } break;
    case ID_DISCONNECTION_NOTIFICATION: {
       // 连接主动断开，自己调CloseConnection()一定收到本消息
       printf("server has disconnected ... \n");
       m_eState = State_Close;
    } break;
    case ID_CONNECTION_LOST: {
        // 对端丢失
        printf("server lost connection ... \n");
        m_eState = State_Close;
    } break;
    default: {
        if (raknetMsgId > ID_USER_PACKET_ENUM) {
            m_HandleServerMsg(packet->data, packet->length);
        } else {
            printf("...RaknetMsgId(%d)\n", raknetMsgId);
        }
    } break;
    }
}
