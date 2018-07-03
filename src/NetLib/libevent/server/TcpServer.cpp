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
#include <event2/thread.h>

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
    evthread_use_windows_threads(); //使libevent线程安全
#else
    //使libevent线程安全
    if(evthread_use_pthreads() == -1) {
        fprintf(stderr, "Libevent initialization error. Cannot use pthreads!\n");
        return;
    }
#endif

    event_base* base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return;
    }

    struct sockaddr_in sin; memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(_config.wPort);

    // bind在上面制定的IP和端口，同时初始化listen的事件循环和callback：listener_cb 
    // 并把listener的事件循环注册在event_base：base上
    evconnlistener* listener = evconnlistener_new_bind(base, cb_listener, this,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
        (struct sockaddr*)&sin,
        sizeof(sin));
    if (!listener) {
        fprintf(stderr, "Could not create a listener!\n");
        return;
    }

    // 初始化信号处理event 
    event* signal_event = evsignal_new(base, SIGINT, cb_signal, base);
    // 把这个callback放入base中 
    if (!signal_event || event_add(signal_event, NULL) < 0) {
        fprintf(stderr, "Could not create/add a signal event!\n");
        return;
    }

    // 程序将在下面这一行内启动event循环，只有在调用event_base_loopexit后 
    // 才会从下面这个函数返回，并向下执行各种清理函数，导致整个程序退出 
    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_free(signal_event);
    event_base_free(base);
}

static void cb_listener(struct evconnlistener *listener, evutil_socket_t fd,
    struct sockaddr *sa, int socklen, void *user_data)
{
    event_base *base = evconnlistener_get_base(listener);

    // 新建一个bufferevent，设定BEV_OPT_CLOSE_ON_FREE， 
    // 保证bufferevent被free的时候fd也会被关闭 
    bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev) {
        fprintf(stderr, "Error constructing bufferevent!");
        event_base_loopbreak(base);
        return;
    }
    auto agent = new TcpClientAgent((TcpServer*)user_data, bev);

    // 设定写buffer的event和其它event 
    bufferevent_setcb(bev, cb_conn_read, NULL, cb_conn_event, agent);
    bufferevent_enable(bev, EV_READ); // 启用事件
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
        const uint kMsgSize = *((uint16*)pPack);	// 【网络包：头2字节为消息体大小】
        const uint kPackSize = kMsgSize + c_off;	// 【网络包长 = 消息体大小 + 头长度】
        char* pMsg = pPack + c_off;                 // 【后移2字节得：消息体指针】

        // 1、检查消息大小
        if (kMsgSize >= agent->m_pMgr->_config.nMaxPackage) //消息太大
        {
            delete_agent(agent);
        }
        // 2、是否接到完整包
        if (kPackSize > buf.readableBytes()) break; // 【包未收完：接收字节 < 包大小】

        // 3、消息解码、处理 decode, unpack and ungroup
        agent->RecvMsg(pMsg, kMsgSize);

        // 4、消息处理完毕，接收字节/包指针更新(处理下一个包)
        buf.readerMove(kPackSize);
        pPack += kPackSize;
    }
}

// 处理读、写event之外的event的callback 
static void cb_conn_event(struct bufferevent *bev, short events, void *user_data)
{
    if (events & BEV_EVENT_EOF) {
        printf("Connection closed\n");
    } else if (events & BEV_EVENT_ERROR) {
        printf("Got an error on the connection: %s\n", strerror(errno));
    }
    delete_agent((TcpClientAgent*)user_data);
}

// 信号处理event，收到SIGINT (ctrl-c)信号后，延迟2s退出event循环 
static void cb_signal(evutil_socket_t sig, short events, void *user_data)
{
    struct event_base *base = (event_base*)user_data;
    struct timeval delay = { 2, 0 };

    printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

    event_base_loopexit(base, &delay);
}
