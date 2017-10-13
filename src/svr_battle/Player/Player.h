#pragma once
#include "tool/Mempool.h"
#include "RpcEnum.h"
#include "RpcQueue.h"
#include "def/ConstDef.h"
#include "Room/Core/GameObject.hpp"

#define _USE_RAKNET 1

#ifdef _USE_RAKNET
class UdpClientAgent;   typedef UdpClientAgent*     NetLinkPtr;
#elif defined(_USE_IOCP)
class ServLink;         typedef ServLink*           NetLinkPtr;
#elif defined(_USE_HANDY)
namespace handy { struct TcpConn; }
typedef std::shared_ptr<handy::TcpConn> NetLinkPtr;
#elif defined(_USE_LIBEVENT)
class TcpClientAgent;   typedef TcpClientAgent*     NetLinkPtr;
#endif

#undef Rpc_Declare
#undef Rpc_Realize
#define Rpc_Declare(typ) void HandleRpc_##typ(NetPack&, NetPack&);
#define Rpc_Realize(typ) void Player::HandleRpc_##typ(NetPack& req, NetPack& ack)

enum RoleType : int
{
    // 不要改变顺序
    ROLE_HERO   = 0,
    ROLE_BOSS   = 1,
    ROLE_MINION = 2,
};

class NetPack;
class FlatBufferBuilder;
class Player {
    Pool_Index_Define(Player, MAX_PLAYER_COUNT);
private:
    static std::map<uint32, Player*> G_PlayerList;

    NetLinkPtr      _clientNetLink = NULL;
public:
    uint32          m_pid = 0;
    std::string     m_name;
    bool            m_isLogin = false;
    uint32          m_roomId = 0;
    uint8           m_teamId = 0;//敌对关系
    RoleType        m_roleType = ROLE_HERO;
    
    bool            m_canJoinRoom = false;
    shared<GameObject> gameObject;
    
public:
    Player(uint32 pid);
    ~Player();
    void    SetNetLink(NetLinkPtr p);
    void    SendMsg(const NetPack& pack);

    uint64  CallRpc(RpcEnum rid, const ParseRpcParam& sendFun);
    void    CallRpc(RpcEnum rid, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun);
    void    SendRpcReplyImmediately();

    static uint GetPlayerCnt() { return G_PlayerList.size(); }
    static Player* FindByPid(uint32 pid);

    //【Notice】单线程业务逻辑的架构，同一时刻仅有一名玩家CallRpc，所以Builder才能是静态的
    static flatbuffers::FlatBufferBuilder& SendBuild() { return sRpcClient.SendBuilder; }
    static flatbuffers::FlatBufferBuilder& BackBuild() { return sRpcClient.BackBuilder; }
public:
    typedef void(Player::*_RpcFunc)(NetPack&, NetPack&);
    static  _RpcFunc      _rpc[rpc_enum_cnt]; //自己实现的rpc
    Rpc_For_Player;
    
public:
    void NotifyClientJoinRoom();
};
