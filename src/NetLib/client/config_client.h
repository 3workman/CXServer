#pragma once

struct ClientLinkConfig
{
    std::string strIP = "127.0.0.1";
    uint16 wServerPort = 4567;
    uint32 dwAssistLoopMs = 10;	//每隔多少时间发送一次所有消息！
    uint32 nMaxPackageSend = 1024 * 20;
};