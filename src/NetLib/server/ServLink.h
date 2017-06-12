/***********************************************************************
* @ IOCP注意事项
* @ brief
	1、WSASend
		·一次仅投递一个SendIO，多次投递下，若某个DoneIO只操作了部分数据，tcp流就错乱了
		·另外，同时投递多个SendIO，实际的IO操作可能不是有序的
		·PostSend共有三处调用(不同线程)要加锁：逻辑线程SendMsg、IO线程补发、辅助线程定期投递
		·发送长度过大塞满socket缓冲区，甚至tcp滑动窗口[TCP Zerowindow]

		·为提高吞吐量，可以多次投递SendIO，只要解决两个问题：
			(1)socket的发送缓冲不能只一个了，因为数据可能只发出部分，并行send，流数据可能错乱
			(2)IO工作线程调度不可控，消息可能失序，自己得加序号，对端收到后据序号重排，再交给业务层
		·游戏这么做的收益不大，性能提升有限，代码复杂度剧增；外网运行情况看，顺序式的PostSendIO表现很好了
	2、对端只Connect不发数据，DoneIO不会被回调
	3、DoneIO的dwNumberOfBytesTransferred为空
		·socket关闭的回调，此值为0
		·recvBuf太小，tcp滑动窗口被塞满，出现[TCP Zerowindow]时也会为0
		·dwErrorCode有效时，dwNumberOfBytesTransferred是否总为0？待验证
	4、WSAENOBUFS错误
		·每当我们重叠提交一个send或receive操作的时候，其中指定的发送或接收缓冲区就被锁定了
		·当内存缓冲区被锁定后，将不能从物理内存进行分页
		·操作系统有一个锁定最大数的限制，一旦超过这个锁定的限制，就会产生WSAENOBUFS错误
	5、WSARecv
		·项目代码看，WSARecv也是每次只投递一个的，完成回调才PostRecv下一次
		·接收缓冲太小的性能损失
		·特殊用法：Recv时可以先Recv一个长度为0的buf，数据来到时会回调你，再去Recv真正的长度
		·因为当你提交操作没有缓冲区时，那么也不会存在内存被锁定了
		·使用这种办法后，当你的receive操作事件完成返回时，该socket底层缓冲区的数据会原封不动的还在其中而没有被读取到receive操作的缓冲区来
		·此时，服务器可以简单的调用非阻塞式的recv将socket缓冲区中的数据全部读出来，一直到recv返回 WSAEWOULDBLOCK 为止
		·这种设计非常适合那些可以牺牲数据吞吐量而换取巨大并发连接数的服务器
		·用“非阻塞的recv”读socket时，若预计服务器会有爆发数据流，可以考虑投递一个或多个receive来取代“非阻塞的recv”
	6、AcceptEx
		·在投递AcceptEx的时候，我们还能够顺便在同一时间，收取client发来的第一组数据
		·这也意味着，client仅仅连入但不发送数据的，我们就不会收到这个AcceptEx的完毕回调

	【优化】
		·目前的关闭方式：Invalid()里shutdown(SD_RECEIVE)，等待三分钟后才强制closesocket（等的时间太长了~囧）
		·先设无效标记，检查WSASend，无ERROR_IO_PENDING时调shutdown(SD_SEND)
		·tcp缓冲发完后FIN，客户端收到后回FIN（四次握手关闭连接），ServLink收到0包，进而closesocket（DoneIOCallback中IO_Read无效时仍要能收）
		·客户端断电就收不到回的FIN了，所以得shutdown列表，5秒还不关就强关
* @ author zhoumf
* @ date 2016-7-15
************************************************************************/
#pragma once
//////////////////////////////////////////////////////////////////////////
// 使用winsock2  避免同winsock冲突
//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN
//#endif
//#include <windows.h>
#include <winsock2.h>
//////////////////////////////////////////////////////////////////////////
#include <Mswsock.h> // AcceptEx
#include "Buffer/buffer.h"
#include "tool/cLock.h"
#include "define.h"
#include <time.h>

class ServLink;
class ServLinkMgr;
struct ServerConfig;

enum EnumIO{ IO_Write, IO_Read };

struct My_OVERLAPPED : public OVERLAPPED
{
	ServLink* client;
	EnumIO	  eType;

