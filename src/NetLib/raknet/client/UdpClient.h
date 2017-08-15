#pragma once
#include "RakNetTypes.h"

struct NetCfgClient;
class UdpClient {
    typedef std::function<void(void* pMsg, int size)> HandleMsgFunc;

    enum EStatus { State_Close, State_Connecting, State_Connected };
private:
    EStatus                     m_eState = State_Close;
    RakNet::RakPeerInterface*   m_rakPeer;
    RakNet::SystemAddress       m_serverAddr;

    HandleMsgFunc               m_HandleServerMsg;
    std::function<void()>       m_onConnect;
    const NetCfgClient&         m_config;

public:
    UdpClient(const NetCfgClient& info);
    bool Start(const HandleMsgFunc& func);
    void Stop();
    void Update();
    void CloseLink();
    bool SendMsg(const void* pMsg, int size);
    void SetOnConnect(const std::function<void()>& func){ m_onConnect = func; }

    bool IsConnect(){ return m_eState == State_Connected; }
    bool IsClose(){ return m_eState == State_Close; }

private:
    void _HandlePacket(RakNet::Packet* packet);
};