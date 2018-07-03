/***********************************************************************
* @ 检查函数执行耗时
* @ brief
    1、利用对象构造析构，检查中途函数的执行耗时

    2、超过既定值的，Log，方便线上性能排查

* @ author zhoumf
* @ date 2017-8-7
************************************************************************/
#pragma once
#include <chrono>

class RuntimeCost {
public:
    RuntimeCost(const char* funName, uint costMsec = 15);
    ~RuntimeCost();
private:
    const uint      m_costMsec;
    const char*		m_funName;

    std::chrono::steady_clock::time_point m_begin;
};