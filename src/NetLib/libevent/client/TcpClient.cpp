#include "stdafx.h"
#include "TcpClient.h"
#include "config_net.h"
#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/bufferevent_struct.h>

TcpClient::TcpClient(const NetCfgClient& info)
    : _config(info)
    , _recvBuf(4096)
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
    bufferevent_write(_bev, &size, sizeof(size));
    bufferevent_write(_bev, pMsg, size);
}

void cb_conn_read(struct bufferevent* bev, void* arg);
void cb_conn_event(struct bufferevent* bev, short events, void* arg);

void TcpClient::_loop()
{
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(0x0201, &wsa_data);
#endif
    _base = event_base_new();
    if (!_base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return;
    }
    _Connect();

    event_base_dispatch(_base); //block

    bufferevent_free(_bev);
    event_base_free(_base);
}
void TcpClient::_Connect()
{
    printf("Connect to server...\n");

    struct sockaddr_in sin; memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(_config.svrPort);
    sin.sin_addr.s_addr = inet_addr(_config.svrIp);

    _bev = bufferevent_socket_new(_base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(_bev, cb_conn_read, NULL, cb_conn_event, this);
    bufferevent_enable(_bev, EV_READ);
    bufferevent_socket_connect(_bev, (struct sockaddr*)&sin, sizeof(sin));
}
void cb_conn_read(struct bufferevent* bev, void* arg)
{
    auto client = (TcpClient*)arg;
    auto& buf = client->_recvBuf;

    size_t n = bufferevent_read(bev, buf.beginWrite(), buf.writableBytes());
    buf.writerMove(n);
    const int c_off = sizeof(uint16);
    char* pPack = buf.beginRead();
    while (buf.readableBytes() >= c_off)
    {
        const uint kMsgSize = *((uint16*)pPack);	// 【网络包：头2字节为消息体大小】
        const uint kPackSize = kMsgSize + c_off;	// 【网络包长 = 消息体大小 + 头长度】
        char* pMsg = pPack + c_off;                 // 【后移2字节得：消息体指针】

        if (kPackSize > buf.readableBytes()) break; // 【包未收完：接收字节 < 包大小】

        client->_HandleServerMsg(pMsg, kMsgSize);

        // 消息处理完毕，接收字节/包指针更新(处理下一个包)
        buf.readerMove(n);
        pPack += kPackSize;
    }
}
void cb_conn_event(struct bufferevent* bev, short events, void* arg)
{
    auto client = (TcpClient*)arg;

    if (events & BEV_EVENT_CONNECTED) {
        client->_OnConnect();
        printf("Connect to server success\n");
        return;
    }

    if (events & BEV_EVENT_EOF) {
        printf("Connection closed\n");
    } else if (events & BEV_EVENT_ERROR) {
        printf("Got an error on the connection: %s\n", strerror(errno));
    }
    bufferevent_free(bev);

    // 断线重连
    auto timer = evtimer_new(client->_base, [](int fd, short events, void* arg){
        ((TcpClient*)arg)->_Connect();
    }, client);
    struct timeval delay = { 10, 0 };
    evtimer_add(timer, &delay);
}