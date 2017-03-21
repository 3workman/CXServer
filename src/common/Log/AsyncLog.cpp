#include "stdafx.h"
#include "AsyncLog.h"

static const DWORD Flush_Interval_MS = 180 * 1000; //INFINITE

AsyncLog::AsyncLog(size_t maxSize, const WriteLogFunc& func)
    : _curBuf(new Buffer(maxSize))
    , _nextBuf(new Buffer(maxSize))
    , _writeLogFunc(func)
    , _maxSize(maxSize)
    //, _thread([&]{ this->_WriteLoop(bufSize); }) // ���ܹ�����;���������̣߳����ʵ���Ч��Դ(_mutex)/δ��ʼ������
{
    ::InitializeConditionVariable(&_cond);

	_bufVec.reserve(8);

    //Notice��������[&]��maxSize�Ǿֲ�����
    _thread = new std::thread([=]{ this->_WriteLoop(); });
    //_thread->detach(); // "����/Stop"�е�_thread->join()�ز����٣�����Ͳ�Ҫ��
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

        //FIXME��new���ܷ���null�������ǻ�ϵͳ�Ѿ�Ҫ���˰�~
        _nextBuf ? (_curBuf = std::move(_nextBuf)) : 
                   (_curBuf.reset(new Buffer(_maxSize)));
        ::WakeConditionVariable(&_cond);
    }
}
void AsyncLog::_WriteLoop()
{
    BufferPtr spareBuf1(new Buffer(_maxSize));
    BufferPtr spareBuf2(new Buffer(_maxSize));
    BufferVec bufToWriteVec;
	bufToWriteVec.reserve(8);
    while (_running){
        printf("loop\n");
        {
            cLock lock(_mutex);
            ::SleepConditionVariableCS(&_cond, &_mutex, Flush_Interval_MS);

            bufToWriteVec.swap(_bufVec);
            bufToWriteVec.push_back(_curBuf);
            _curBuf = std::move(spareBuf1);
			if (_nextBuf == NULL) _nextBuf = std::move(spareBuf2);
        }
        _writeLogFunc(bufToWriteVec); // ??? IOʱ�����̵߳���Append()���漴dtor���Ǳ���Append�����ݲ����Log����Ϊ�´�loopʱ_runningΪfalse��

        if (spareBuf1 == NULL){ spareBuf1 = bufToWriteVec[0]; spareBuf1->clear(); }
        if (spareBuf2 == NULL){ spareBuf2 = bufToWriteVec[1]; spareBuf2->clear(); }

        bufToWriteVec.clear();
    }
    printf("loop end\n");

    //��race condition 4����AsyncLog����������_running false����д��������
    // ִ������/Stop�Ż�����whileѭ��������_bufVecд���ݲ����ھ�̬��Stop���join��������_WriteLoopִ�����
    _bufVec.push_back(_curBuf);
    _writeLogFunc(_bufVec);
}
void AsyncLog::Stop()
{
    _running = false;
    ::WakeConditionVariable(&_cond);
    _thread->join(); //Notice���������ȴ����߳�ִ�н���
    /*
        ����ʱ����������join������detach������������߳̽���
        �ٲŻ������̵߳���Դ����_mutex/_curBuf
        �������̵߳���������ɣ�WriteLoop�߳��ٻ���ʱ���ͷ��ʵ���Ч��Դ��
    */
}