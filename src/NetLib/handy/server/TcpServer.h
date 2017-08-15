#pragma once

namespace handy { 
    class EventBase;
    class TcpConnPtr;
}
namespace std {
    class thread;
}
struct NetCfgServer;
class TcpServer {
public:
public:
    typedef void(*HandleMsgFunc)(void* player, const void* pMsg, int size);
    typedef bool(*BindLinkFunc)(void*& refPlayer, handy::TcpConnPtr p, const void* pMsg, int size);
    typedef void(*ReportErrorFunc)(void* player, int InvalidEnum, int nErrorCode, int nParam);

    const NetCfgServer& _config;
    BindLinkFunc        _BindLinkAndPlayer = NULL;
    HandleMsgFunc       _HandleClientMsg = NULL;
    ReportErrorFunc     _ReportErrorMsg = NULL;
    handy::EventBase*   _EventLoop = NULL;
    std::thread*        _thread = NULL;

public:
    TcpServer(const NetCfgServer& info);
    ~TcpServer();
    void Start(BindLinkFunc bindPlayer, HandleMsgFunc handleClientMsg, ReportErrorFunc reportErrorMsg);
};