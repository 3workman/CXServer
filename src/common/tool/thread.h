#pragma once
#include <thread>
#include <synchapi.h>

class Thread{
public:
	Thread(){
		_bEnd = false; 
		_hKillEvent = NULL;
	}
    ~Thread(){
        EndThread();
    };

	typedef void(*Callback)(LPVOID);
	bool RunThread(Callback func, LPVOID lParam = NULL)
	{
		_bEnd = false;

		if (_hKillEvent) return false;

		_hKillEvent = CreateEvent(NULL, 0, 0, 0);
		_thread = new std::thread(func, lParam);
		_thread->detach();
		return true;
	}
	void EndThread()
	{
		_bEnd = true;

		SetEvent(_hKillEvent);

		delete _thread;

		CloseHandle(_hKillEvent);
		_thread = NULL;
		_hKillEvent = NULL;
	}
	DWORD WaitKillEvent(DWORD dwMilliseconds = 0)
	{
		return _hKillEvent == NULL ? 0 : WaitForSingleObject(_hKillEvent, dwMilliseconds);
	}
private:
	bool		 _bEnd;
	HANDLE		 _hKillEvent;
	std::thread* _thread;
};