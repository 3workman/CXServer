#include "stdafx.h"
#include "CPlayer.h"
#include "raknet/client/UdpClient.h"

std::map<int, CPlayer::_RpcFunc> CPlayer::_rpc;
NetCfgClient CPlayer::_netCfg;

void CPlayer::UpdateNet(){ _netLink->Update(); }

CPlayer::CPlayer()
{
    if (_rpc.empty())
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpc[sRpcClientPlayer.RpcNameToId(#typ)] = &Player::HandleRpc_##typ;
        Rpc_For_Client;
    }

    _netLink = new UdpClient(_netCfg);
    _netLink->SetOnConnect([&](){
        this->CallRpc("rpc_battle_login", [](NetPack& buf){
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
CPlayer::~CPlayer()
{
    _netLink->Stop();
    delete _netLink;
}
uint64 CPlayer::CallRpc(const char* name, const ParseRpcParam& sendFun)
{
    return sRpcClientPlayer._CallRpc(name, sendFun, std::bind(&CPlayer::SendMsg, this, std::placeholders::_1));
}
void CPlayer::CallRpc(const char* name, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun)
{
    uint64 reqKey = CallRpc(name, sendFun);

    sRpcClientPlayer.RegistResponse(reqKey, recvFun);
}
void CPlayer::SendMsg(const NetPack& pack)
{
    _netLink->SendMsg(pack.contents(), pack.size());
}

//////////////////////////////////////////////////////////////////////////
// rpc

