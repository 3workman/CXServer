#pragma once
#include "RakNetTypes.h"

struct NetCfgServer;
class UdpClientAgent;
class UdpServer {
public:
    RakNet::RakPeerInterface* m_rakPeer;
    std::map<RakNet::RakNetGUID, UdpClientAgent*> m_clientList;
public:
    typedef void(*HandleMsgFunc)(void* player, const void* pMsg, int size);
    typedef bool(*BindLinkFunc)(void*& refPlayer, UdpClientAgent* p, const void* pMsg, int size);
    typedef void(*ReportErrorFunc)(void* player, int InvalidEnum, int nErrorCode);

    const NetCfgServer& _config;
    BindLinkFunc        _BindLinkAndPlayer = NULL;
    HandleMsgFunc       _HandleClientMsg = NULL;
    ReportErrorFunc     _ReportErrorMsg = NULL;

public:
    UdpServer(const NetCfgServer& info);
    ~UdpServer();
    void Start(BindLinkFunc bindPlayer, HandleMsgFunc handleClientMsg, ReportErrorFunc reportErrorMsg);
    void Update();
    void CloseLink(const RakNet::SystemAddress& addr);

    UdpClientAgent* FindClientAgent(const RakNet::RakNetGUID& guid);
    UdpClientAgent* AddClientAgent(const RakNet::RakNetGUID& guid);
    void RemoveClientAgent(const RakNet::RakNetGUID& guid, std::function<void(UdpClientAgent*)> cb);
    void OnLinkClosed(const RakNet::RakNetGUID& guid);

private:
    void _HandlePacket(RakNet::Packet* packet);
};