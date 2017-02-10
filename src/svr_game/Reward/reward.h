/***********************************************************************
* @ 资源增减
* @ brief
    1、统一起来，便于管理，散乱的扣钱扣金币...太繁琐了

    2、批量扣资源时，注意中途失败的情况

    3、此类敏感数据，安全性做高些，尤其注意零值...配表为空，读出0，然后Cut成功，就惨了

    4、枚举的设置，应根据项目表情况变通。比如可将资源也配在Item表中，枚举与itemId一致，g_handler变成map结构

    5、PackItem()这样的接口最好删掉，太容易出错。找不到处理函数的，默认调物品接口。其它需要Pack**的类型就别放进来，业务层自己管

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
    static inline int PackItem(int itemId, char cnt){ return (itemId << 8) + cnt; } // 低8位:数量，高24位:物品ID

    //各类资源变更函数
public:
#undef Declare
#define Declare(typ) bool _Set_##typ(Player& player, int diff);

    Reward_Enum
};
#define sResource Reward::Instance()
