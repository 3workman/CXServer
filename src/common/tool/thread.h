#pragma once
#include <thread>
#include <condition_variable>

using namespace std;

class Thread {
    std::thread*        _thread;
    cMutex              _mutex;
    condition_variable  _cond;
public:
    Thread() { _thread = NULL; }
    ~Thread(){ EndThread(); }

	typedef void(*Callback)(void*);
    bool RunThread(Callback func, void* lParam = NULL)
	{
		_thread = new std::thread(func, lParam);
		_thread->detach();
		return true;
	}
	void EndThread()
	{
        _cond.notify_one();
		delete _thread;
		_thread = NULL;
	}
    cv_status WaitKillEvent(uint dwMilliseconds = 0)
	{
        std::unique_lock<std::mutex> lock(_mutex);
        return _cond.wait_for(lock, std::chrono::milliseconds(dwMilliseconds));
	}
};

/*
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
    uint WaitKillEvent(uint dwMilliseconds = 0)
    {
        // WAIT_TIMEOUT
        return _hKillEvent == NULL ? 0 : WaitForSingleObject(_hKillEvent, dwMilliseconds);
    }
private:
    bool		 _bEnd;
    HANDLE		 _hKillEvent;
    std::thread* _thread;
};
*/