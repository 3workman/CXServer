#include "stdafx.h"
#include "RpcQueue.h"
#include "../svr_game/Player/Player.h"
#include "Buffer/NetPack.h"
#include "RpcEnum.h"
#include "Csv/CSVparser.hpp"

std::map<std::string, int> g_rpc_table;
static void LoadRpcCsv()
{
    csv::Parser file = csv::Parser("../data/csv/rpc.csv");
    uint cnt = file.rowCount();
    for (uint i = 0; i < cnt; ++i) {
        csv::Row& row = file[i];
        g_rpc_table[row["name"]] = atoi(row["id"].c_str());
    }
}


RpcQueue::RpcQueue()
{
    LoadRpcCsv();
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpc[RpcNameToId(#typ)] = &Player::HandleRpc_##typ;
    Rpc_Enum;
}
void RpcQueue::Insert(Player* player, const void* pData, uint size)
{
    _queue.push(std::make_pair(player, new NetPack(pData, size)));
}
void RpcQueue::Update()
{
    std::pair<Player*, NetPack*> data;
    if (_queue.pop(data))
    {
        _Handle(data.first, *data.second);

        delete data.second;
    }
}
void RpcQueue::_Handle(Player* player, NetPack& buf)
{
    uint16 opCode = buf.GetOpcode();

    auto it = _rpc.find(opCode);
    if (it != _rpc.end()) {
        NetPack& backBuffer = player->BackBuffer();
        backBuffer.ClearBody();
        backBuffer.SetOpCode(buf.GetOpcode());
        backBuffer.SetPacketType(buf.GetPacketType());

        (player->*(it->second))(buf);

        if (backBuffer.BodyBytes()) player->SendMsg(backBuffer);
    } else {
        auto it = _response.find(opCode);
        if (it != _response.end()) it->second(buf);
    }
}
void RpcQueue::RegistResponse(int opCode, const RecvRpcParam& func)
{
    assert(_rpc.find(opCode) == _rpc.end());

    _response.insert(make_pair(opCode, func));
}
int RpcQueue::RpcNameToId(const char* name)
{
    auto it = g_rpc_table.find(name);
    if (it == g_rpc_table.end()) {
        assert(0);
        return 0;
    }
    return it->second;
}