#include "stdafx.h"
#include "GameApi.h"

namespace GameApi {

void SplitStr(const std::string& str, std::vector<std::string>& retVec, const char split/* = '|'*/)
{
    size_t beginIdx = 0;
    size_t retIdx = str.find_first_of(split, beginIdx);
    while (retIdx != -1)
    {
        if (retIdx > beginIdx)
        {
            retVec.push_back(str.substr(beginIdx, retIdx - beginIdx));
        }
        beginIdx = retIdx + 1;
        retIdx = str.find_first_of(split, beginIdx);
    }
    if (beginIdx < (int)str.length())
    {
        retVec.push_back(str.substr(beginIdx, str.length() - beginIdx));
    }
}
void SplitStr(const std::string& str, std::vector<int>& retVec, const char split/*=','*/)
{
    size_t beginIdx = 0;
    size_t retIdx = str.find_first_of(split, beginIdx);
    while (retIdx != -1)
    {
        if (retIdx > beginIdx)
        {
            retVec.push_back(atoi(str.substr(beginIdx, retIdx - beginIdx).c_str()));
        }
        beginIdx = retIdx + 1;
        retIdx = str.find_first_of(split, beginIdx);
    }
    if (beginIdx < (int)str.length())
    {
        retVec.push_back(atoi(str.substr(beginIdx, str.length() - beginIdx).c_str()));
    }
}
void SplitStr(const std::string& str, IntPairVec& retVec) // "(2|3)(7|4)(9|6)"
{
    std::pair<int, int> tmp;
    size_t beginIdx = 0;
    size_t retIdx = str.find_first_of('(', beginIdx);
    while (retIdx != -1)
    {
        beginIdx = retIdx + 1;
        retIdx = str.find_first_of('|', beginIdx);
        tmp.first = atoi(str.substr(beginIdx, retIdx - beginIdx).c_str());

        beginIdx = retIdx + 1;
        retIdx = str.find_first_of(')', beginIdx);
        tmp.second = atoi(str.substr(beginIdx, retIdx - beginIdx).c_str());

        retVec.push_back(tmp);
        retIdx = str.find_first_of('(', beginIdx);
    }
}
void SplitStr2(const std::string& str, IntPairVec& retVec) // "2|3,7|4,9|6"
{
    std::pair<int, int> tmp;
    size_t beginIdx = 0;
    size_t retIdx = str.find_first_of('|', beginIdx);
    while (retIdx != -1)
    {
        tmp.first = atoi(str.substr(beginIdx, retIdx - beginIdx).c_str());

        beginIdx = retIdx + 1;
        retIdx = str.find_first_of(',', beginIdx);
        if (retIdx == -1)
        {
            tmp.second = atoi(str.substr(beginIdx).c_str());
            retVec.push_back(tmp);
            break;
        }
        tmp.second = atoi(str.substr(beginIdx, retIdx - beginIdx).c_str());
        retVec.push_back(tmp);

        beginIdx = retIdx + 1;
        retIdx = str.find_first_of('|', beginIdx);
    }
}

using namespace std::chrono;

static time_t g_time_now = 0;
void RefreshTimeSecond() { g_time_now = system_clock::to_time_t(system_clock::now()); }
time_t TimeSecond() { return g_time_now; }
time_t TimeMS() { return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(); }

std::chrono::milliseconds TimeNow()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}
    
struct tm* TimeDate() { return localtime(&g_time_now); }

static time_t _GetDayMask(time_t sec) {
    struct tm* t = ::localtime(&sec);
    time_t ret = t->tm_year;
    ret = ret << 8;
    ret += t->tm_mon;
    ret = ret << 8;
    ret += t->tm_mday;
    return ret;
}
bool IsToday(time_t sec)                { return _GetDayMask(sec) == _GetDayMask(g_time_now); }
bool IsSameDay(time_t sec1, time_t sec2){ return _GetDayMask(sec1) == _GetDayMask(sec2); }

time_t ParseTime(time_t num) //20160223145632：2016年2月23号14:56:32
{
#define _Parse_(n) num%(n); num /= (n)
    struct tm t;
    t.tm_sec = _Parse_(100);
    t.tm_min = _Parse_(100);
    t.tm_hour = _Parse_(100);
    t.tm_mday = _Parse_(100);
    t.tm_mon = _Parse_(100);
    t.tm_year = _Parse_(10000);
    t.tm_year -= 1900;
    t.tm_isdst = 0;
#undef _Parse_
    return mktime(&t);
}

}
