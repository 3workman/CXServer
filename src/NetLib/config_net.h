#pragma once

struct NetCfgServer
{
    const char* ip = "127.0.0.1";
    uint16 wPort = 7030;
    uint32 nRecvPacketCheckTime = 10;
    uint32 nRecvPacketLimit = 1200;
    uint32 dwAssistLoopMs = 10;
    uint32 nMaxPackage = 1024;
    int    nDeadTime = 300;      //多少秒没收到client消息，断开
    uint32 nTimeLoop = 10;		//多线程的情况下，多少时间遍历所有的socket,必须跟Send_Group一起使用
    uint32 nInBuffer = 2048;
    uint32 nPackSize = 512;
    uint32 DecodeWaitTime = 1000;	//connect完成到decode的最大时间(超过这个时间还没有decode 则会踢掉)  ms级
    uint32 dwMaxLink = 100/*20000*/;
    int   nPreLink = 1;			//预先创建的Link
    int	  nPreAccept = 1;		//预先投递的AcceptEx
    const char* kPassword = "ChillyRoom";
};

struct NetCfgClient
{
    const char* svrIp = "127.0.0.1";
    uint16 svrPort = 7003; //svr_cross
    uint32 nMaxPackageSend = 1024 * 20;
    const char* kPassword = "ChillyRoom";
};