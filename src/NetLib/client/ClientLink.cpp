#include "stdafx.h"
#include "ClientLink.h"

#pragma comment (lib,"Ws2_32.lib")

const DWORD IN_BUFFER_SIZE = 1024 * 4;
const DWORD MAX_MESSAGE_LENGTH = 1024 * 1024;

void CALLBACK ClientLink::DoneIO(DWORD dwErrorCode,
	DWORD dwNumberOfBytesTransferred,
	LPOVERLAPPED lpOverlapped)
{
	if (lpOverlapped == NULL)
	{
		printf("DoneIO Null code:%x - bytes:%d \n", dwErrorCode, dwNumberOfBytesTransferred);
		return;
	}
	My_OVERLAPPED* ov = (My_OVERLAPPED*)lpOverlapped;
	ClientLink* client = ov->client;

	if (0 != dwErrorCode && dwErrorCode != ERROR_HANDLE_EOF)
	{
		printf("DoneIO Err code:%x - bytes:%d \n", dwErrorCode, dwNumberOfBytesTransferred);
		client->CloseClient(dwErrorCode);
		return;
	}
	client->DoneIOCallback(dwNumberOfBytesTransferred, ov->eType);
}

bool ClientLink::InitWinsock()
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
bool ClientLink::CleanWinsock()
{
	int nError = WSACleanup();
	return nError == 0;
}

ClientLink::ClientLink(const ClientLinkConfig& info) 
	: _config(info)
	, _recvBuf(IN_BUFFER_SIZE * 2)
	, _sendBuf(IN_BUFFER_SIZE)
{
	_ovRecv.eType = IO_Read;
	_ovSend.eType = IO_Write;
}

void ClientLink::DoneIOCallback(DWORD dwNumberOfBytesTransferred, EnumIO type)
{
	if (type == IO_Write)
	{
		if (_eState == State_Connecting)
		{
			cLock lock(_csWrite);
			OnConnect();
		}
		if (_eState == State_Connected)
		{
			if (dwNumberOfBytesTransferred)
			{
				OnSend_DoneIO(dwNumberOfBytesTransferred);
			}
		}
	}
	else if (type == IO_Read)
	{
		if (_eState == State_Connected)
		{
			if (dwNumberOfBytesTransferred)
			{
				OnRead_DoneIO(dwNumberOfBytesTransferred);

				PostRecv(_recvBuf.beginWrite(), _recvBuf.writableBytes());
			}
			else
				CloseClient(0); //收到0包，对端关闭了
		}
	}
}
bool ClientLink::CreateLinkAndConnect(HandleMsgFunc handleMsg)
{
    _HandleServerMsg = handleMsg;

	if (_sClient != INVALID_SOCKET)
	{
		printf("Error_CreateLink：socket being used. \n");
		return false;
	}

	_ovSend.SetLink(this);
	_ovRecv.SetLink(this);

	_eState = State_Close;
	_bCanWrite = true;

	_sClient = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (_sClient == INVALID_SOCKET)
	{
		return false;
	}

	//关闭Nagle算法，无论多小的包都直接发
	//如果不关闭，client的网络延时会不会出现震荡？一次只准发一个小包，其它的会堆积等待
	bool noDelay = true;
	if (setsockopt(_sClient, IPPROTO_TCP, TCP_NODELAY, (char*)&noDelay, sizeof(noDelay)) == SOCKET_ERROR)
	{
		printf("setsockopt() failed with TCP_NODELAY");
		return false;
	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(0);
	::bind(_sClient, (const sockaddr*)&addr, sizeof(addr));

	if (BindIoCompletionCallback((HANDLE)_sClient, DoneIO, 0)){
		_eState = State_Connecting;
		if (!ConnectEx())
		{
			DWORD dwError = GetLastError();
			if (WSAEWOULDBLOCK != dwError && ERROR_IO_PENDING != dwError)
			{
				printf("NetError_Connect:%x \n", dwError);
				closesocket(_sClient);
				_eState = State_Close;
				return false;
			}
		}
	}
	return true;
}
BOOL ClientLink::ConnectEx()
{
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(_config.strIP.c_str());
	addr.sin_port = htons(_config.wServerPort);

	LPFN_CONNECTEX pfnConnectEx;
	GUID GuidConnectEx = WSAID_CONNECTEX;
	DWORD dwBytes(0);
	int nRet = WSAIoctl(_sClient, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx),
		&pfnConnectEx, sizeof(pfnConnectEx), &dwBytes, NULL, NULL);
	if (nRet || !pfnConnectEx)
	{
		return false;
	}
	return pfnConnectEx(_sClient, (const sockaddr*)&addr, sizeof(SOCKADDR_IN), NULL, 0, NULL, &_ovSend);
}
void ClientLink::OnConnect()
{
	_eState = State_Connected;
	printf("connect to server success\n");

	cLock lock(_csRead);

	WSABUF buf;
	buf.buf = _recvBuf.beginWrite();
	buf.len = _recvBuf.writableBytes();
	DWORD dwBytes(0), dwFlags(0);
	// event that will trigger after data is receieved
	if (WSARecv(_sClient, &buf, 1, &dwBytes, &dwFlags, &_ovRecv, 0) == SOCKET_ERROR)
	{
		int nLastError = WSAGetLastError();
		if (nLastError != WSA_IO_PENDING)
		{
			printf("WSARecv Error:%x \n", nLastError);
		}
	}
}
void ClientLink::CloseClient(int nErrorCode)
{
	printf("CloseClient ErrCode:%d \n", nErrorCode);
	_eState = State_Close;
	shutdown(_sClient, SD_BOTH);
	closesocket(_sClient); // 客户端的关闭好暴力~
}

