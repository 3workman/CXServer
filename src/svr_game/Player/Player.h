#pragma once
#include "../rpc/RpcEnum.h"
#include "../rpc/RpcQueue.h"
#include "def/ConstDef.h"

#ifdef _USE_UDP // 在"项目属性->预处理器->预处理器定义"中设置
class UdpClientAgent;
typedef UdpClientAgent  NetLink;
#else
class ServLink;
typedef ServLink  NetLink;
#endif

#undef Rpc_Declare
#undef Rpc_Realize
#define Rpc_Declare(typ) void HandleRpc_##typ(NetPack&);
#define Rpc_Realize(typ) void Player::HandleRpc_##typ(NetPack& recvBuf)

class NetPack;
class PlayerRoomData;
class Player {
    Pool_Index_Define(Player, MAX_PLAYER_COUNT);
private:
    NetLink*        _clientNetLink = NULL;
public:
    uint64          m_pid = 0;
    bool            m_isLogin = false;
//////////////////////////////////////////////////////////////////////////
    PlayerRoomData* m_RoomData;
public:
    Player();
    ~Player();
    void SetNetLink(NetLink* p);
    void SendMsg(const NetPack& pack);
    NetPack& BackBuffer() { return sRpcClient.BackBuffer; }
    uint64 CallRpc(const char* name, const ParseRpcParam& sendFun);
    void CallRpc(const char* name, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun);
public:
    typedef void(Player::*_RpcFunc)(NetPack&);
    static std::map<int, _RpcFunc>      _rpc; //自己实现的rpc
    Rpc_For_Player;
public:

};
