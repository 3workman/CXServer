#pragma once

struct NetCfgServer
{
    const char* ip = "127.0.0.1";
    uint16 wPort = 7030;
    uint32 nRecvPacketCheckTime = 10;
    uint32 nRecvPacketLimit = 1200;
    uint32 dwAssistLoopMs = 10;
    uint32 nMaxPackage = 1024;
    int    nDeadTime = 300;      //������û�յ�client��Ϣ���Ͽ�
    uint32 nTimeLoop = 10;		//���̵߳�����£�����ʱ��������е�socket,�����Send_Groupһ��ʹ��
    uint32 nInBuffer = 2048;
    uint32 nPackSize = 512;
    uint32 DecodeWaitTime = 1000;	//connect��ɵ�decode�����ʱ��(�������ʱ�仹û��decode ����ߵ�)  ms��
    uint32 dwMaxLink = 100/*20000*/;
    int   nPreLink = 1;			//Ԥ�ȴ�����Link
    int	  nPreAccept = 1;		//Ԥ��Ͷ�ݵ�AcceptEx
    const char* kPassword = "ChillyRoom";
};

struct NetCfgClient
{
    const char* svrIp = "127.0.0.1";
    uint16 svrPort = 7003; //svr_cross
    uint32 nMaxPackageSend = 1024 * 20;
    const char* kPassword = "ChillyRoom";
};