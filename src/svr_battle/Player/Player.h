#pragma once
#include "tool/Mempool.h"
#include "RpcEnum.h"
#include "RpcQueue.h"
#include "Config/ConstDef.h"
//logic module
#include "Room/PlayerRoomData.h"

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

#undef Rpc_Realize
#define Rpc_Realize(typ) void Player::HandleRpc_##typ(NetPack& req, NetPack& ack)

class NetPack;
class GameObject;
class FlatBufferBuilder;
class Player {
    Pool_Index_Define(Player, MAX_PLAYER_COUNT);
private:
    static std::map<uint32, Player*> G_PlayerList;
    NetLinkPtr      _netLink = NULL;
public:
    uint32          m_pid = 0;
    std::string     m_name;
    bool            m_isLogin = false;
public:
    Player(uint32 pid);
    ~Player();
    void    SetNetLink(NetLinkPtr p);
    void    SendMsg(const NetPack& pack);

    uint64  CallRpc(RpcEnum rid, const ParseRpcParam& sendFun);
    void    CallRpc(RpcEnum rid, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun);
    void    SendRpcReplyImmediately();

    static size_t GetPlayerCnt() { return G_PlayerList.size(); }
    static Player* FindByPid(uint32 pid);

    //【Notice】单线程业务逻辑的架构，同一时刻仅有一名玩家CallRpc，所以Builder才能是静态的
    static flatbuffers::FlatBufferBuilder& SendBuild() { return _rpc.SendBuilder; }
    static flatbuffers::FlatBufferBuilder& BackBuild() { return _rpc.BackBuilder; }
public:
    typedef void(Player::*_RpcFunc)(NetPack&, NetPack&);
    static  _RpcFunc      _rpcfunc[RpcEnumCnt]; //自己实现的rpc
    static  RpcQueue<Player>& _rpc;
    Rpc_For_Player;
    
public: //logic module
    PlayerRoomData  m_room;
};