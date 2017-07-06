#pragma once
#include "RakNetTypes.h"

struct NetCfgServer;
class UdpClientAgent;
class UdpServer {
    RakNet::RakPeerInterface* m_rakPeer;
    std::map<RakNet::RakNetGUID, UdpClientAgent*> m_clientList;
public:
    typedef void(*HandleMsgFunc)(void* player, const void* pMsg, int size);
    typedef bool(*BindLinkFunc)(void*& refPlayer, UdpClientAgent* p, const void* pMsg, int size);
    typedef void(*ReportErrorFunc)(void* player, int InvalidEnum, int nErrorCode, int nParam);
    BindLinkFunc        _BindLinkAndPlayer = NULL;
    HandleMsgFunc       _HandleClientMsg = NULL;
    ReportErrorFunc     _ReportErrorMsg = NULL;
    const NetCfgServer& _config;

    UdpServer(const NetCfgServer& info);
    bool Start(BindLinkFunc bindPlayer, HandleMsgFunc handleClientMsg, ReportErrorFunc reportErrorMsg);
    void Stop();
    void Update();
    void CloseLink(const RakNet::SystemAddress& addr);
    bool SendMsg(const RakNet::SystemAddress& clientAddr, const void* pMsg, int size);

    UdpClientAgent* FindClientAgent(const RakNet::RakNetGUID& guid);
    UdpClientAgent* AddClientAgent(const RakNet::RakNetGUID& guid);
    void RemoveClientAgent(const RakNet::RakNetGUID& guid);
    void OnLinkClosed(const RakNet::RakNetGUID& guid);

private:
    void _HandlePacket(RakNet::Packet* packet);
};