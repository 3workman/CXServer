#include "stdafx.h"
#include "reward.h"

#undef Declare
#define Declare(typ) &Reward::_Set_##typ,
static const Reward::ResourceFunc g_handler[] = {
    Reward_Enum
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
#undef Realize
#define Realize(typ) bool Reward::_Set_##typ(Player& player, int diff)

Realize(Gold)
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
Realize(Diamond)
{
    return false;
}
Realize(Exp)
{
    return false;
}
Realize(HeroExp)
{
    int heroIdx = diff & 0xFF;
    int exp = diff >> 8;
    return false;
}
Realize(Item)
{
    int itemId = diff & 0xFFFF;
    int cnt = diff >> 16;
    return false;
}