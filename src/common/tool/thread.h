#pragma once
#include <thread>
#include <condition_variable>

class Thread {
    std::thread*                _thread;
    cMutex                      _mutex;
    std::condition_variable     _cond;
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
    std::cv_status WaitKillEvent(uint dwMilliseconds = 0)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _cond.wait_for(lock, std::chrono::milliseconds(dwMilliseconds));
    }
};
