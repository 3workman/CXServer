#include "stdafx.h"
#include "TimerWheel.h"
#include "tool/GameApi.h"
#include "Log/LogFile.h"

uint32 TimeWheel::WHEEL_MASK[] = {};
uint32 TimeWheel::WHEEL_CAP[] = {};

TimeWheel::TimeWheel() {
    assert(SUM_ARRAY(WHEEL_BIT, WHEEL_NUM) < 32);
    for (int i = 0; i < WHEEL_NUM; ++i) {
        //WHEEL_CAP[i] = i > 0 ? (WHEEL_CAP[i-1]*WHEEL_SIZE[i]) : WHEEL_SIZE[i]; //下一级刻度总量 = 上一级刻度总量 * 本级格子数
        WHEEL_CAP[i] = 1 << SUM_ARRAY(WHEEL_BIT, i+1);
		uint32 size = 1 << WHEEL_BIT[i];
		WHEEL_MASK[i] = size - 1;
        _wheels[i] = new stWheel(size);
    }
}
TimeWheel::~TimeWheel() {
    for (int i = 0; i < WHEEL_NUM; ++i) {
        if (_wheels[i]) delete _wheels[i];
    }
}
void TimeNode::_Callback() {
	_funcIng = true;
	int n = func();
	_funcIng = false;
	if (n > 0) {
		when += n;
		TimeWheel::Instance()._AddTimerNode(n, this);
	} else {
		delete this; //Bug：回调中TimerMgr::DelTimer()，就多次delete了
	}
}
inline void TimeNode::Stop() { sTimerMgr.DelTimer(this); }

TimeNode* TimeWheel::AddTimer(const std::function<int()>& f, float delaySec) {
    uint32 delay = uint32(delaySec * 1000);
    TimeNode* node = new TimeNode(f);
    node->when = GameApi::TimeMS() + delay;
    _AddTimerNode(delay, node);
    return node;
}
void TimeWheel::_AddTimerNode(uint32 milseconds, TimeNode* _node) {
    NodeLink* slot = NULL;
    uint32 tickCnt = milseconds / TIME_TICK_LEN;
    if (tickCnt < WHEEL_CAP[0]) {
        uint32 index = (_wheels[0]->slotIdx + tickCnt) & WHEEL_MASK[0]; //2的N次幂位操作取余
        slot = _wheels[0]->slots + index;
        //LOG_TRACK("wheel[%u], slot[%u], curSlot[%u], node[%p], msec[%u]", 0, index, _wheels[0]->slotIdx, node, milseconds);
    } else {
        for (int i = 1; i < WHEEL_NUM; ++i) {
            if (tickCnt < WHEEL_CAP[i]) {
                uint32 preCap = WHEEL_CAP[i - 1]; //上一级总容量即为本级的一格容量
                uint32 index = (_wheels[i]->slotIdx + tickCnt / preCap - 1) & WHEEL_MASK[i]; //勿忘-1
                slot = _wheels[i]->slots + index;
                //LOG_TRACK("wheel[%u], slot[%u], curSlot[%u], node[%p], msec[%u]", i, index, _wheels[i]->slotIdx, node, milseconds);
                break;
            }
        }
    }
    NodeLink* node = &(_node->link);
    node->prev = slot->prev; //插入格子的prev位置(尾节点)
    node->prev->next = node;
    node->next = slot;
    slot->prev = node;
}
void TimeWheel::DelTimer(TimeNode* _node) {
	if (_node->_funcIng) return; //Bug：避免与TimerNode::_Callback()中重复delete
    NodeLink& node = _node->link;
    node.prev->next = node.next;
    node.next->prev = node.prev;
    node.prev = node.next = &node; //circle
    delete _node;
}
void TimeWheel::Refresh(uint32 elapse, const time_t timenow) {
    _time_elapse += elapse;
    uint32 tickCnt = _time_elapse / TIME_TICK_LEN;
    _time_elapse %= TIME_TICK_LEN;
    for (uint32 i = 0; i < tickCnt; ++i) { //扫过的slot均超时
        bool isCascade = false;
        stWheel* wheel = _wheels[0];
        NodeLink* slot = wheel->GetCurSlot();
        if (++(wheel->slotIdx) >= wheel->size) {
            wheel->slotIdx = 0;
            isCascade = true;
        }
        NodeLink* node = slot->next;
        slot->next = slot->prev = slot; //清空当前格子
        while (node != slot) {          //环形链表遍历
            TimeNode* tmp = (TimeNode*)node;
            node = node->next; //得放在前面，后续函数调用，可能会更改node的链接关系
            AddToReadyNode(tmp);
        }
        if (isCascade) Cascade(1, timenow); //跳级
    }
    DoTimeOutCallBack();
}
void TimeWheel::AddToReadyNode(TimeNode* _node) {
    NodeLink* node = &(_node->link);
    node->prev = _readyNode.prev;
    node->prev->next = node;
    node->next = &_readyNode;
    _readyNode.prev = node;
}
void TimeWheel::DoTimeOutCallBack() {
    NodeLink* node = _readyNode.next;
    while (node != &_readyNode) {
        TimeNode* tmp = (TimeNode*)node;
        node = node->next;
        tmp->_Callback();
    }
    _readyNode.next = _readyNode.prev = &_readyNode;
}
void TimeWheel::Cascade(uint32 wheelIdx, const time_t timenow) {
    if (wheelIdx < 1 || wheelIdx >= WHEEL_NUM) return;

    bool isCascade = false;
    stWheel* wheel = _wheels[wheelIdx];
    NodeLink* slot = wheel->GetCurSlot();
    //【Bug】须先更新槽位————扫格子时加新Node，不能再放入当前槽位了
    if (++(wheel->slotIdx) >= wheel->size) {
        wheel->slotIdx = 0;
        isCascade = true;
    }
    NodeLink* node = slot->next;
    slot->next = slot->prev = slot; //清空当前格子
    while (node != slot) {
        TimeNode* tmp = (TimeNode*)node;
        node = node->next;
        if (tmp->when <= timenow) {
            AddToReadyNode(tmp);
        } else {
            //LOG_TRACK("wheel[%u], curSlot[%u], node[%p], msec[%u]", wheelIdx, wheel->slotIdx, node, node->timeDead - timenow);
            //【Bug】加新Node，须加到其它槽位，本槽位已扫过(失效，等一整轮才会再扫到)
            _AddTimerNode(uint32(tmp->when - timenow), tmp);
        }
    }
    if (isCascade) Cascade(++wheelIdx, timenow);
}
void TimeWheel::Printf() {
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
                TimeNode* node = (TimeNode*)link;
                link = link->next;
                LOG_TRACK("node[%p], next[%p], prev[%p] timeDead[%lld]", node, link->next, link->prev, node->when);
            }
        }
    }
}
