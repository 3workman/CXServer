#include "stdafx.h"
#include "GameApi.h"
#include <time.h>

namespace GameApi {

void SplitStr(const std::string& str, std::vector<std::string>& retVec, const char split/* = '|'*/)
{
    int beginIdx = 0;
    int retIdx = str.find_first_of(split, beginIdx);
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
    int beginIdx = 0;
    int retIdx = str.find_first_of(split, beginIdx);
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
    int beginIdx = 0;
    int retIdx = str.find_first_of('(', beginIdx);
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
    int beginIdx = 0;
    int retIdx = str.find_first_of('|', beginIdx);
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

static time_t g_time_now = 0;
time_t TimeNow() { return g_time_now; }
void RefreshTimeNow() { g_time_now = ::time(NULL); }

int TimeHour() {
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    return p->tm_hour;
}
int TimeMonth() {
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    return p->tm_mon;
}
int TimeYear() {
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    return p->tm_year;
}
int TimeDayOfWeek() {
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    return p->tm_wday;
}
int TimeYearOfWeek() {
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    return p->tm_yday;
}

static time_t _GetCurDay(time_t sec){
    tm* t = ::localtime(&sec);
    time_t ret = t->tm_year;
    ret = ret << 8;
    ret += t->tm_mon;
    ret = ret << 8;
    ret += t->tm_mday;
    return ret;
}
bool IsToday(time_t sec)
{
    return _GetCurDay(sec) == _GetCurDay(g_time_now);
}
bool IsSameDay(time_t sec1, time_t sec2)
{
    return _GetCurDay(sec1) == _GetCurDay(sec2);
}
time_t ParseTime(time_t num) { //20160223145632£º2016Äê2ÔÂ23ºÅ14:56:32
#define _Parse_(n) num%(n); num /= (n)
    struct tm t;
    t.tm_sec = _Parse_(100);
    t.tm_min = _Parse_(100);
    t.tm_hour = _Parse_(100);
    t.tm_mday = _Parse_(100);
    t.tm_mon = _Parse_(100);
    t.tm_year = _Parse_(10000);
    if (t.tm_year < 1900) { t.tm_year = TimeYear(); }
    t.tm_year -= 1900;
    t.tm_isdst = 0;
#undef _Parse_
    return mktime(&t);
}

}