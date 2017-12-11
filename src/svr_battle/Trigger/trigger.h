/***********************************************************************
* @ 通用条件判断模块
* @ brief
    1、把活丢给策划
    2、一些与玩家数据无关的条件，指针会主动传NULL

    3、array式组织结构，对比reward的map式组织结构
* @ author zhoumf
* @ date 2016-11-21
************************************************************************/
#pragma once

#undef Declare
#undef Trigger_Enum
#define Declare(typ) typ,
#define Trigger_Enum\
    Declare(UpLevel)        /*等级超过*/\
    Declare(DuringTime)     /*在两时间点之间*/\


class Player;
struct TriggerTable
{
    uint16      id;
    uint16      type;
    int32       val1; //1223145632：12月23号14:56:32
    int32       val2;
};
class Trigger {
    typedef std::map<int, const TriggerTable> TriggerMap;
    TriggerMap m_TriggerLst;

    Trigger();
public:
    typedef bool(Trigger::*TriggerFunc)(Player*, int32, int32);
    enum Type : uint {
        Trigger_Enum

        MAX_ENUM
    };
    static Trigger& Instance(){ static Trigger T; return T; }

    bool Check(Player* player, const int triggerId);
    bool Check(Player* player, const std::vector<int>& triggerIds);

    //各类判断函数，这里就不检查空指针了，逻辑层负责
public:
#undef Declare
#define Declare(typ) bool _Is_##typ(Player* player, int32 val1, int32 val2);
    Trigger_Enum
};
#define sTrigger Trigger::Instance()
