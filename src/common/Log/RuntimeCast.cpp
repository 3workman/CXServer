#include "stdafx.h"
#include "RuntimeCast.h"
#include "LogFile.h"

bool RuntimeCast::IsLog = true;

RuntimeCast::RuntimeCast(const char* funName, uint cast)
{
    assert(funName);

    if (false == IsLog) return;

    m_cast = cast;
    m_funName = funName;
    QueryPerformanceCounter(&m_begin);
}
RuntimeCast::~RuntimeCast()
{
    if (false == IsLog) return;

    LARGE_INTEGER end;
    QueryPerformanceCounter(&end);

    static LARGE_INTEGER freq;
    if (freq.QuadPart == 0) QueryPerformanceFrequency(&freq);

    uint ms = uint((end.QuadPart - m_begin.QuadPart)*1000 / freq.QuadPart);

    if (ms >= m_cast)
    {
        LOG_WARN("RuntimeCast: %06d	%s", ms, m_funName);
    }
}