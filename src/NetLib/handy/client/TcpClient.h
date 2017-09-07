#pragma once

#include <thread>
#include "handy/handy.h"

struct NetCfgClient;
class TcpClient {
    typedef std::function<void()> OnConnectFunc;
    typedef std::function<void(const void* pMsg, int size)> HandleMsgFunc;
public:
    const NetCfgClient& _config;
    std::thread*        _thread = NULL;
    handy::TcpConnPtr   _conn = NULL;
    HandleMsgFunc       _HandleServerMsg;
    OnConnectFunc       _OnConnect;

public:
    TcpClient(const NetCfgClient& info);
    ~TcpClient();

    void SendMsg(const void* pMsg, uint16 size);
    void CreateLinkAndConnect(const OnConnectFunc& onConnect, const HandleMsgFunc& onMsg);
    void _loop();
};
