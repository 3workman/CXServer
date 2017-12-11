#include "stdafx.h"
#include "CPlayer.h"
#include "raknet/client/UdpClient.h"

CPlayer::_RpcFunc  CPlayer::_rpcfunc[RpcEnumCnt] = {0};
RpcQueue<CPlayer>& CPlayer::_rpc = RpcQueue<CPlayer>::Instance();

void CPlayer::UpdateNet(){ _netLink->Update(); }

CPlayer::CPlayer()
{
    //if (!_rpcfunc[])
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpcfunc[typ] = &Player::HandleRpc_##typ;
        Rpc_For_Client;
    }

    _netLink = new UdpClient(_netCfg);

    RunClientNet();
}
CPlayer::~CPlayer()
{
    _netLink->Stop();
    delete _netLink;
}
void CPlayer::RunClientNet()
{
    _netLink->SetOnConnect([&](){
        this->CallRpc(rpc_battle_login, [](NetPack& buf){
            buf.WriteUInt32(1);
        },
            [&](NetPack& recvBuf){
            recvBuf >> this->m_index;
            this->m_isLogin = true;
        });
    });
    _netLink->Start([&](void* p, int size){
        NetPack msg(p, size);
        _rpc._Handle(this, msg);
    });
}
uint64 CPlayer::CallRpc(RpcEnum rid, const ParseRpcParam& sendFun)
{
    return _rpc._CallRpc(rid, sendFun, std::bind(&CPlayer::SendMsg, this, std::placeholders::_1));
}
void CPlayer::CallRpc(RpcEnum rid, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun)
{
    uint64 reqKey = CallRpc(rid, sendFun);

    _rpc.RegistResponse(reqKey, recvFun);
}
void CPlayer::SendMsg(const NetPack& pack)
{
    _netLink->SendMsg(pack.contents(), pack.size());
}

//////////////////////////////////////////////////////////////////////////
// rpc
#undef Rpc_Realize
#define Rpc_Realize(typ) void CPlayer::HandleRpc_##typ(NetPack& req, NetPack& ack)
