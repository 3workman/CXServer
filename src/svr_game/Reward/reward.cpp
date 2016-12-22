#include "stdafx.h"
#include "reward.h"


static const Reward::ResourceFunc g_handler[] = {
    &Reward::_SetGold,
    &Reward::_SetDiamond,
    &Reward::_SetExp,
    &Reward::_SetHeroExp,
    &Reward::_SetItem,
};
STATIC_ASSERT_ARRAY_LENGTH(g_handler, Reward::MAX_ENUM);


bool Reward::Change(Player& player, Type typ, const int diff)
{
    _isCheck = _isWrite = true;

    return _Change(player, typ, diff);
}
bool Reward::Change(Player& player, const IntPairVec& resource)
{
    //批量操作，第一次只是检查
    _isCheck = true; _isWrite = false;
    for (auto& it : resource)
    {
        if (!_Change(player, (Type)it.first, it.second)) return false;
    }

    //检查全通过，真正改写数据
    _isCheck = false; _isWrite = true;
    for (auto& it : resource) _Change(player, (Type)it.first, it.second);
    return true;
}
bool Reward::_Change(Player& player, Type typ, int diff)
{
    if (diff == 0) return false;

    if (typ < MAX_ENUM)
        return (this->*g_handler[typ])(player, diff);
    else
        return false;
}

//----------------------------------------------------------
//各类资源变更函数
bool Reward::_SetGold(Player& player, int diff)
{
    if (_isCheck)
    {
        int playerGold = 100; //取玩家数据
        if (playerGold + diff < 0)
            return false;
    }
    if (_isWrite)
    {
        //变更玩家数据
    }
    return true;
}
bool Reward::_SetDiamond(Player& player, int diff)
{
    return false;
}
bool Reward::_SetExp(Player& player, int diff)
{
    return false;
}
bool Reward::_SetHeroExp(Player& player, int diff)
{
    int heroIdx = diff & 0xFF;
    int exp = diff >> 8;
    return false;
}
bool Reward::_SetItem(Player& player, int diff)
{
    int itemId = diff & 0xFFFF;
    int cnt = diff >> 16;
    return false;
}