#include "stdafx.h"
#include "ServLinkMgr.h"
#include "define.h"
#include "ServLink.h"
#include "..\..\common\tool\thread.h"

#pragma comment (lib,"Ws2_32.lib")

#define CHECK_INTERVAL		1000

ServLinkMgr::ServLinkMgr(const ServerConfig& info) : _config(info)
{
	_pThread = NULL;
	_vecLink.reserve(info.dwMaxLink);
}
ServLinkMgr::~ServLinkMgr()
{
	delete _pThread;
}

bool ServLinkMgr::InitWinsock()
{
	WSADATA wsaData = { 0 };
	int nError = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nError != 0) return false;

	if (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		return false;
	}
	return true;
}
bool ServLinkMgr::CleanWinsock()
{
	int nError = WSACleanup();
	return nError == 0;
}
bool ServLinkMgr::CreateServer(BindLinkFunc bindPlayer, HandleMsgFunc handleClientMsg, ReportErrorFunc reportErrorMsg)
{
    _BindLinkAndPlayer = bindPlayer;
    _HandleClientMsg = handleClientMsg;
    _ReportErrorMsg = reportErrorMsg;

	_sListener = socket(AF_INET, SOCK_STREAM, 0);
	if (_sListener == INVALID_SOCKET)
	{
		printf("创建socket错误，请检查socket是否被初始化！");
		return false;
	}

	//FIXME：ServLink用的SO_LINGER强制关闭，没有TIME_WAIT，不必SO_REUSEADDR吧~
	bool reuseAddr = true;
	if (setsockopt(_sListener, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseAddr, sizeof(reuseAddr)) == SOCKET_ERROR)
	{
		printf("setsockopt() failed with SO_REUSEADDR");
		return false;
	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(_config.strIP.c_str());
	addr.sin_port = htons(_config.wPort);

	if (0 != ::bind(_sListener, (SOCKADDR *)&addr, sizeof(addr)))
	{
		printf("bind错误！");
		return false;
	}
	if (0 != listen(_sListener, 5)) // 三次握手完成，呼入连接请求队列的最大长度
	{
		printf("listen错误，请检查端口是否已经被使用！");
		return false;
	}
    if (0 == BindIoCompletionCallback((HANDLE)_sListener, ServLink::DoneIO, 0))
	{
		printf("BindIoCompletionCallback错误，请检查系统资源是否耗尽！");
		return false;
	}

	// 先投递几个AcceptEx，m_nAccept会在sClient投递完毕后更新
	_nInvalid = _config.dwMaxLink;
	_nAccept = 0;
	_nConnect = 0;
	//memset(_arrLink, 0, sizeof(_arrLink));
	//for (int i = 0; i < _nMaxLink; ++i)
	//{
	//	_arrLink[i] = new ServLink(this, _config.nSendBuffer); //创建所有link，太占内存了，可以优化为一个带上限的池子
	//	if (_nAccept < _config.nPreCreate)
	//	{
	//		if (!_arrLink[i]->CreateLinkAndAccept())
	//		{
	//			printf("创建link错误，请检查参数是否正确！");
	//			return false;
	//		}
	//	}
	//}
	_vecLink.resize(_config.nPreLink); //先创建一批，Maintain里慢慢补
	for (auto& it : _vecLink)
	{
		it = new ServLink(this);
		if (_nAccept < _config.nPreAccept)
		{
			if (!it->CreateLinkAndAccept())  //【里面会创建客户端socket，并AcceptEx(m_hListener, sClient...)】
			{
				printf("创建link错误，请检查参数是否正确！");
				return false;
			}
		}
	}

    AssistThreadLoop();
	return true;
}
bool ServLinkMgr::Close()
{
	if (_pThread)
	{
		_pThread->EndThread();
		delete _pThread;
		_pThread = NULL;
	}
	printf("CloseListen");
	closesocket(_sListener);

	for (auto& it : _vecLink)
	{
		if (it->IsSocket())
		{
			it->CloseLink();
		}
	}
	CleanWinsock();
	Sleep(2000);
	for (auto& it : _vecLink) { delete it; }
	return true;
}


static void AssistLoop(LPVOID pParam)
{
	((ServLinkMgr*)pParam)->_AssistLoop();
}
bool ServLinkMgr::AssistThreadLoop()
{
	_pThread = new Thread;
	return _pThread->RunThread(::AssistLoop, this);
}
bool ServLinkMgr::_AssistLoop()
{
	if (!_pThread) return false;

	time(&_timeNow);
	DWORD dwInitTime = GetTickCount();
	DWORD dwElaspedTime = 0;
	while (WAIT_TIMEOUT == _pThread->WaitKillEvent(_config.dwAssistLoopMs))
	{
		DWORD tempNow = GetTickCount();
		DWORD tempElasped = tempNow - dwInitTime;
		dwInitTime = tempNow;

		time(&_timeNow);

		for (auto& it : _vecLink)
		{
			if (it->IsConnected()) it->ServerRun_SendIO(); //【brief.7】另辟线程定期发送所有buffer
		}

		dwElaspedTime += tempElasped;

		if (dwElaspedTime > CHECK_INTERVAL) { // 多少毫秒检查一次
			dwElaspedTime = 0;
			Maintain(_timeNow); //检查维护serverLink
		}
	}
	return true;
}
void ServLinkMgr::Maintain(time_t timenow)
{
	for (auto& it : _vecLink)
	{
		if (it->IsSocket())
			it->Maintain(timenow);
		else if (_nAccept < _config.nPreAccept)
			it->CreateLinkAndAccept();
	}
	//Notice：还不够，补新的
	while (_nAccept < _config.nPreAccept && _vecLink.size() < _config.dwMaxLink)
	{
		if (ServLink* pLink = new ServLink(this)) //TODO：可以做个对象池优化下
		{
			_vecLink.push_back(pLink);
			pLink->CreateLinkAndAccept();
		}
	}
}

void ServLinkMgr::BroadcastMsg(void* pMsg, DWORD msgSize)
{
	for (auto& it : _vecLink)
	{
		if (it->IsConnected())
		{
            it->SendMsg(pMsg, msgSize);
		}
	}
}