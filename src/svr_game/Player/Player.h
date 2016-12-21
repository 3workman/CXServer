#pragma once
#include "tool\Mempool.h"

struct stMsg;
#define Msg_Declare(e) void HandleMsg_##e(stMsg&);
#define Msg_Realize(e) void Player::HandleMsg_##e(stMsg& req)

class ServLink;

class Player {
    Pool_Index_Define(Player, 50);
private:
    ServLink* const _clientNetLink;
public:


public:
    Player(ServLink* p);

    void SendMsg(stMsg& msg, DWORD size);
public:
    #include "PlayerMsg.h"
};
