#include "stdafx.h"
#include "TcpServer.h"
#include "TcpClientAgent.h"
#include "config_net.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/bufferevent_struct.h>

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

static void cb_listener(evconnlistener*, evutil_socket_t, sockaddr*, int socklen, void*);
static void cb_conn_read(struct bufferevent *bev, void *user_data);
static void cb_conn_event(bufferevent*, short, void*);
static void cb_signal(evutil_socket_t, short, void*);

void TcpServer::_loop()
{
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(0x0201, &wsa_data);
#endif
    event_base* base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return;
    }

    struct sockaddr_in sin; memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(_config.wPort);

    // bind�������ƶ���IP�Ͷ˿ڣ�ͬʱ��ʼ��listen���¼�ѭ����callback��listener_cb 
    // ����listener���¼�ѭ��ע����event_base��base��
    evconnlistener* listener = evconnlistener_new_bind(base, cb_listener, this,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
        (struct sockaddr*)&sin,
        sizeof(sin));
    if (!listener) {
        fprintf(stderr, "Could not create a listener!\n");
        return;
    }

    // ��ʼ���źŴ���event 
    event* signal_event = evsignal_new(base, SIGINT, cb_signal, base);
    // �����callback����base�� 
    if (!signal_event || event_add(signal_event, NULL) < 0) {
        fprintf(stderr, "Could not create/add a signal event!\n");
        return;
    }

    // ������������һ��������eventѭ����ֻ���ڵ���event_base_loopexit�� 
    // �Ż����������������أ�������ִ�и������������������������˳� 
    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_free(signal_event);
    event_base_free(base);
}

static void cb_listener(struct evconnlistener *listener, evutil_socket_t fd,
    struct sockaddr *sa, int socklen, void *user_data)
{
    event_base *base = evconnlistener_get_base(listener);

    // �½�һ��bufferevent���趨BEV_OPT_CLOSE_ON_FREE�� 
    // ��֤bufferevent��free��ʱ��fdҲ�ᱻ�ر� 
    bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev) {
        fprintf(stderr, "Error constructing bufferevent!");
        event_base_loopbreak(base);
        return;
    }
    auto agent = new TcpClientAgent((TcpServer*)user_data, bev);

    // �趨дbuffer��event������event 
    bufferevent_setcb(bev, cb_conn_read, NULL, cb_conn_event, agent);
    bufferevent_enable(bev, EV_READ); // �����¼�
}
static void delete_agent(TcpClientAgent* agent)
{
    agent->CloseLink();
    delete agent;
}

static void cb_conn_read(struct bufferevent *bev, void *user_data)
{
    auto agent = (TcpClientAgent*)user_data;
    auto& buf = agent->m_recvBuf;

    size_t n = bufferevent_read(bev, buf.beginWrite(), buf.writableBytes());
    buf.writerMove(n);
    const int c_off = sizeof(uint16);
    char* pPack = buf.beginRead();
    while (buf.readableBytes() >= c_off)
    {
        const uint kMsgSize = *((uint16*)pPack);	// ���������ͷ2�ֽ�Ϊ��Ϣ���С��
        const uint kPackSize = kMsgSize + c_off;	// ��������� = ��Ϣ���С + ͷ���ȡ�
        char* pMsg = pPack + c_off;                 // ������2�ֽڵã���Ϣ��ָ�롿

        // 1�������Ϣ��С
        if (kMsgSize >= agent->m_pMgr->_config.nMaxPackage) //��Ϣ̫��
        {
            delete_agent(agent);
        }
        // 2���Ƿ�ӵ�������
        if (kPackSize > buf.readableBytes()) break; // ����δ���꣺�����ֽ� < ����С��

        // 3����Ϣ���롢���� decode, unpack and ungroup
        agent->RecvMsg(pMsg, kMsgSize);

        // 4����Ϣ������ϣ������ֽ�/��ָ�����(������һ����)
        buf.readerMove(n);
        pPack += kPackSize;
    }
}

// �������дevent֮���event��callback 
static void cb_conn_event(struct bufferevent *bev, short events, void *user_data)
{
    if (events & BEV_EVENT_EOF) {
        printf("Connection closed\n");
    } else if (events & BEV_EVENT_ERROR) {
        printf("Got an error on the connection: %s\n", strerror(errno));
    }
    delete_agent((TcpClientAgent*)user_data);
}

// �źŴ���event���յ�SIGINT (ctrl-c)�źź��ӳ�2s�˳�eventѭ�� 
static void cb_signal(evutil_socket_t sig, short events, void *user_data)
{
    struct event_base *base = (event_base*)user_data;
    struct timeval delay = { 2, 0 };

    printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

    event_base_loopexit(base, &delay);
}