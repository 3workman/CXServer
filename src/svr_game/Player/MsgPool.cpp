#include "stdafx.h"
#include "MsgPool.h"
#include "..\NetLib\server\define.h"
#include "Player.h"

MsgPool::MsgPool() : _pool(512, 4096)
{
    memset(_func, 0, sizeof(_func));

#undef Msg_Declare
#define Msg_Declare(e) _func[e] = &Player::HandleMsg_##e;
#include "PlayerMsg.h"

#ifdef _DEBUG
    for (auto& it : _func) assert(it);
#endif
}
void MsgPool::Insert(Player* player, stMsg* msg, DWORD size)
{
    stMsg* pMsg = (stMsg*)_pool.Alloc();
    memcpy(pMsg, msg, size);
    _queue.push(std::make_pair(player, pMsg));
}
void MsgPool::Handle()
{
    std::pair<Player*, stMsg*> data;
    if (_queue.pop(data))
    {
        (data.first->*_func[data.second->msgId])(*data.second);
    }
}