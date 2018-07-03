#include "stdafx.h"
#include "TcpServer.h"
#include "config_net.h"

TcpServer::TcpServer(const NetCfgServer& info)
    : _config(info)
{

}
void TcpServer::Start(BindLinkFunc bindPlayer, HandleMsgFunc handleClientMsg, ReportErrorFunc reportErrorMsg) {
    _BindLinkAndPlayer = bindPlayer;
    _HandleClientMsg = handleClientMsg;
    _ReportErrorMsg = reportErrorMsg;

    _thread = new std::thread([&]{ _loop(); });
}
TcpServer::~TcpServer() {
    delete _thread;
}
void TcpServer::_loop()
{
    handy::EventBase base;
    handy::Signal::signal(SIGINT, [&]{ base.exit(); });

    handy::TcpServerPtr svr = handy::TcpServer::startServer(&base, _config.ip, _config.wPort);
    if (svr == NULL) {
        printf("Server failed to start.  Terminating.\n");
        return;
    }
    svr->onConnCreate([&] {
        handy::TcpConnPtr conn(new handy::TcpConn);
        conn->onState([&](const handy::TcpConnPtr& con) {
            switch (con->getState()) {
            case handy::TcpConn::Connected:
                break;
            case handy::TcpConn::Closed:
                _ReportErrorMsg(conn->context<void*>(), 0, 0);
                conn->context<void*>() = NULL;
                break;
            default:
                break;
            }
        });
        conn->onMsg(new handy::LengthCodec, [&](const handy::TcpConnPtr& conn, handy::Slice msg) {
            if (conn->context<void*>() == NULL)
            {
                if (!_BindLinkAndPlayer(conn->context<void*>(), conn, msg.begin(), msg.size()))
                {
                    _ReportErrorMsg(conn->context<void*>(), 0, 0);
                    conn->close();
                }
            }
            _HandleClientMsg(conn->context<void*>(), msg.begin(), msg.size());
        });
        return conn;
    });
    base.loop();
}