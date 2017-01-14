/***********************************************************************
* @ 资源增减
* @ brief
    1、统一起来，便于管理，散乱的扣钱扣金币...太繁琐了

    2、批量扣资源时，注意中途失败的情况

    3、此类敏感数据，安全性做高些，尤其注意零值...配表为空，读出0，然后Cut成功，就惨了

* @ return ((!_isCheck) || retCheck) && ((!_isWrite) || retWrite);
    1、要检查，则检查结果由ret决定
    2、不检查，则检查结果恒true
    3、写入的情形一致的

* @ author zhoumf
* @ date 2016-11-22
************************************************************************/
#pragma once

//Notice：须同道具表一致(item_proto)
#undef Declare
#undef Reward_Enum
#define Declare(typ) typ,
#define Reward_Enum\
    Declare(Gold)       \
    Declare(Diamond)    \
    Declare(Exp)        \
    Declare(HeroExp)    /*英雄经验*/\
    Declare(Item)       /*物品*/\


class Player;
class Reward {
    bool _isCheck;
    bool _isWrite;
public:
    typedef bool(Reward::*ResourceFunc)(Player&, int);
    enum Type : uint {
        Reward_Enum

        MAX_ENUM
    };
    static Reward& Instance(){ static Reward T; return T; }

    bool Change(Player& player, Type typ, int diff);
    bool Change(Player& player, const IntPairVec& resource);
    bool _Change(Player& player, Type typ, int diff);

	//辅助函数
    //Notice：优先级问题啊，不要想当然（。－_－。）
    static inline int PackHeroExp(uint8 heroIdx, int exp){ return (exp << 8) + heroIdx; } // 低8位:第几个Hero，高24位:经验值
    static inline int PackItem(uint16 itemId, uint16 cnt){ return (cnt << 16) + itemId; } // 低16位:物品ID，高16位:数量

    //各类资源变更函数
public:
#undef Declare
#define Declare(typ) bool _Set_##typ(Player& player, int diff);

    Reward_Enum
};
#define sResource Reward::Instance()
