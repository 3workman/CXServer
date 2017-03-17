#include "stdafx.h"
#include "..\NetLib\server\ServLink.h"
#include "Player.h"
#include "..\msg\TestMsg.h"

Player::Player(ServLink* p)
    : _clientNetLink(p)
{

}
void Player::SetServLink(ServLink* p)
{
    _clientNetLink = p;
}
void Player::SendMsg(stMsg& msg, DWORD size)
{
    _clientNetLink->SendMsg(&msg, size);
}

Msg_Realize(C2S_Login)
{
    printf("aaa\n");
}
Msg_Realize(C2S_ReConnect)
{
    printf("bbb\n");
}
Msg_Realize(C2S_Echo)
{
    TestMsg& msg = (TestMsg&)req;
    char* str = msg.data;
    SendMsg(msg, msg.size());
    printf("Echo: %s\n", str);
}