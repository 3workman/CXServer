#include "stdafx.h"
#include "..\NetLib\server\ServLink.h"
#include "Player.h"
#include "..\msg\TestMsg.h"
#include "Buffer\NetPack.h"

NetPack& Player::_backBuffer = NetPack(0);

Player::Player(ServLink* p)
    : _clientNetLink(p)
{

}
void Player::SetServLink(ServLink* p)
{
    _clientNetLink = p;
}
void Player::SendMsg(const stMsg& msg, uint16 size)
{
    _clientNetLink->SendMsg(&msg, size);
}
void Player::SendPack(const NetPack& pack)
{
    _clientNetLink->SendMsg(pack.Buffer(), pack.Size());
}
//////////////////////////////////////////////////////////////////////////
// msg 响应函数实现
Msg_Realize(C2S_Login)
{
    printf("C2S_Login\n");
}
Msg_Realize(C2S_ReConnect)
{
    printf("C2S_ReConnect\n");
}
Msg_Realize(C2S_Echo)
{
    TestMsg& msg = (TestMsg&)req;
    char* str = msg.data;
    SendMsg(msg, msg.size());
    printf("Echo: %s\n", str);
}


//////////////////////////////////////////////////////////////////////////
// rpc
Rpc_Realize(rpc_login) { printf("rpc_login\n"); }
Rpc_Realize(rpc_reconnect) { printf("rpc_reconnect\n"); }
Rpc_Realize(rpc_echo)
{
    string str = recvBuf.ReadString();
    printf("Echo: %s\n", str.c_str());
    NetPack& backBuffer = BackBuffer();
    backBuffer << str;
}