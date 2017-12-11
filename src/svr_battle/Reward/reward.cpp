#include "stdafx.h"
#include "reward.h"


static std::map<int, Reward::RewardFunc> g_handler;

Reward::Reward()
{
    _isCheck = _isWrite = false;

#undef Declare
#define Declare(typ, n) g_handler[typ] = &Reward::_Change_##typ;

    Reward_Enum
}
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

    Reward::RewardFunc func = g_handler[typ];
    if (func == NULL)
        return ChangeItem(player, typ, diff);
    else
        return (this->*func)(player, diff);
}

//----------------------------------------------------------
//各类资源变更函数
bool Reward::ChangeItem(Player& player, int itemId, int diff)
{
    //if (_isCheck) {
    //    if (player.m_bag->CountItem(itemId) + diff < 0)
    //        return false;
    //}
    //if (_isWrite) {
    //    if (diff > 0) {
    //        player.CreateItem(pb::IR_OPERATION_REWARD_GET, itemId, diff);
    //    }
    //    else {
    //        player.DestroyItem(pb::IR_LOTTO_COST, itemId, -diff);
    //    }
    //}
    return true;
}

#undef Realize
#define Realize(typ) bool Reward::_Change_##typ(Player& player, int diff)
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
//Realize(Item)
//{
//    int cnt = diff & 0xFF;
//    int itemId = diff >> 8;
//    return false;
//}