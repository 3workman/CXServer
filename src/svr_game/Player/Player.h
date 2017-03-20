#pragma once
#include "tool\Mempool.h"
#include "..\msg\MsgEnum.h"
#include "..\rpc\RpcEnum.h"

struct stMsg;
#undef Msg_Declare
#undef Msg_Realize
#define Msg_Declare(typ, n) void HandleMsg_##typ(stMsg&);
#define Msg_Realize(typ) void Player::HandleMsg_##typ(stMsg& req)

class NetPack;
#undef Rpc_Declare
#undef Rpc_Realize
#define Rpc_Declare(typ, n) void HandleRpc_##typ(NetPack&);
#define Rpc_Realize(typ) void Player::HandleRpc_##typ(NetPack& recvBuf)

class ServLink;

class Player {
    Pool_Index_Define(Player, 50);
private:
    ServLink*       _clientNetLink;
    static NetPack& _backBuffer;

public:
    Player(ServLink* p);

    void SetServLink(ServLink* p);
    void SendMsg(const stMsg& msg, uint16 size);
    void SendPack(const NetPack& pack);
    NetPack& BackBuffer() { return _backBuffer; }
public:
    Msg_Enum;
    Rpc_Enum;
};
