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
    printf("aaaaaaaaaaaaaaaa\n");
}
Msg_Realize(C2S_Echo)
{
    TestMsg& msg = (TestMsg&)req;
    char* str = ((char*)&msg) + 4;
    SendMsg(msg, 4 + strlen(str) + 1);
    printf("Echo: %s\n", str);
}