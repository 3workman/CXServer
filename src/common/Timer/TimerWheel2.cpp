// cpp file //////////////////////////////////////////////////
#include "stdafx.h"
#ifdef _MSC_VER
# include <sys/timeb.h>
#else
# include <sys/time.h>
#endif
#include "TimerWheel.h"
#include "tool/GameApi.h"

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)
#define OFFSET(N) (TVR_SIZE + (N) *TVN_SIZE)
#define INDEX(V, N) ((V >> (TVR_BITS + (N) *TVN_BITS)) & TVN_MASK)

//////////////////////////////////////////////////////////////////////////
// Timer
TimerNode::~TimerNode()
{
    Stop();
}

void TimerNode::Start(const std::function<void()>& fun, unsigned interval, TimerType timeType /* = ONCE */)
{
    Stop();
    interval_ = interval;
    timerFun_ = fun;
    timerType_ = timeType;
    expires_ = interval_ + GameApi::TimeMS();
    sTimerMgr._AddTimer(this);
}
void TimerNode::Stop()
{
    if (vecIndex_ != -1)
    {
        sTimerMgr._DelTimer(this);
        vecIndex_ = -1;
    }
}

void TimerNode::OnTimer(uint64 now)
{
    if (timerType_ == TimerNode::CIRCLE)
    {
        expires_ = interval_ + now;
        sTimerMgr._AddTimer(this);
        timerFun_();
    }
    else
    {
        vecIndex_ = -1;
        timerFun_(); //Bug：里面调TimerMgr::DelTimer()，就多次delete了
        delete this;
    }
}

//////////////////////////////////////////////////////////////////////////
// TimerManager

TimerMgr::TimerMgr()
{
    tvec_.resize(TVR_SIZE + 4 * TVN_SIZE);
    checkTime_ = GameApi::TimeMS();
}

void TimerMgr::_AddTimer(TimerNode* timer)
{
    uint64 expires = timer->expires_;
    uint64 idx = expires - checkTime_;

    if (idx < TVR_SIZE)
    {
        timer->vecIndex_ = expires & TVR_MASK;
    }
    else if (idx < 1 << (TVR_BITS + TVN_BITS))
    {
        timer->vecIndex_ = OFFSET(0) + INDEX(expires, 0);
    }
    else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS))
    {
        timer->vecIndex_ = OFFSET(1) + INDEX(expires, 1);
    }
    else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS))
    {
        timer->vecIndex_ = OFFSET(2) + INDEX(expires, 2);
    }
    else if ((long long)idx < 0)
    {
        timer->vecIndex_ = checkTime_ & TVR_MASK;
    }
    else
    {
        if (idx > 0xffffffffUL)
        {
            idx = 0xffffffffUL;
            expires = idx + checkTime_;
        }
        timer->vecIndex_ = OFFSET(3) + INDEX(expires, 3);
    }

    TimeList& tlist = tvec_[timer->vecIndex_];
    tlist.push_back(timer);
    timer->itr_ = tlist.end();
    --timer->itr_;
}

void TimerMgr::_DelTimer(TimerNode* timer)
{
    TimeList& tlist = tvec_[timer->vecIndex_];
    tlist.erase(timer->itr_);
}

void TimerMgr::Refresh(const time_t timenow)
{
    while (checkTime_ <= timenow)
    {
        int index = checkTime_ & TVR_MASK;
        if (!index &&
            !_Cascade(OFFSET(0), INDEX(checkTime_, 0)) &&
            !_Cascade(OFFSET(1), INDEX(checkTime_, 1)) &&
            !_Cascade(OFFSET(2), INDEX(checkTime_, 2)))
        {
            _Cascade(OFFSET(3), INDEX(checkTime_, 3));
        }
        ++checkTime_;

        TimeList& tlist = tvec_[index];
        TimeList temp;
        temp.splice(temp.end(), tlist);
        for (TimeList::iterator itr = temp.begin(); itr != temp.end(); ++itr)
        {
            (*itr)->OnTimer(timenow);
        }
    }
}

int TimerMgr::_Cascade(int offset, int index)
{
    TimeList& tlist = tvec_[offset + index];
    TimeList temp;
    temp.splice(temp.end(), tlist);

    for (TimeList::iterator itr = temp.begin(); itr != temp.end(); ++itr)
    {
        _AddTimer(*itr);
    }

    return index;
}

TimerNode* TimerMgr::AddTimer(const std::function<void()>& f, float delaySec)
{
    TimerNode* p = new TimerNode();
    p->Start(f, uint32(delaySec * 1000));
    return p;
}
void TimerMgr::DelTimer(TimerNode* ptr)
{
    if (ptr->vecIndex_ != -1)
    {
        delete ptr;
    }
}