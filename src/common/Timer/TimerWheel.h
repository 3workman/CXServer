/***********************************************************************
* @ 时间轮算法
* @ brief
    1、与时间有关的周期性重复执行，须考虑卡顿后的追帧问题，参见Service.h的List、Patch差异

    2、TimerNode::_Callback 里重新放入轮子
        *、不应再取系统时间，否则不会卡顿后立即追帧，且该注册函数的执行总时长会变长，易引起错乱

* @ author zhoumf
* @ date 2017-2-20
************************************************************************/
#pragma once

#include <functional>
#include "tool/Mempool.h"

const char WHEEL_BIT[] = {8, 6, 6, 6, 5}; //用了累计位移，总和超32，位操作会(未定义行为)
#define WHEEL_NUM sizeof(WHEEL_BIT)/sizeof(WHEEL_BIT[0])
#define TIME_TICK_LEN 25 //一格的刻度 ms

struct NodeLink {
    NodeLink* prev;
    NodeLink* next;
    NodeLink() { prev = next = this; } //circle
};
struct TimeNode {
    Pool_Obj_Define(TimeNode, 32)
    NodeLink link; //must in the head
    time_t   when;
	bool     _funcIng = false;
    std::function<int()> func;

    TimeNode(const std::function<int()>& f) : when(0), func(f){};
    void _Callback();
    inline void Stop();
};
struct stWheel {
    NodeLink* slots; //每个slot维护的node链表为一个环，如此可以简化插入删除的操作。slot->next为node链表中第一个节点，prev为node的最后一个节点
    const uint32 size;
    uint32 slotIdx;
    stWheel(uint32 n) : size(n), slotIdx(0){ slots = new NodeLink[size]; }
    ~stWheel() {
        if (slots) {
            for (uint32 j = 0; j < size; ++j) {
                NodeLink* link = (slots + j)->next;
                while (link != slots + j) {
                    TimeNode* node = (TimeNode*)link;
                    link = node->link.next;
                    delete node;
                }
            }
            delete[]slots;
        }
    }
    NodeLink* GetCurSlot() { return slots + slotIdx; }
};
class TimeWheel {
    static uint32 WHEEL_MASK[WHEEL_NUM];
    static uint32 WHEEL_CAP[WHEEL_NUM];

    stWheel* _wheels[WHEEL_NUM];
    NodeLink _readyNode;
    uint32   _time_elapse = 0;

    TimeWheel();
    ~TimeWheel();
public:
    static TimeWheel& Instance(){ static TimeWheel T; return T; }
    void Refresh(uint32 time_elasped, const time_t timenow);

    TimeNode* AddTimer(const std::function<int()>& f, float delaySec);
    void _AddTimerNode(uint32 milseconds, TimeNode* node);
    void DelTimer(TimeNode* node);
private:
    void Cascade(uint32 wheelIdx, const time_t timenow);
    void AddToReadyNode(TimeNode* node);
    void DoTimeOutCallBack();
    void Printf();
};
#define sTimerMgr TimeWheel::Instance()
