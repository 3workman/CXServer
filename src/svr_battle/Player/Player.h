#pragma once
#include "../rpc/RpcEnum.h"
#include "../rpc/RpcQueue.h"
#include "def/ConstDef.h"

#define _USE_RAKNET 1

#ifdef _USE_RAKNET
class UdpClientAgent;   typedef UdpClientAgent*     NetLinkPtr;
#elif defined(_USE_IOCP)
class ServLink;         typedef ServLink*           NetLinkPtr;
#elif defined(_USE_HANDY)
class TcpConnPtr;       typedef handy::TcpConnPtr   NetLinkPtr;
#endif

#undef Rpc_Declare
#undef Rpc_Realize
#define Rpc_Declare(typ) void HandleRpc_##typ(NetPack&, NetPack&);
#define Rpc_Realize(typ) void Player::HandleRpc_##typ(NetPack& req, NetPack& ack)

class NetPack;
class PlayerRoomData;
class FlatBufferBuilder;
class Player {
    Pool_Index_Define(Player, MAX_PLAYER_COUNT);
private:
    static std::map<uint32, Player*> G_PlayerList;

    NetLinkPtr      _clientNetLink = NULL;
public:
    uint32          m_pid = 0;
    string          m_name;
    bool            m_isLogin = false;
//////////////////////////////////////////////////////////////////////////
    PlayerRoomData& m_Room;
public:
    Player(uint32 pid);
    ~Player();
    void    SetNetLink(NetLinkPtr p);
    void    SendMsg(const NetPack& pack);

    uint64  CallRpc(const char* name, const ParseRpcParam& sendFun);
    void    CallRpc(const char* name, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun);
    void    SendRpcAckImmediately();

    static uint GetPlayerCnt() { return G_PlayerList.size(); }
    static Player* FindByPid(uint32 pid);

    //【Notice】单线程业务逻辑的架构，同一时刻仅有一名玩家CallRpc，所以Builder才能是静态的
    static flatbuffers::FlatBufferBuilder& SendBuild() { return sRpcClient.SendBuilder; }
    static flatbuffers::FlatBufferBuilder& BackBuild() { return sRpcClient.BackBuilder; }

public:
    typedef void(Player::*_RpcFunc)(NetPack&, NetPack&);
    static std::map<int, _RpcFunc>      _rpc; //自己实现的rpc
    Rpc_For_Player;

public:

};
