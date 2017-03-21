#include "stdafx.h"
#include "TimerWheel.h"
#include <windows.h>
#include "..\Log\LogFile.h"

uint32 CTimerMgr::WHEEL_SIZE[] = {};
uint32 CTimerMgr::WHEEL_CAP[] = {};

CTimerMgr::CTimerMgr() {
    assert(SUM_ARR(WHEEL_BIT, WHEEL_NUM) < 32);
    _checkTime = TimeElasped_Msec;
    for (int i = 0; i < WHEEL_NUM; ++i) {
        WHEEL_SIZE[i] = 1 << WHEEL_BIT[i];
        //WHEEL_CAP[i] = i > 0 ? (WHEEL_CAP[i-1]*WHEEL_SIZE[i]) : WHEEL_SIZE[i]; //��һ���̶����� = ��һ���̶����� * ����������
        WHEEL_CAP[i] = 1 << SUM_ARR(WHEEL_BIT, i+1);
        _wheels[i] = new stWheel(WHEEL_SIZE[i]);
    }
}
CTimerMgr::~CTimerMgr() {
    for (int i = 0; i < WHEEL_NUM; ++i) {
        if (_wheels[i]) delete _wheels[i];
    }
}
void TimerNode::_Callback(){
    loop -= interval;
    if (loop >= 0) {
        //timeDead = TimeElasped_Msec + interval;
        timeDead += interval; //Notice������ִ�еĺ���������������Ӧ��׷֡����ȡϵͳ��ǰʱ���Ǵ��
        CTimerMgr::Instance()._AddTimerNode(interval, this);
        func(); //must at the last line; timer may be deleted in _func();
    } else {
        func();
        delete this;
    }
}
TimerNode* CTimerMgr::AddTimer(const std::function<void()>& f, uint32 delaySec, uint32 cdSec /* = 0 */, int totalSec /* = 0 */) {
    TimerNode* node = new TimerNode(f, cdSec * 1000, totalSec * 1000);
    node->timeDead = TimeElasped_Msec + delaySec * 1000;
    _AddTimerNode(delaySec * 1000, node);
    return node;
}
void CTimerMgr::_AddTimerNode(uint32 milseconds, TimerNode* node) {
    NodeLink* slot = NULL;
    uint32 tickCnt = milseconds / TIME_TICK_LEN;

    if (tickCnt < WHEEL_CAP[0]) {
        uint32 index = (_wheels[0]->slotIdx + tickCnt) & (WHEEL_SIZE[0] - 1); //2��N����λ����ȡ��
        slot = _wheels[0]->slots + index;
    } else {
        for (int i = 1; i < WHEEL_NUM; ++i) {
            if (tickCnt < WHEEL_CAP[i]) {
                uint32 preCap = WHEEL_CAP[i - 1]; //��һ����������Ϊ������һ������
                uint32 index = (_wheels[i]->slotIdx + tickCnt / preCap - 1) & (WHEEL_SIZE[i] - 1); //����-1
                slot = _wheels[i]->slots + index;
                break;
            }
        }
    }
    NodeLink* link = &(node->link);
    link->prev = slot->prev; //������ӵ�prevλ��(β�ڵ�)
    link->prev->next = link;
    link->next = slot;
    slot->prev = link;
}
void CTimerMgr::RemoveTimer(TimerNode* node) {
    LOG_TRACK("node[%p], timeDead[%lld]", node, node->timeDead);
    NodeLink* link = &(node->link);
    if (link->prev) {
        link->prev->next = link->next;
    }
    if (link->next) {
        link->next->prev = link->prev;
    }
    link->prev = link->next = NULL;

    delete node;
}
void CTimerMgr::Refresh(const uint32 timenow) {
    uint32 tickCnt = timenow > _checkTime ? (timenow - _checkTime) / TIME_TICK_LEN : 0;
    //if (tickCnt) Printf();
    for (uint32 i = 0; i < tickCnt; ++i) { //ɨ����slot����ʱ
        stWheel* wheel = _wheels[0];
        NodeLink* slot = wheel->slots + wheel->slotIdx;
        NodeLink* link = slot->next;
        slot->next = slot->prev = slot; //��յ�ǰ����
        while (link != slot) {          //�����������
            TimerNode* node = (TimerNode*)link;
            link = node->link.next; //�÷���ǰ�棬�����������ã����ܻ����node�����ӹ�ϵ
            AddToReadyNode(node);
        }
        if (++(wheel->slotIdx) >= wheel->size) {
            wheel->slotIdx = 0;
            Cascade(1, timenow); //����
        }
        _checkTime += TIME_TICK_LEN;
    }
    DoTimeOutCallBack();
}
void CTimerMgr::AddToReadyNode(TimerNode* node) {
    NodeLink* link = &(node->link);
    link->prev = _readyNode.prev;
    link->prev->next = link;
    link->next = &_readyNode;
    _readyNode.prev = link;
}
void CTimerMgr::DoTimeOutCallBack() {
    NodeLink* link = _readyNode.next;
    while (link != &_readyNode) {
        TimerNode* node = (TimerNode*)link;
        link = node->link.next;
        node->_Callback();
    }
    _readyNode.next = _readyNode.prev = &_readyNode;
}
uint32 CTimerMgr::Cascade(uint32 wheelIdx, const uint32 timenow) {
    if (wheelIdx < 1 || wheelIdx >= WHEEL_NUM) {
        return 0;
    }
    int casCnt = 0;
    stWheel* wheel = _wheels[wheelIdx];
    NodeLink* slot = wheel->slots + wheel->slotIdx;
    NodeLink* link = slot->next;
    slot->next = slot->prev = slot; //��յ�ǰ����
    while (link != slot) {
        TimerNode* node = (TimerNode*)link;
        link = node->link.next;
        if (node->timeDead <= timenow) {
            AddToReadyNode(node);
        } else {
            _AddTimerNode(node->timeDead - timenow, node); //�����������ѳ�ʱ���������������¼�һ��
            ++casCnt;
            LOG_TRACK("wheelIdx[%u], link[%p], milseconds[%u]", wheelIdx, link, node->timeDead - timenow);
        }
    }
    if (++(wheel->slotIdx) >= wheel->size) {
        wheel->slotIdx = 0;
        casCnt += Cascade(++wheelIdx, timenow);
    }
    return casCnt;
}
void CTimerMgr::Printf() {
    LOG_TRACK("=======Printf=======");
    for (int i = 0; i < WHEEL_NUM; ++i) {
        stWheel* wheel = _wheels[i];
        LOG_TRACK("wheel[%d].size[%u], slotIdx[%u]", i, wheel->size, wheel->slotIdx);
        for (uint32 j = 0; j < wheel->size; ++j) {
            NodeLink* slot = wheel->slots + j;
            NodeLink* link = slot->next;
            if (link != slot) {
                LOG_TRACK("slotIdx[%d], addr[%p], next[%p], prev[%p]", j, slot, slot->next, slot->prev);
            }
            while (link != slot) {
                TimerNode* node = (TimerNode*)link;
                link = node->link.next;
                LOG_TRACK("node[%p], next[%p], prev[%p] timeDead[%lld]", link, link->next, link->prev, node->timeDead);
            }
        }
    }
}