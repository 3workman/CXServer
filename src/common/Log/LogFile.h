/***********************************************************************
* @ 高性能异步日志
* @ brief
    1、尽量每条日志占一行，容易用grep等命令行工具分析
    2、时间戳精确到微秒。用gettimeofday(2)，在Linux上它不是系统调用，不会陷入内核
    3、打印线程id，便于分析多线程时序，检测死锁
    4、日志级别、源文件名、行号

* @ optimize
    1、时间戳字符串，分别缓存，一秒内的只需重新格式化微秒部分
    2、线程id预先格式化为字符串，Log时只需简单拷贝几字节

* @ TODO：还要种记录业务数据的入库表日志，能方便统计、出bug补错等

* @ author zhoumf
* @ date 2016-11-23
************************************************************************/
#pragma once

class AsyncLog;
class LogFile {
public:
    static LogFile* g_log; //为了让析构函数在逻辑层调用，把单例换成了指针。交给系统回收静态对象，析构执行太晚，AsyncLog的子线程结束便太晚，进程僵死
    enum LogLv {
        TRACK,
        DEBUG,
        INFO,
        WARN,
        ERR,
    };
    LogFile(std::string fileName, LogLv lv, bool isPrint);
    ~LogFile();

    void SetLog(LogLv lv, bool isPrint){ _level = lv; _isPrint = isPrint; }
    void Log(const char* curFile, const int curLine, LogLv kLv, const char* fmt, ...);
private:
    FILE*       _fp = NULL;
    AsyncLog*   _async = NULL;
    LogLv       _level = DEBUG;
    bool        _isPrint = true;
};

#define _LOG_MAIN_(obj) LogFile::g_log = &obj /*Notice：要在main函数里，不然程序未结束Log就析构了*/
#define LOG_INFO(...)   LogFile::g_log->Log(__FILE__, __LINE__, LogFile::INFO, __VA_ARGS__)
#define LOG_WARN(...)   LogFile::g_log->Log(__FILE__, __LINE__, LogFile::WARN, __VA_ARGS__)
#define LOG_ERROR(...)  LogFile::g_log->Log(__FILE__, __LINE__, LogFile::ERR, __VA_ARGS__)
#ifdef _DEBUG
#define LOG_TRACK(...)  { \
    char str[128]; \
    sprintf(str, "%s-%s", __FILE__, __FUNCTION__); \
    LogFile::g_log->Log(str, __LINE__, LogFile::TRACK, __VA_ARGS__); \
}
#define LOG_DEBUG(...)  LogFile::g_log->Log(__FILE__, __LINE__, LogFile::DEBUG, __VA_ARGS__)
#else
#define LOG_TRACK(...)
#define LOG_DEBUG(...)
#endif