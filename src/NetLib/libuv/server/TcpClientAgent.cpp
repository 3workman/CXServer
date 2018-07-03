#include "stdafx.h"
#include "TcpClientAgent.h"
#include "TcpServer.h"

static const size_t MAX_MSG_BUF_LEN = 20480;

TcpClientAgent::TcpClientAgent(TcpServer* svr, uv_stream_t* p)
    : m_pMgr(svr)
    , m_handle(p)
    , m_recvBuf(4096)
    , m_sendBuf(10240)
{
}

static void on_close(uv_handle_t* handle) {
    printf("Tcp on_close(%p)... \n", handle);
    auto agent = (TcpClientAgent*)handle->data;
    free(handle);
    delete agent;
}

void TcpClientAgent::CloseLink()
{
    printf("Tcp CloseLink(%p)... \n", m_handle);
    if (!m_handle) return;
    uv_close((uv_handle_t*)m_handle, on_close);
    m_handle = NULL;
    m_pMgr->_ReportErrorMsg(m_player, 0, 0);
    m_player = NULL;
    m_recvBuf.clear();
    m_sendBuf.clear();
}
void TcpClientAgent::RecvMsg(const void* pMsg, uint16 size)
{
    if (m_player == NULL)
    {
        if (!m_pMgr->_BindLinkAndPlayer(m_player, this, pMsg, size))
        {
            CloseLink();
        }
    }
    m_pMgr->_HandleClientMsg(m_player, pMsg, size);
}

WriteReq* AllocWriteParam(size_t len)
{
    auto req = (WriteReq*)malloc(sizeof(WriteReq));
    req->buf.len = MAX_MSG_BUF_LEN;
    req->buf.base = (char*)malloc(req->buf.len);
    return req;
}
void FreeWriteParam(WriteReq* req)
{
    free(req->buf.base);
    free(req);
}

static void after_write(uv_write_t* req, int status)
{
    auto agent = (TcpClientAgent*)req->data;
    {
        cLock lock(agent->_csLock);
        agent->m_pMgr->_write_list.push_back((WriteReq*)req);
    }

    /* Free the read/write buffer and the request */
    if (status == 0) return;
    fprintf(stderr,
        "uv_write error: %s - %s\n",
        uv_err_name(status),
        uv_strerror(status));
}
void TcpClientAgent::SendMsg(const void* pMsg, uint16 size)
{
    if (!m_handle) return;

    if (size > MAX_MSG_BUF_LEN) {
        assert(0);
        fprintf(stderr, "----> uv_write buf err\n");
        return;
    }

    cLock lock(_csLock);
    WriteReq* wr = nullptr;
    {
        if (m_pMgr->_write_list.empty()) {
            wr = AllocWriteParam(size);
        } else {
            wr = m_pMgr->_write_list.front();
            m_pMgr->_write_list.pop_front();
        }
    }

    wr->req.data = this;

    m_sendBuf.append(size);
    m_sendBuf.append(pMsg, size);
    wr->buf.len = m_sendBuf.readableBytes();

    memcpy(wr->buf.base, m_sendBuf.beginRead(), m_sendBuf.readableBytes());
    m_sendBuf.clear();

    int ret = uv_write(&wr->req, m_handle, &wr->buf, 1, after_write);
    if (ret) {
        fprintf(stderr, "uv_write failed %s:%d\n", __FILE__, __LINE__);
    }
}