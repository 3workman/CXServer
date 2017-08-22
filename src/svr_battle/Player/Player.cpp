#include "stdafx.h"
#include "Player.h"
#ifdef _USE_RAKNET
#include "raknet/server/UdpClientAgent.h"
#elif defined(_USE_IOCP)
#include "iocp/server/ServLink.h"
#elif defined(_USE_HANDY)
#include "handy/server/TcpServer.h"
#endif
#include "Buffer/NetPack.h"
#include "Room/PlayerRoomData.h"
#include "Lua/LuaCall.h"
#include "tool/compress.h"

static Byte g_compress_buf[1024] = { 0 };
static const uint G_Compress_Limit_Size = 128;
static const uint G_Compress_Flag = 0x80000000;

std::map<int, Player::_RpcFunc> Player::_rpc;
std::map<uint32, Player*> Player::G_PlayerList;

Player::Player(uint32 pid)
    : m_pid(pid)
    , m_Room(*new PlayerRoomData(*this))
{
    G_PlayerList[pid] = this;

    if (_rpc.empty())
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpc[sRpcClient.RpcNameToId(#typ)] = &Player::HandleRpc_##typ;
        Rpc_For_Player;
    }
}
Player::~Player()
{
    delete &m_Room;
    SetNetLink(NULL);
    G_PlayerList.erase(m_pid); m_pid = 0;
}
void Player::SetNetLink(NetLinkPtr p)
{
    if (_clientNetLink)
    {
        _clientNetLink->CloseLink();
    }
    _clientNetLink = p;
}
void Player::SendMsg(const NetPack& pack)
{
    const uint8 type = pack.Type();
    const void* buf = NULL; int bufLen = 0;

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

    assert(buf && bufLen && bufLen < 1024);
    switch (type) {
    case NetPack::TYPE_UDP:
        //_clientNetLink->SendUdpMsg(buf, bufLen);
        //break;
    case NetPack::TYPE_UNRELIABLE:
        //_clientNetLink->SendReliablyMsg(buf, bufLen);
        //break;
    default:
        _clientNetLink->SendMsg(buf, bufLen);
        break;
    }
}
uint64 Player::CallRpc(const char* name, const ParseRpcParam& sendFun)
{
    return sRpcClient._CallRpc(name, sendFun, std::bind(&Player::SendMsg, this, std::placeholders::_1));
}
void Player::CallRpc(const char* name, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun)
{
    uint64 reqKey = CallRpc(name, sendFun);
    sRpcClient.RegistResponse(reqKey, recvFun);
}
void Player::SendRpcAckImmediately()
{
    sRpcClient.SendBackBuffer(this);
}

Player* Player::FindByPid(uint32 pid)
{
    auto it = G_PlayerList.find(pid);
    if (it == G_PlayerList.end()) return NULL;
    return it->second;
}

//////////////////////////////////////////////////////////////////////////
// rpc
Rpc_Realize(rpc_battle_login)
{
    printf("rpc_login\n");

    if (m_Room.m_canJoinRoom) m_Room.NotifyClientJoinRoom();

    ack << m_index;

    G_Lua->Call("rpc_client_test", "i", m_index);
    G_Lua->Call("rpc_client_test2", "p", this);
}
Rpc_Realize(rpc_battle_logout)
{
    printf("rpc_logout\n");
    m_isLogin = false;
}
Rpc_Realize(rpc_battle_reconnect)
{
    printf("rpc_reconnect\n");

    //TODO:到这里已经重连成功了，通知client
}
Rpc_Realize(rpc_echo)
{
    std::string str = req.ReadString();
    const char* pstr = str.c_str();
    LOG_TRACK("Echo: %s\n", pstr);

    ack << str;
}
