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
    std::pair<Player*, NetPack*> data;
    if (_queue.pop(data))
    {
        NetPack& backBuffer = data.first->BackBuffer();
        backBuffer.SetOpCode(data.second->GetOpcode());
        backBuffer.SetPacketType(data.second->GetPacketType());

        (data.first->*_func[data.second->GetOpcode()])(*data.second);
        delete data.second;

        if (backBuffer.BodyBytes()) {
            data.first->SendPack(backBuffer);
            backBuffer.Clear();
        }
    }
}