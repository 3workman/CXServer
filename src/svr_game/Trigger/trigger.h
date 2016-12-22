/***********************************************************************
* @ 通用条件判断模块
* @ brief
    1、把活丢给策划
    2、一些与玩家数据无关的条件，指针会主动传NULL

* @ author zhoumf
* @ date 2016-11-21
************************************************************************/
#pragma once

class Player;
struct TriggerTable
{
    uint16      id;
    uint16      type;
    uint32      val1; //1223145632：12月23号14:56:32
    uint32      val2;
};
class Trigger {
    typedef std::map<int, const TriggerTable> TriggerMap;
    TriggerMap m_TriggerLst;

    Trigger();
public:
    typedef bool(Trigger::*TriggerFunc)(Player&, uint32, uint32);
    enum {
        UpLevel,
        DuringTheTime,

        MAX_ENUM
    };
    static Trigger& Instance(){ static Trigger T; return T; }

    bool Check(Player& player, const int triggerId);
    bool Check(Player& player, const std::vector<int>& triggerIds);

    //各类判断函数
    bool IsUpLevel(Player& player, uint32 val1, uint32 val2);
    bool IsDuringTime(Player& player, uint32 val1, uint32 val2);
};
#define sTrigger Trigger::Instance()
