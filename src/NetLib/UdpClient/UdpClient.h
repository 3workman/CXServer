#pragma once
#include "RakNetTypes.h"

typedef void(*HandleMsgFunc)(void* pMsg, int size);

class UdpClient {
    enum EStatus { State_Close, State_Connecting, State_Connected };
private:
    EStatus                     m_eState = State_Close;
    HandleMsgFunc               m_HandleServerMsg = NULL;
    RakNet::RakPeerInterface*   m_rakPeer;
    RakNet::SystemAddress       m_serverAddr;

public:
    bool Start(HandleMsgFunc func);
    void Stop();
    void Update();
    void CloseLink();
    bool SendMsg(const void* pMsg, int size);

    bool IsConnect(){ return m_eState == State_Connected; }
    bool IsClose(){ return m_eState == State_Close; }

    void OnConnect();
private:
    void _HandlePacket(RakNet::Packet* packet);
};