#include "stdafx.h"
#include "Player.h"
#include "../NetLib/UdpClient/UdpClient.h"

std::map<int, Player::_RpcFunc> Player::_rpc;


void Player::UpdateNet(){ _netLink->Update(); }

Player::Player()
{
    if (_rpc.empty())
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpc[sRpcClientPlayer.RpcNameToId(#typ)] = &Player::HandleRpc_##typ;
        Rpc_For_Client;
    }

    _netLink = new UdpClient();
    _netLink->SetOnConnect([&](){
        this->CallRpc("rpc_login", [](NetPack& buf){
            buf.WriteUInt32(1);
        },
            [&](NetPack& recvBuf){
            recvBuf >> this->m_index;
            this->m_isLogin = true;
        });
    });
    _netLink->Start([&](void* p, int size){
        NetPack msg(p, size);
        sRpcClientPlayer._Handle(this, msg);
    });
}
Player::~Player()
{
    _netLink->Stop();
    delete _netLink;
}
uint64 Player::CallRpc(const char* name, const ParseRpcParam& sendFun)
{
    return sRpcClientPlayer._CallRpc(name, sendFun, std::bind(&Player::SendMsg, this, std::placeholders::_1));
}
void Player::CallRpc(const char* name, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun)
{
    uint64 reqKey = CallRpc(name, sendFun);

    sRpcClientPlayer.RegistResponse(reqKey, recvFun);
}
void Player::SendMsg(const NetPack& pack)
{
    _netLink->SendMsg(pack.Buffer(), pack.Size());
}

//////////////////////////////////////////////////////////////////////////
// rpc

