#include "stdafx.h"
#include "MsgPool.h"
#include "..\NetLib\server\define.h"
#include "..\svr_game\Player\Player.h"
#include "MsgEnum.h"

static const int kMaxMsgSize = 512;

MsgPool::MsgPool() : _pool(kMaxMsgSize, 4096)
{
#undef Msg_Declare
#define Msg_Declare(typ, n) _func[typ] = &Player::HandleMsg_##typ;
    Msg_Enum;
}
void MsgPool::Insert(Player* player, void* pData, DWORD size)
{
    assert(size <= kMaxMsgSize);
    stMsg* pMsg = (stMsg*)_pool.Alloc();
    memcpy(pMsg, pData, size);
    _queue.push(std::make_pair(player, pMsg));
}
void MsgPool::Handle()
{
    std::pair<Player*, stMsg*> data;
    if (_queue.pop(data))
    {
        (data.first->*_func[data.second->msgId])(*data.second);
        _pool.Dealloc(data.second);
    }
}