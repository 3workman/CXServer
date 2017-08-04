#pragma once
//#include <atlstr.h>

namespace GameApi {
    // 把str拆分成多个字符串;
    void SplitStr(const std::string& str, std::vector<std::string>& retVec, const char split = '|');
    // 把str拆分成多个int
    void SplitStr(const std::string& str, std::vector<int>& retVec, const char split = ',');

    // "(2|3)(7|4)(9|6)"，分割为数值对
    void SplitStr(const std::string& str, IntPairVec& retVec);
    // "2|3,7|4,9|6"，分割为数值对
    void SplitStr2(const std::string& str, IntPairVec& retVec);

    //每帧一次Refresh，业务去缓存数据，避免多次调用底层api
    time_t TimeMS();
    time_t TimeNow(); //second
    void RefreshTimeNow();

    int TimeHour();
    int TimeMonth();
    int TimeYear();
    int TimeDayOfWeek();
    int TimeYearOfWeek();

    bool IsToday(time_t sec);
    bool IsSameDay(time_t sec1, time_t sec2);

    time_t ParseTime(time_t num); //20160223145632：2016年2月23号14:56:32
}