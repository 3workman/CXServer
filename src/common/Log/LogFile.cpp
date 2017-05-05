#include "stdafx.h"
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include "LogFile.h"
#include "../tool/mkdir.h"
#include "AsyncLog.h"

LogFile* LogFile::g_log = NULL;

static const int MAX_MSFBUFF_SIZE = 1024;
static char g_logBuff[MAX_MSFBUFF_SIZE] = { '\0' };
static const char* LevelToString[] = {
    "TRACK",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
};
STATIC_ASSERT_ARRAY_LENGTH(LevelToString, LogFile::ERR + 1);

void LogFile::Log(const char* curFile, const int curLine, LogLv kLevel, const char* fmt, ...)
{
    if (kLevel < _level) return;

    int idx = sprintf(g_logBuff, "[%s]", LevelToString[kLevel]);
    time_t t; time(&t);
    idx += strftime(g_logBuff+idx, 32, "[%Y-%m-%d %H:%M:%S]", localtime(&t));
    idx += sprintf(g_logBuff+idx, "[%s(%d)]  ", Dir::FindName(curFile), curLine);

    //TODO:[pid=15529][thread=0x12345]  进程id、线程id

    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(g_logBuff+idx, MAX_MSFBUFF_SIZE-idx, fmt, ap);
    va_end(ap);

    idx = (n>0 && n < MAX_MSFBUFF_SIZE-idx-1) ? idx+n : MAX_MSFBUFF_SIZE-2;

    g_logBuff[idx++] = '\n';
    g_logBuff[idx] = '\0';

    _async->Append(g_logBuff, idx);
    //if (_fp) {
    //    fwrite(g_logBuff, sizeof(char), idx, _fp); //Notice：字长不包括结尾'\0'，否则乱码
    //    fflush(_fp);
    //}
    if (_isPrint) printf("%s", g_logBuff);
}
LogFile::LogFile(std::string fileName, LogLv lv, bool isPrint)
{
    char sTime[32];
    time_t t; time(&t);
    strftime(sTime, 32, "%Y%m%d%H%M%S.log", localtime(&t));
    fileName.append(sTime);
    const char* pStr = fileName.c_str();

    Dir::CreatDir(pStr);
    _fp = fopen(pStr, "a");
    if (_fp == NULL) {
        printf("\n LogFile Init Error!!! \n");
        return;
    }

    _async = new AsyncLog(1024, [&](const AsyncLog::BufferVec& vec){
        for (auto& it : vec){
            if (_fp && it->readableBytes() > 0) {
                fwrite(it->beginRead(), sizeof(char), it->readableBytes(), _fp);
                //printf("Async---%s", it->beginRead());
            }
        }
        fflush(_fp);
    });

    SetLog(lv, isPrint);
}
LogFile::~LogFile()
{
    delete _async; // 必须在_fp之前，AsyncLog析构时还有写一次文件
    if (_fp) fclose(_fp);
    g_log = NULL;
}
