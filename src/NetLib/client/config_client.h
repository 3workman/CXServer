#pragma once

struct ClientLinkConfig
{
	std::string strIP = "127.0.0.1";
	WORD  wServerPort = 4567;
	DWORD dwAssistLoopMs = 10;	//每隔多少时间发送一次所有消息！
	DWORD nMaxPackageSend = 1024 * 20;
};