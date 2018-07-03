#pragma once
#include <thread>
#include "Buffer/buffer.h"

struct NetCfgClient;
class TcpClient {
    typedef std::function<void()> OnConnectFunc;
    typedef std::function<void(const void* pMsg, int size)> HandleMsgFunc;
public:
    const NetCfgClient& _config;
    struct bufferevent* _bev = NULL;
    struct event_base*  _base = NULL;
    std::thread*        _thread = NULL;
    HandleMsgFunc       _HandleServerMsg;
    OnConnectFunc       _OnConnect;
    net::Buffer         _recvBuf;
    net::Buffer         _sendBuf;

public:
    TcpClient(const NetCfgClient& info);
    ~TcpClient();

    void SendMsg(const void* pMsg, uint16 size);
    void CreateLinkAndConnect(const OnConnectFunc& onConnect, const HandleMsgFunc& onMsg);
    void _loop();
    void _Connect();
};
