/***********************************************************************
* @ �������첽��־
* @ brief
    1��ʹ�ü򵥣���¼������Ϣ

    2��TODO����Ҫ�ּ�¼ҵ�����ݵ�������־���ܷ���ͳ�ơ���bug�����

* @ author zhoumf
* @ date 2016-11-23
************************************************************************/
#pragma once

//#include <string>

class AsyncLog;
class LogFile {
public:
    static LogFile* g_log; //Ϊ���������������߼�����ã��ѵ���������ָ�롣����ϵͳ���վ�̬��������ִ��̫��AsyncLog�����߳̽�����̫�����̽���
    enum LogLv {
        OFF = 6,
        ERRO = 5,
        WARING = 4,
        INFO = 3,
        Debug = 2,
        TRACK = 1,
        ALL = 0,
    };
    LogFile(std::string fileName, LogLv lv, bool isPrint);
    ~LogFile();

    void SetLog(LogLv lv, bool isPrint){ _level = lv; _isPrint = isPrint; }
    void Log(const char* curFile, const int curLine, LogLv kLv, const char* fmt, ...);
private:
    FILE*       _fp = NULL;
    AsyncLog*   _async = NULL;
    LogLv       _level = OFF;
    bool        _isPrint = true;

    const char* LevelToString(LogLv kLevel);
};

#define _LOG_MAIN_(obj) LogFile::g_log = &obj /*Notice��Ҫ��main�������Ȼ����δ����Log��������*/
#define LOG_INFO(...)   LogFile::g_log->Log(__FILE__, __LINE__, LogFile::INFO, __VA_ARGS__)
#define LOG_WARN(...)   LogFile::g_log->Log(__FILE__, __LINE__, LogFile::WARING, __VA_ARGS__)
#define LOG_ERROR(...)  LogFile::g_log->Log(__FILE__, __LINE__, LogFile::ERRO, __VA_ARGS__)
#ifdef _DEBUG
#define LOG_TRACK(...)  { \
    char str[128]; \
    sprintf(str, "%s-%s", __FILE__, __FUNCTION__); \
    LogFile::g_log->Log(str, __LINE__, LogFile::TRACK, __VA_ARGS__); \
}
#define LOG_DEBUG(...)  LogFile::g_log->Log(__FILE__, __LINE__, LogFile::Debug, __VA_ARGS__)
#else
#define LOG_TRACK(...)
#define LOG_DEBUG(...)
#endif