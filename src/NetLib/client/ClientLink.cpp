#include "stdafx.h"
#include "ClientLink.h"

#pragma comment(lib,"Ws2_32.lib")

const DWORD IN_BUFFER_SIZE = 1024 * 4;

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
        printf("DoneIO Err code:%x(%d) - bytes:%d \n", dwErrorCode, dwErrorCode, dwNumberOfBytesTransferred);
        client->CloseLink(dwErrorCode);
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
    , _sendBuf(IN_BUFFER_SIZE)
    , _recvBuf(IN_BUFFER_SIZE * 2)
{
	_ovRecv.eType = IO_Read;
	_ovSend.eType = IO_Write;
}

void ClientLink::DoneIOCallback(DWORD dwNumberOfBytesTransferred, EnumIO type)
{
    //Notice���ͻ��˵�dwNumberOfBytesTransferred����Ϊ0��ʲô����ģ�
    //1��Ӧ����ConnectEx�Ļص���������û��������������
    //2��client�ĵ�һ��PostRecv������OnConnect()��
    //3��server�Ǳ��е㲻һ������OnConnect()��״̬���ٽ���OnRead_DoneIO()
	if (type == IO_Write)
	{
		if (_eState == State_Connecting)
		{
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
            if (dwNumberOfBytesTransferred) {
                OnRead_DoneIO(dwNumberOfBytesTransferred);
            } else {
                CloseLink(0); //�յ�0�����Զ˹ر���
            }
		}
	}
}
bool ClientLink::CreateLinkAndConnect(HandleMsgFunc handleMsg)
{
    _HandleServerMsg = handleMsg;

	_ovSend.SetLink(this);
	_ovRecv.SetLink(this);
    _sendBuf.clear();
    _recvBuf.clear();
    _bCanWrite = false;
    _eState = State_Close;

    return Connect();
}
bool ClientLink::Connect()
{
    if (_sClient != INVALID_SOCKET)
    {
        printf("Error_CreateLink��socket being used. \n");
        return false;
    }
    _sClient = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
    if (_sClient == INVALID_SOCKET)
    {
        return false;
    }

    //�ر�Nagle�㷨�����۶�С�İ���ֱ�ӷ�
    //������رգ�client��������ʱ�᲻������𵴣�һ��ֻ׼��һ��С���������Ļ�ѻ��ȴ�
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
        if (!ConnectEx()){
            DWORD dwError = GetLastError();
            if (WSAEWOULDBLOCK != dwError && ERROR_IO_PENDING != dwError)
            {
                printf("NetError_Connect:%x(%d) \n", dwError, dwError);
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
    _eState = State_Connecting;

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
    printf("connect to server success\n");
    _eState = State_Connected;
    _bCanWrite = true;
    PostRecv();
}
void ClientLink::CloseLink(int nErrorCode)
{
    printf("CloseClient ErrCode:%x(%d) \n", nErrorCode, nErrorCode);
	_eState = State_Close;
	shutdown(_sClient, SD_BOTH);
	closesocket(_sClient); // �ͻ��˵Ĺرպñ���~
    _sClient = INVALID_SOCKET;

    // �رպ�����
    while (!Connect()){
        DWORD dwError = GetLastError();
        if (WSAEWOULDBLOCK != dwError && ERROR_IO_PENDING != dwError)
        {
            printf("NetError_Connect:%x(%d) \n", dwError, dwError);
        }
        Sleep(3000);
    }
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

	//Notice�����ȹ�������socket������������[TCP Zerowindow]
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
bool ClientLink::PostRecv()
{
	if (_eState != State_Connected) return false;

	//cLock lock(_csRead); //���ղ���������PostSend()ǰlock����Ϊ�˱���IO�ص������߳�Ͷ�ݵĳ�ͻ

	//Notice��buf����̫�̵�������ʧ��_recvBuf�����㹻��8k��
	WSABUF buf;
    buf.buf = _recvBuf.beginWrite();
    buf.len = _recvBuf.writableBytes();
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
	_recvBuf.writerMove(dwBytesTransferred); // IO��ɻص��������ֽڵ���
    const DWORD c_off = sizeof(uint16);
	char* pPack = _recvBuf.beginRead();
	while (_recvBuf.readableBytes() >= c_off)
	{
        const DWORD kMsgSize = *((uint16*)pPack);	// ���������ͷ2�ֽ�Ϊ��Ϣ���С��
		const DWORD kPackSize = kMsgSize + c_off;	// ��������� = ��Ϣ���С + ͷ���ȡ�
		char* pMsg = pPack + c_off;                 // ������2�ֽڵã���Ϣ��ָ�롿

		if (kPackSize > _recvBuf.readableBytes()) break; // ����δ���꣺�����ֽ� < ����С��

		// 2����Ϣ���롢���� decode, unpack and ungroup
		RecvMsg(pMsg, kMsgSize);

		// 3����Ϣ������ϣ������ֽ�/��ָ�����(������һ����)
		_recvBuf.readerMove(kPackSize);
		pPack += kPackSize;
	}
    PostRecv();
}
void ClientLink::RecvMsg(char* pMsg, DWORD size)
{
    _HandleServerMsg(pMsg, size);
}
void ClientLink::SendMsg(const void* pMsg, uint16 size)
{
	cLock lock(_csWrite);

    _sendBuf.append(size);
    _sendBuf.append(pMsg, size);

	//�ͻ���������������Ƶģ���Ϣ���۳�����Ͷ�ݣ���ServLink::SendMsg

	if (_bCanWrite && size > 0)
	{
		PostSend(_sendBuf.beginRead(), _sendBuf.readableBytes());
	}
}