#include "stdafx.h"
#include "RpcQueue.h"
#include "..\NetLib\server\define.h"
#include "..\svr_game\Player\Player.h"
#include "Buffer\NetPack.h"
#include "RpcEnum.h"

RpcQueue::RpcQueue()
{
#undef Rpc_Declare
#define Rpc_Declare(typ, n) _func[typ] = &Player::HandleRpc_##typ;
    Rpc_Enum;
}
void RpcQueue::Insert(Player* player, void* pData, DWORD size)
{
    _queue.push(std::make_pair(player, new NetPack(pData, size)));
}
void RpcQueue::Handle()
{
    uint16 opCode = 0;
    NetPack* pMsg = NULL;
    Player* player = NULL;
    std::pair<Player*, NetPack*> data;
    if (_queue.pop(data))
    {
        player = data.first;
        pMsg = data.second;
        opCode = pMsg->GetOpcode();

        if (_rpc.find(opCode) != _rpc.end())
        {
            NetPack& backBuffer = player->BackBuffer();
            backBuffer.ClearBody();
            backBuffer.SetOpCode(pMsg->GetOpcode());
            backBuffer.SetPacketType(pMsg->GetPacketType());

            (player->*_rpc[opCode])(*pMsg);

            if (backBuffer.BodyBytes()) player->SendPack(backBuffer);
        }
        else if (_response.find(opCode) != _response.end())
        {
            _response[opCode](*pMsg);
        }

        delete pMsg;
    }
}
void RpcQueue::RegistResponse(int opCode, const ReadRpcBack& func)
{
    assert(_rpc.find(opCode) == _rpc.end());

    _response.insert(make_pair(opCode, func));
}