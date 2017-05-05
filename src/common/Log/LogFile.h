/***********************************************************************
* @ �������첽��־
* @ brief
    1������ÿ����־ռһ�У�������grep�������й��߷���
    2��ʱ�����ȷ��΢�롣��gettimeofday(2)����Linux��������ϵͳ���ã����������ں�
    3����ӡ�߳�id�����ڷ������߳�ʱ�򣬼������
    4����־����Դ�ļ������к�

* @ optimize
    1��ʱ����ַ������ֱ𻺴棬һ���ڵ�ֻ�����¸�ʽ��΢�벿��
    2���߳�idԤ�ȸ�ʽ��Ϊ�ַ�����Logʱֻ��򵥿������ֽ�

* @ TODO����Ҫ�ּ�¼ҵ�����ݵ�������־���ܷ���ͳ�ơ���bug�����

* @ author zhoumf
* @ date 2016-11-23
************************************************************************/
#pragma once

class AsyncLog;
class LogFile {
public:
    static LogFile* g_log; //Ϊ���������������߼�����ã��ѵ���������ָ�롣����ϵͳ���վ�̬��������ִ��̫��AsyncLog�����߳̽�����̫�����̽���
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

#define _LOG_MAIN_(obj) LogFile::g_log = &obj /*Notice��Ҫ��main�������Ȼ����δ����Log��������*/
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