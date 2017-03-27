#pragma once
#include "RakNetTypes.h"

class NetPack;
class UdpClientAgent;
class UdpServer {
    RakNet::RakPeerInterface* m_rakPeer;
    std::map<RakNet::RakNetGUID, UdpClientAgent*> m_clientList;
public:
    static UdpServer& Instance(){ static UdpServer T; return T; }
    bool Start();
    void Stop();
    void Update();
    bool SendMsg(const RakNet::SystemAddress& clientAddr, const NetPack& msg);

    UdpClientAgent* FindClientAgent(const RakNet::RakNetGUID& guid);
    UdpClientAgent* AddClientAgent(const RakNet::RakNetGUID& guid);
    void RemoveClientAgent(const RakNet::RakNetGUID& guid);
    void CloseLink(const RakNet::RakNetGUID& guid);
    void OnLinkClosed(const RakNet::RakNetGUID& guid);

private:
    void _HandlePacket(RakNet::Packet* packet);
};
#define sUdpServer UdpServer::Instance()