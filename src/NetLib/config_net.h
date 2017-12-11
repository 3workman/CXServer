#pragma once

#pragma pack(push,1)
struct NetMeta
{
    std::string module;
    int         svr_id;
    std::string svr_name;
    std::string version;
    std::string ip;
    std::string out_ip;
    uint16      tcp_port;
    uint16      http_port;
    int         max_conn;
    std::string connect;
    //需动态同步的数据
    bool is_closed = false;

    static const char* GetFormat() {
        return "sisssshhis";
    }
    static std::vector<NetMeta> _table;
    static const NetMeta* G_Local_Meta;
    static const NetMeta* GetMeta(std::string module, int svrId = -1);
    static void AddMeta(const NetMeta& meta);
    static void DelMeta(std::string module, int svrId);

    void DataToBuf(class NetPack& buf) const;
    void BufToData(class NetPack& buf);
};
#pragma pack(pop)

struct NetCfgServer
{
    static NetCfgServer& Instance() { static NetCfgServer T; return T; }

    const char* ip = NetMeta::G_Local_Meta->ip.c_str();
    int    svrId = NetMeta::G_Local_Meta->svr_id;
    uint16 wPort = NetMeta::G_Local_Meta->tcp_port;
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
    const char* svrIp;
    uint16 svrPort;
    uint32 nMaxPackageSend = 1024 * 20;
    const char* kPassword = "ChillyRoom";
};