#include "stdafx.h"
#include "RpcQueue.h"
#include "..\NetLib\server\define.h"
#include "..\svr_game\Player\Player.h"
#include "Buffer\NetPack.h"
#include "RpcEnum.h"

//TODO:换成配表
const std::map<std::string, int> g_rpc_table = {
    // 服务器实现的rpc
    { "rpc_echo",           0 },
    { "rpc_login",          1 },
    { "rpc_logout",         2 },
    { "rpc_reconnect",      3 },
    { "rpc_create_room",    10 },
    { "rpc_join_room",      11 },
    { "rpc_exit_room",      12 },
    { "rpc_move_delta",     13 },

    // 客户端实现的rpc
    { "rpc_notify_player_join_room",    10001 },
    { "rpc_notify_player_exit_room",    10002 },
    { "rpc_sync_position",              10003 },
};


RpcQueue::RpcQueue()
{
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpc[RpcNameToId(#typ)] = &Player::HandleRpc_##typ;
    Rpc_Enum;
}
void RpcQueue::Insert(Player* player, const void* pData, DWORD size)
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