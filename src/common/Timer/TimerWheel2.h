// header file //////////////////////////////////
#pragma once
#include <list>
#include <vector>
#include <functional>
#include "tool/Mempool.h"

class TimerMgr;
struct TimerNode {
    Pool_Obj_Define(TimerNode, 2)
public:
    friend class TimerMgr;
    enum TimerType { ONCE, CIRCLE };

    ~TimerNode();

    void Start(const std::function<void()>& fun, unsigned interval, TimerType timeType = ONCE);
    void Stop();

private:
    void OnTimer(uint64 now);

private:
    TimerType timerType_;
    std::function<void()> timerFun_;
    unsigned interval_;
    uint64   expires_;

    int vecIndex_ = -1;
    std::list<TimerNode*>::iterator itr_;
};

class TimerMgr
{
public:
    TimerMgr();
    static TimerMgr& Instance() { static TimerMgr T; return T; }
    void   Refresh(const time_t timenow);
    TimerNode* AddTimer(const std::function<void()>& f, float delaySec);
    void   DelTimer(TimerNode* ptr);

private:
    friend struct TimerNode;
    void _AddTimer(TimerNode* timer);
    void _DelTimer(TimerNode* timer);
    int  _Cascade(int offset, int index);

private:
    typedef std::list<TimerNode*> TimeList;
    std::vector<TimeList> tvec_;
    uint64 checkTime_;
};
#define sTimerMgr TimerMgr::Instance()