void ClientLink::OnSend_DoneIO(DWORD dwBytesTransferred)
{
	cLock lock(_csWrite);

	_bCanWrite = true;

	_sendBuf.readerMove(dwBytesTransferred);

	if (DWORD nLeft = _sendBuf.readableBytes())
	{
		PostSend(_sendBuf.beginRead(), nLeft);
	}
}
bool ClientLink::PostSend(char* buffer, DWORD nLen)
{
	if (_eState != State_Connected) return false;

	//Notice：长度过大塞满socket缓冲区，甚至[TCP Zerowindow]
	WSABUF buf;
	buf.buf = buffer;
	buf.len = nLen;
	DWORD dwBytes(0);
	int ret = WSASend(_sClient, &buf, 1, &dwBytes, 0, &_ovSend, 0);

	_bCanWrite = false;

	if (SOCKET_ERROR == ret)
	{
		int nLastError = WSAGetLastError();
		if (nLastError != WSA_IO_PENDING)
		{
			printf("WSASend Error %x", nLastError);
			return false;
		}
	}
	return true;
}
bool ClientLink::PostRecv(char* buffer, DWORD nLen)
{
	if (_eState != State_Connected) return false;

	//cLock lock(_csRead); //接收不用锁；调PostSend()前lock，是为了避免IO回调、主线程投递的冲突

	//Notice：buf长度太短的性能损失（_recvBuf开的足够大8k）
	WSABUF buf;
	buf.buf = buffer;
	buf.len = nLen;
	DWORD dwBytes(0), dwFlags(0);
	if (WSARecv(_sClient, &buf, 1, &dwBytes, &dwFlags, &_ovRecv, 0) == SOCKET_ERROR)
	{
		int nLastError = WSAGetLastError();
		if (nLastError != WSA_IO_PENDING)
		{
			printf("WSARecv Error %x", nLastError);
			return false;
		}
	}
	return true;
}
void ClientLink::OnRead_DoneIO(DWORD dwBytesTransferred)
{
	_recvBuf.writerMove(dwBytesTransferred); // IO完成回调，接收字节递增
	const DWORD c_off = sizeof(DWORD);
	char* pPack = _recvBuf.beginRead();
	while (_recvBuf.readableBytes() >= c_off)
	{
		const DWORD c_msgSize = *((DWORD*)pPack);	// 【网络包：头4字节为消息体大小】
		const DWORD c_packSize = c_msgSize + c_off;	// 【网络包长 = 消息体大小 + 头长度】
		char* pMsg = pPack + c_off;                 // 【后移4字节得：消息体指针】

		if (c_packSize > _recvBuf.readableBytes()) break;         // 【包未收完：接收字节 < 包大小】

		// 2、消息解码、处理 decode, unpack and ungroup
		RecvMsg(pMsg, c_msgSize);

		// 3、消息处理完毕，接收字节/包指针更新(处理下一个包)
		_recvBuf.readerMove(c_packSize);
		pPack += c_packSize;
	}
}
void ClientLink::RecvMsg(char* pMsg, DWORD size)
{
    _HandleServerMsg(pMsg, size);
}
void ClientLink::SendMsg(void* pMsg, DWORD size)
{
	cLock lock(_csWrite);

	_sendBuf.append(size);
    _sendBuf.append(pMsg, size);

	//客户端无需像服务器似的，消息积累超长才投递，见ServLink::SendMsg

	if (_bCanWrite && size > 0)
	{
		PostSend(_sendBuf.beginRead(), _sendBuf.readableBytes());
	}
}