#include "stdafx.h"
#include "AsyncLog.h"

static const uint Flush_Interval_Sec = 180; //INFINITE

AsyncLog::AsyncLog(size_t maxSize, const WriteLogFunc& func)
    : _maxSize(maxSize)
    , _curBuf(new Buffer(maxSize))
    , _nextBuf(new Buffer(maxSize))
    , _writeLogFunc(func)
    //, _thread([&]{ this->_WriteLoop(bufSize); }) // 可能构造中途调度至子线程，访问到无效资源(_mutex)/未初始化变量
{
	_bufVec.reserve(8);
    //Notice：不能用[&]，maxSize是局部变量
    _thread = new std::thread([=]{ this->_WriteLoop(); });
    //_thread->detach(); // "析构/Stop"中的_thread->join()必不可少，这里就不要了
}
AsyncLog::~AsyncLog()
{
    if (_running) Stop();

    delete _thread;

    printf("dtor\n");
}

void AsyncLog::Append(const void* data, size_t len)
{
    cLock lock(_mutex);
    _curBuf->append(data, len);

    if (_curBuf->writableBytes() == 0)
    {
        _bufVec.push_back(_curBuf);

        //FIXME：new可能返回null，不过那会系统已经要跪了吧~
        if (_nextBuf)
            _curBuf = std::move(_nextBuf);
        else
            _curBuf.reset(new Buffer(_maxSize));
        _cond.notify_one();
        //::WakeConditionVariable(&_cond);
    }
}
void AsyncLog::_WriteLoop()
{
    BufferPtr spareBuf1(new Buffer(_maxSize));
    BufferPtr spareBuf2(new Buffer(_maxSize));
    BufferVec bufToWriteVec;
	bufToWriteVec.reserve(8);
    while (_running){
        //printf("loop\n");
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cond.wait_for(lock, std::chrono::seconds(Flush_Interval_Sec));
            //::SleepConditionVariableCS(&_cond, &_mutex, Flush_Interval_MS);

            bufToWriteVec.swap(_bufVec);
            bufToWriteVec.push_back(_curBuf);
            _curBuf = std::move(spareBuf1);
			if (_nextBuf == NULL) _nextBuf = std::move(spareBuf2);
        }
        _writeLogFunc(bufToWriteVec); // ??? IO时，主线程调了Append()且随即dtor，那本次Append的数据不会记Log，因为下次loop时_running为false了

        if (spareBuf1 == NULL){ spareBuf1 = bufToWriteVec[0]; spareBuf1->clear(); }
        if (spareBuf2 == NULL){ spareBuf2 = bufToWriteVec[1]; spareBuf2->clear(); }

        bufToWriteVec.clear();
    }
    printf("loop end\n");

    //【race condition 4】：AsyncLog对象析构，_running false，补写遗留数据
    // 执行析构/Stop才会跳出while循环，这里_bufVec写数据不存在竞态，Stop里的join会阻塞到_WriteLoop执行完毕
    _bufVec.push_back(_curBuf);
    _writeLogFunc(_bufVec);
}
void AsyncLog::Stop()
{
    _running = false;
    _cond.notify_one();
    //::WakeConditionVariable(&_cond);
    _thread->join(); //Notice：阻塞，等待子线程执行结束
    /*
        析构时，这里必须调join（不能detach），必须等子线程结束
        再才回收主线程的资源，如_mutex/_curBuf
        否则，主线程的析构先完成，WriteLoop线程再唤醒时，就访问到无效资源了
    */
}