	void SetLink(ServLink* p)
	{
		memset(this, 0, sizeof(OVERLAPPED));
		client = p;
	}
};

class ServLink{
	enum EStatus{ STATE_DEAD, STATE_ACCEPTING, STATE_CONNECTED };
public:
    static void CALLBACK DoneIO(DWORD, DWORD, LPOVERLAPPED);

    ServLink(ServLinkMgr* p);
    //~ServLink(){};

	static uint16 s_nID;

	EStatus _eState = STATE_DEAD;
	void OnConnect();
	bool CloseLink();
	bool CreateLinkAndAccept();
	void UpdateAcceptAddr();

	void DoneIOCallback(DWORD dwNumberOfBytesTransferred, EnumIO type);

	bool _bCanWrite;            // 一次::WSASend()完毕，才能做下一次
	void OnSend_DoneIO(DWORD dwNumberOfBytesTransferred);
	void ServerRun_SendIO();	// 外部线程调用
	bool PostSend(char* buffer, DWORD size);	// 投递一个发送IO(写)请求，Add a packet to output buffer
	void OnRead_DoneIO(DWORD size);		        // Retrieve a packet from input buffer
	bool PostRecv();		                    // 投递一个接收IO(读)请求，Do actual receive
    int RecvMsg(char* pMsg, DWORD size);

	void Maintain(time_t timenow);

	bool SendMsg(const void* pMsg, uint16 msgSize);

	LPCSTR GetIP(){ return _szIP; }
	int GetID(){ return _nLinkID; }

	bool IsSocket(){ return _sClient != INVALID_SOCKET; }
	bool IsConnected(){ return _eState == STATE_CONNECTED; }
	void Invalid(InvalidMessageEnum eReason){
		InterlockedExchange(&_bInvalid, 1); //多线程bug
		time(&_timeInvalid);
		if (_sClient != INVALID_SOCKET){
			shutdown(_sClient, SD_RECEIVE);
			printf("shutdown socket IP:%s - ID:%d - EnumReason:%d\n", _szIP, _nLinkID, eReason);
		}
	}

	InvalidMessageEnum _eLastError = Message_NoError;
	void OnInvalidMessage(InvalidMessageEnum e, int nErrorCode, bool bToClient, int nParam = 0);
    void HandleClientMessage(void* pMsg, int size);

	void Err(LPCSTR sz){
		printf("%s:%d - ID:%d\n", sz, WSAGetLastError(), _nLinkID);
	}
	void Err(LPCSTR sz, DWORD err){
		printf("%s:%d - ID:%d\n", sz, err, _nLinkID);
	}


	DWORD _recvPacket;		//收到多少条消息
	time_t _recvPacketTime;	//从哪个时间点开始计算的
	time_t _recvIOTime;
	__forceinline void RecvIOTimeStart(time_t timenow)
	{
		_recvPacket = 0;
		_recvPacketTime = timenow;
		_recvIOTime = timenow;
	}
	__forceinline void LastRecvIOTime(time_t timenow){ _recvIOTime = timenow; }
	__forceinline int RecvIOElapsed(time_t timenow){ return (int)(timenow - _recvIOTime); }

	//心跳验证
	DWORD m_dwLastHeart;
	void OnHeartMsg(){ m_dwLastHeart = GetTickCount(); }
	void CheckHeart()
	{
		DWORD now = GetTickCount();
		if (now - m_dwLastHeart > 60000)
		{
			OnInvalidMessage(Net_HeartKick, 0, true);
		}
		m_dwLastHeart = now;
	}

private:
    void*     _player = NULL; // 收到玩家第一条消息(登录)，从角色内存池(有上限5000)中取个，与ServLink绑定
    const int _nLinkID; // ServerLink自己的ID，可以很大

	net::Buffer _recvBuf;
	net::Buffer _sendBuf;

	cMutex _csLock;

	My_OVERLAPPED _ovRecv;	// Used in WSARead() calls
	My_OVERLAPPED _ovSend;	// Used in WSASend() calls

	char _szIP[MAX_IP];
	sockaddr_in _local;
	sockaddr_in _peer;
	SOCKET _sClient = INVALID_SOCKET;
	WSAEVENT _hEventClose;

	ServLinkMgr* const _pMgr;
	const ServerConfig& Config();

	LONG _bInvalid		= false;
	time_t _timeInvalid = 0;
};