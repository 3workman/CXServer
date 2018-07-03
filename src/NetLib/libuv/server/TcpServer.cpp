#include "stdafx.h"
#include "TcpServer.h"
#include "TcpClientAgent.h"
#include "config_net.h"
#include "uv.h"
#include <stdio.h>

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "userenv.lib")

static uv_loop_t* loop;
static uv_tcp_t tcpServer;
static uv_handle_t* server;
TcpServer* m_server;

TcpServer::TcpServer(const NetCfgServer& info)
    : _config(info)
{

}
void TcpServer::Start(BindLinkFunc bindPlayer, HandleMsgFunc handleClientMsg, ReportErrorFunc reportErrorMsg) {
    _BindLinkAndPlayer = bindPlayer;
    _HandleClientMsg = handleClientMsg;
    _ReportErrorMsg = reportErrorMsg;

    _thread = new std::thread([&] { _loop(); });
}
TcpServer::~TcpServer() {
    delete _thread;
}

static void after_write(uv_write_t* req, int status);
static void after_read(uv_stream_t*, ssize_t nread, const uv_buf_t* buf);
static void on_connection(uv_stream_t*, int status);
static void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

void TcpServer::_loop() {
    loop = uv_default_loop();
    m_server = this;

    server = (uv_handle_t*)&tcpServer;

    int r = uv_tcp_init(loop, &tcpServer);
    if (r) {
        /* TODO: Error codes */
        fprintf(stderr, "Socket creation error\n");
        return;
    }

    uv_tcp_nodelay(&tcpServer, 1);

    sockaddr_in addr;
    uv_ip4_addr(_config.ip, _config.wPort, &addr);
    r = uv_tcp_bind(&tcpServer, (sockaddr*)&addr, 0);
    if (r) {
        /* TODO: Error codes */
        fprintf(stderr, "Bind error\n");
        return;
    }

    r = uv_listen((uv_stream_t*)&tcpServer, 5, on_connection);
    if (r) {
        /* TODO: Error codes */
        fprintf(stderr, "Listen error %s\n", uv_err_name(r));
        return;
    }
    uv_run(loop, UV_RUN_DEFAULT);
}
static void on_connection(uv_stream_t* server, int status) {
    if (status != 0) {
        fprintf(stderr, "Connect error %s\n", uv_err_name(status));
    }
    assert(status == 0);

    uv_stream_t* stream = (uv_stream_t*)malloc(sizeof(uv_tcp_t)); assert(stream != NULL);
    auto agent = new TcpClientAgent(m_server, stream);
    stream->data = agent;

    int r = uv_tcp_init(loop, (uv_tcp_t*)stream); assert(r == 0);

    if (uv_accept(server, stream) == 0) {
        r = uv_read_start(stream, alloc_buffer, after_read); assert(r == 0);
    } else {
        agent->CloseLink();
    }
}

static void after_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
    auto agent = (TcpClientAgent*)handle->data;

    if (nread < 0) {
        agent->CloseLink();
        return;
    } else if (nread == 0) {
        /* Everything OK, but nothing read. */
        return;
    }

    auto& recvBuf = agent->m_recvBuf;
    recvBuf.writerMove(nread);

    const int c_off = sizeof(uint16);
    char* pPack = buf->base;
    while (recvBuf.readableBytes() >= c_off)
    {
        const uint kMsgSize = *((uint16*)pPack);	// 【网络包：头2字节为消息体大小】
        const uint kPackSize = kMsgSize + c_off;	// 【网络包长 = 消息体大小 + 头长度】
        char* pMsg = pPack + c_off;                 // 【后移2字节得：消息体指针】

                                                    // 1、检查消息大小
        if (kMsgSize >= agent->m_pMgr->_config.nMaxPackage) //消息太大
        {
            agent->CloseLink();
        }
        // 2、是否接到完整包
        if (kPackSize > recvBuf.readableBytes()) break; // 【包未收完：接收字节 < 包大小】

                                                    // 3、消息解码、处理 decode, unpack and ungroup
        agent->RecvMsg(pMsg, kMsgSize);

        // 4、消息处理完毕，接收字节/包指针更新(处理下一个包)
        recvBuf.readerMove(kPackSize);
        pPack += kPackSize;
    }
}
static void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    auto agent = (TcpClientAgent*)handle->data;
    auto& recvBuf = agent->m_recvBuf;
    recvBuf.ensureWritableBytes(1);
    buf->base = recvBuf.beginWrite();
    buf->len = recvBuf.writableBytes();
}