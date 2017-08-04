#pragma once
//#include <atlstr.h>

namespace GameApi {
    // ��str��ֳɶ���ַ���;
    void SplitStr(const std::string& str, std::vector<std::string>& retVec, const char split = '|');
    // ��str��ֳɶ��int
    void SplitStr(const std::string& str, std::vector<int>& retVec, const char split = ',');

    // "(2|3)(7|4)(9|6)"���ָ�Ϊ��ֵ��
    void SplitStr(const std::string& str, IntPairVec& retVec);
    // "2|3,7|4,9|6"���ָ�Ϊ��ֵ��
    void SplitStr2(const std::string& str, IntPairVec& retVec);

    //ÿ֡һ��Refresh��ҵ��ȥ�������ݣ������ε��õײ�api
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

    time_t ParseTime(time_t num); //20160223145632��2016��2��23��14:56:32
}