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
    //������������һ��ֻ�Ǽ��
    _isCheck = true; _isWrite = false;
    for (auto& it : resource)
    {
        if (!_Change(player, (Type)it.first, it.second)) return false;
    }

    //���ȫͨ����������д����
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
//������Դ�������
#undef Realize
#define Realize(typ) bool Reward::_Set_##typ(Player& player, int diff)

Realize(Gold)
{
    if (_isCheck)
    {
        int playerGold = 100; //ȡ�������
        if (playerGold + diff < 0)
            return false;
    }
    if (_isWrite)
    {
        //����������
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