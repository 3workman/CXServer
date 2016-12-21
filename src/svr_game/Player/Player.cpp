#include "stdafx.h"
#include "..\NetLib\server\ServLink.h"
#include "Player.h"
#include "..\msg\TestMsg.h"

Player::Player(ServLink* p)
    : _clientNetLink(p)
{

}
void Player::SendMsg(stMsg& msg, DWORD size)
{
    _clientNetLink->SendMsg(&msg, size);
}
Msg_Realize(Login)
{
    printf("aaaaaaaaaaaaaaaa\n");
}
Msg_Realize(Echo)
{
    TestMsg& msg = (TestMsg&)req;
    char* str = ((char*)&msg) + 4;
    SendMsg(msg, 4 + strlen(str) + 1);
    printf("Echo: %s\n", str);
}