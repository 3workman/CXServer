#include "stdafx.h"
#include "RuntimeCast.h"
#include "LogFile.h"

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/time.h>
#endif

bool RuntimeCast::IsLog = true;

RuntimeCast::RuntimeCast(const char* funName, uint cast)
{
    assert(funName);

    if (false == IsLog) return;

    m_cast = cast;
    m_funName = funName;

#ifdef _WIN32
    QueryPerformanceCounter(&m_begin);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    m_begin = tv.tv_sec * 1000 * 1000 + tv.tv_usec;
#endif
}
RuntimeCast::~RuntimeCast()
{
    if (false == IsLog) return;

#ifdef _WIN32
    LARGE_INTEGER end;
    QueryPerformanceCounter(&end);

    static LARGE_INTEGER freq;
    if (freq.QuadPart == 0) QueryPerformanceFrequency(&freq);

    uint ms = uint((end.QuadPart - m_begin.QuadPart)*1000 / freq.QuadPart);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64 end = tv.tv_sec * 1000 * 1000 + tv.tv_usec;
    uint ms = uint(end - m_begin);
#endif
    if (ms >= m_cast) LOG_WARN("RuntimeCast: %06d	%s", ms, m_funName);
}