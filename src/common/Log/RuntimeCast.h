/***********************************************************************
* @ 检查函数执行耗时
* @ brief
    1、利用对象构造析构，检查中途函数的执行耗时

    2、超过既定值的，Log，方便线上性能排查

* @ author zhoumf
* @ date 2017-8-7
************************************************************************/
#pragma once

class RuntimeCast
{
public:
    RuntimeCast(const char* funName, uint cast = 15);
    ~RuntimeCast();

    static bool     IsLog;
private:
    LARGE_INTEGER	m_begin;
    uint            m_cast;
    const char*		m_funName;
};