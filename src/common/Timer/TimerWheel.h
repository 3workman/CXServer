#pragma once

#include <functional>
#include "..\tool\Mempool.h"

const char WHEEL_BIT[] = {8, 6, 6, 6, 5}; //�����ۼ�λ�ƣ��ܺͳ�32��λ������(δ������Ϊ)
#define WHEEL_NUM sizeof(WHEEL_BIT)/sizeof(WHEEL_BIT[0])
#define TIME_TICK_LEN 25 //һ��Ŀ̶� ms

struct NodeLink {
    NodeLink* prev;
    NodeLink* next;
    NodeLink() { prev = next = this; } //circle
};
struct TimerNode {
    Pool_Obj_Define(TimerNode, 32)
    NodeLink link; //must in the head
    uint32 timeDead;
    uint32 interval; //������
    int loop;        //�ܹ�ѭ�����
    std::function<void()> func;

    TimerNode(std::function<void()> f, uint32 cd = 0, int total = 0)
        : interval(cd)
        , loop(total)
        , func(f){};
    void _Callback();
};
struct stWheel {
    NodeLink* slots; //ÿ��slotά����node����Ϊһ��������˿��Լ򻯲���ɾ���Ĳ�����slot->nextΪnode�����е�һ���ڵ㣬prevΪnode�����һ���ڵ�
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