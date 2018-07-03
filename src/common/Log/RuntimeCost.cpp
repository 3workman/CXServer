#include "stdafx.h"
#include "RuntimeCost.h"
#include "LogFile.h"

static const bool G_IsLog = true;

using namespace std::chrono;

RuntimeCost::RuntimeCost(const char* funName, uint costMsec)
    : m_funName(funName)
    , m_costMsec(costMsec)
{
    assert(funName);
    if (false == G_IsLog) return;

    m_begin = steady_clock::now();
}
RuntimeCost::~RuntimeCost()
{
    if (false == G_IsLog) return;

    uint ms = (uint)duration_cast<milliseconds>(steady_clock::now()-m_begin).count();

    if (ms >= m_costMsec) LOG_WARN("RuntimeCost: %06d	%s", ms, m_funName);
}