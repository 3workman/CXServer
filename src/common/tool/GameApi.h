#pragma once

namespace GameApi {
    // 把str拆分成多个字符串;
    void SplitStr(const std::string& str, std::vector<std::string>& retVec, const char split = '|');
    // 把str拆分成多个int
    void SplitStr(const std::string& str, std::vector<int>& retVec, const char split = ',');

    // "(2|3)(7|4)(9|6)"，分割为数值对
    void SplitStr(const std::string& str, IntPairVec& retVec);
    // "2|3,7|4,9|6"，分割为数值对
    void SplitStr2(const std::string& str, IntPairVec& retVec);

    //每帧一次Refresh，业务取缓存数据，避免多次调用底层api
    void RefreshTimeSecond();
    time_t TimeSecond();
    time_t TimeMS(); //uint timenow = GetTickCount(); uint32的毫秒计数，最多到49.7天，系统长期运行后，计时器归0，许多逻辑就错乱了

    /*
        【溢出Bug】uint timenow = GetTickCount();
        uint32的毫秒计数，最多到49.7天，系统长期运行后，计时器归0，许多逻辑就错乱了
    */
    std::chrono::milliseconds TimeNow();

    int TimeHour();
    int TimeMonth();
    int TimeYear();
    int TimeDayOfWeek();
    int TimeYearOfWeek();
    struct tm* TimeDate();

    bool IsToday(time_t sec);
    bool IsSameDay(time_t sec1, time_t sec2);

    time_t ParseTime(time_t num); //20160223145632：2016年2月23号14:56:32
}
