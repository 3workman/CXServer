#pragma once
#include "tool\Mempool.h"
#include "..\rpc\RpcEnum.h"
#include "..\rpc\RpcQueue.h"
#include "def\ConstDef.h"

#ifdef _USE_UDP // 在"项目属性->预处理器->预处理器定义"中设置
#define _NET_LINK_CLASS UdpClientAgent
#else
#define _NET_LINK_CLASS ServLink
#endif

#undef Rpc_Declare
#undef Rpc_Realize
#define Rpc_Declare(typ) void HandleRpc_##typ(NetPack&);
#define Rpc_Realize(typ) void Player::HandleRpc_##typ(NetPack& recvBuf)

class NetPack;
class _NET_LINK_CLASS;
class PlayerRoomData;


class Player {
    Pool_Index_Define(Player, MAX_PLAYER_COUNT);
private:
    _NET_LINK_CLASS*    _clientNetLink = NULL;
    static NetPack&     _backBuffer;
public:
    uint64          m_pid = 0;
    bool            m_isLogin = false;
//////////////////////////////////////////////////////////////////////////
    PlayerRoomData* m_RoomData;
public:
    Player();
    void SetNetLink(_NET_LINK_CLASS* p);
    void SendMsg(const NetPack& pack);
    NetPack& BackBuffer() { return _backBuffer; }
    int  CallRpc(const char* name, const SendRpcParam& func);
    void CallRpc(const char* name, const SendRpcParam& func, const RecvRpcParam& callback);
public:
    Rpc_Enum;
public:

};
