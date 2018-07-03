#include "stdafx.h"
#include "Player.h"
#ifdef _USE_RAKNET
#include "PacketPriority.h"
#include "raknet/server/UdpClientAgent.h"
#elif defined(_USE_IOCP)
#include "iocp/server/ServLink.h"
#elif defined(_USE_HANDY)
#include "handy/handy.h"
#undef info
#elif defined(_USE_LIBEVENT)
#include "libevent/server/TcpClientAgent.h"
#endif
#include "Buffer/NetPack.h"
//#include "Lua/LuaCall.h"
#include "tool/compress.h"
#include "Room/Room.h"

static Byte g_compress_buf[1024] = { 0 };
static const uint G_Compress_Limit_Size = 128;
static const uint G_Compress_Flag = 0x80000000;

Player::_RpcFunc Player::_rpcfunc[RpcEnumCnt] = {0};
RpcQueue<Player>& Player::_rpc = RpcQueue<Player>::Instance();
std::map<uint32, Player*> Player::G_PlayerList;

Player::Player(uint32 pid)
    : m_pid(pid)
    , m_room(*this)
    , m_dbData(*this)
{
    if (!_rpcfunc[rpc_battle_login])
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpcfunc[typ] = &Player::HandleRpc_##typ;
        Rpc_For_Player;
    }
    G_PlayerList[pid] = this;
}

Player::~Player()
{
    if (CRoom* p = CRoom::FindByUniqueId(m_roomId))
    {
        p->ExitRoom(*this);
    }

    G_PlayerList.erase(m_pid); m_pid = 0;
}

Player* Player::FindByPid(uint32 pid)
{
    auto it = G_PlayerList.find(pid);
    if (it == G_PlayerList.end()) return NULL;
    return it->second;
}

void Player::CloseNetLink()
{
    if (_netLink) _netLink->CloseLink();
}
void Player::ResetNetLink(NetLinkPtr p)
{
    if (_netLink) {
        _netLink->m_player = NULL;
        _netLink->CloseLink();
    }
    _netLink = p;
}

void Player::SendMsg(const NetPack& pack)
{
    if (!_netLink) return;

    const uint8 type = pack.Type();
    const void* buf = NULL;
    size_t bufLen = 0;

    if (pack.size() < G_Compress_Limit_Size) {
        buf = pack.contents();
        bufLen = pack.size();
    } else {
        //前四个字节写压缩标记
        //蛋疼啊，udp第一个字节得预留给RakNet
        uLong size = sizeof(g_compress_buf) - 5;
        *g_compress_buf = *pack.contents();
        *(uint*)(g_compress_buf + 1) = G_Compress_Flag;
        gzcompress((Bytef*)pack.contents(), pack.size(), g_compress_buf + 5, &size);
        buf = g_compress_buf;
        bufLen = size + 5;
    }
    assert(buf && bufLen && bufLen < sizeof(g_compress_buf));

    switch (type) {
    case NetPack::TYPE_UDP:
        //_netLink->SendUdpMsg(buf, bufLen);
        //break;
    case NetPack::TYPE_UNRELIABLE:
        //_netLink->SendReliablyMsg(buf, bufLen);
        //break;
    default:
        _netLink->SendMsg(buf, bufLen);
        break;
    }
}

uint64 Player::CallRpc(RpcEnum rid, const ParseRpcParam& sendFun)
{
    return _rpc._CallRpc(this, rid, sendFun);
}
void Player::CallRpc(RpcEnum rid, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun)
{
    auto reqKey = CallRpc(rid, sendFun);
    _rpc.RegistResponse(reqKey, recvFun);
}
void Player::SendRpcReplyImmediately()
{
    _rpc.SendBackBuffer(this);
}

//////////////////////////////////////////////////////////////////////////
// rpc
Rpc_Realize(rpc_battle_login)
{
    printf("rpc_login: %s\n", m_name.c_str());

    if (m_room.m_canJoinRoom)
    {
        m_room.NotifyClientJoinRoom();
    }

    ack << m_index;

    //G_Lua->Call("rpc_client_test", "i", m_index);
//    G_Lua->Call("rpc_client_test2", "p", this);
}

Rpc_Realize(rpc_battle_logout)
{
    printf("rpc_logout\n");
    m_isLogin = false;
}

Rpc_Realize(rpc_battle_reconnect)
{
    printf("rpc_reconnect\n");
    //TODO:到这里已经重连成功了，重发战场信息
}