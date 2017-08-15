#include "stdafx.h"
#include "TcpServer.h"
#include "config_net.h"
#include "handy/handy.h"
#include <thread>

TcpServer::TcpServer(const NetCfgServer& info)
    : _config(info)
{

}
void TcpServer::Start(BindLinkFunc bindPlayer, HandleMsgFunc handleClientMsg, ReportErrorFunc reportErrorMsg) {
    _BindLinkAndPlayer = bindPlayer;
    _HandleClientMsg = handleClientMsg;
    _ReportErrorMsg = reportErrorMsg;

    handy::Signal::signal(SIGINT, [&] { _EventLoop.exit(); });

    handy::TcpServerPtr svr = handy::TcpServer::startServer(&_EventLoop, "", _config.wPort);
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
                break;
            }
        });
        conn->onMsg(new handy::LengthCodec, [&](const handy::TcpConnPtr& conn, handy::Slice msg) {
            if (msg.size() == 0) { //忽略空消息
                return;
            }
            conn->sendMsg(handy::util::format("#sended to %d users", 233));

            if (conn->context<void*>() == NULL)
            {
                if (!_BindLinkAndPlayer(conn->context<void*>(), conn, msg.begin(), msg.size()))
                {
                    _ReportErrorMsg(conn->context<void*>(), 0, 0, 0);
                    conn->close();
                }
            }
            _HandleClientMsg(conn->context<void*>(), msg.begin(), msg.size());
        });
        return conn;
    });
    _thread = new std::thread([&] { _EventLoop.loop(); });
}
TcpServer::~TcpServer() {
    delete _thread;
}