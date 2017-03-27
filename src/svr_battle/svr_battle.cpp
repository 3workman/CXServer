#include "stdafx.h"
#include "UdpServer\UdpServer.h"
#include "RakSleep.h"


int _tmain(int argc, _TCHAR* argv[])
{
    sUdpServer.Start();
    while (true) {
        sUdpServer.Update();
        RakSleep(30);
    }
    sUdpServer.Stop();
	return 0;
}