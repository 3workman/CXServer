#include "stdafx.h"
#include "TcpClient.h"
#include "config_net.h"

TcpClient::TcpClient(const NetCfgClient& info)
    : _config(info)
{

}
void TcpClient::CreateLinkAndConnect(const OnConnectFunc& onConnect, const HandleMsgFunc& onMsg)
{
    _OnConnect = onConnect;
    _HandleServerMsg = onMsg;

    _thread = new std::thread([&]{ _loop(); });
}
TcpClient::~TcpClient() {
    delete _thread;
}
void TcpClient::SendMsg(const void* pMsg, uint16 size)
{
    handy::Slice msg((const char*)pMsg, size);
    _conn->sendMsg(msg);
}
void TcpClient::_loop()
{
    handy::EventBase base;
    handy::Signal::signal(SIGINT, [&]{ base.exit(); });

    _conn = handy::TcpConn::createConnection(&base, _config.svrIp, _config.svrPort, 3000);
    _conn->onMsg(new handy::LengthCodec, [&](const handy::TcpConnPtr& con, handy::Slice msg) {
        _HandleServerMsg(msg.begin(), msg.size());
    });
    _conn->setReconnectInterval(3000);
    _conn->onState([&](const handy::TcpConnPtr& con) {
        switch (con->getState()) {
        case handy::TcpConn::Connected:
            _OnConnect();
            break;
        case handy::TcpConn::Closed:
            break;
        default:
            break;
        }
    });
    base.loop();
}