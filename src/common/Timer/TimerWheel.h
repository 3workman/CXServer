#pragma once

#include <functional>
#include "..\tool\Mempool.h"

const char WHEEL_BIT[] = {8, 6, 6, 6, 5}; //用了累计位移，总和超32，位操作会(未定义行为)
#define WHEEL_NUM sizeof(WHEEL_BIT)/sizeof(WHEEL_BIT[0])
#define TIME_TICK_LEN 25 //一格的刻度 ms

struct NodeLink {
    NodeLink* prev;
    NodeLink* next;
    NodeLink() { prev = next = this; } //circle
};
struct TimerNode {
    Pool_Obj_Define(TimerNode, 32)
    NodeLink link; //must in the head
    uint32 timeDead;
    uint32 interval; //间隔多久
    int loop;        //总共循环多久
    std::function<void()> func;

    TimerNode(std::function<void()> f, uint32 cd = 0, int total = 0)
        : interval(cd)
        , loop(total)
        , func(f){};
    void _Callback();
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
                    TimerNode* node = (TimerNode*)link;
                    link = node->link.next;
                    delete node;
                }
            }
            delete[]slots;
        }
    }
};
class CTimerMgr {
    static uint32 WHEEL_SIZE[WHEEL_NUM];
    static uint32 WHEEL_CAP[WHEEL_NUM];

    stWheel* _wheels[WHEEL_NUM];
    uint32 _checkTime;
    NodeLink _readyNode;

    CTimerMgr();
    ~CTimerMgr();
public:
    static CTimerMgr& Instance(){ static CTimerMgr T; return T; }
    void Refresh(const uint32 timenow);

    TimerNode* AddTimer(std::function<void()> f, uint32 delaySec, uint32 cdSec = 0, int totalSec = 0);
    void _AddTimerNode(uint32 milseconds, TimerNode* node);

    void RemoveTimer(TimerNode* node);
private:
    uint32 Cascade(uint32 wheelIdx, const uint32 timenow);
    void AddToReadyNode(TimerNode* node);
    void DoTimeOutCallBack();
    void Printf();
};
#define sTimerMgr CTimerMgr::Instance()