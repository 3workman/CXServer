/***********************************************************************
* @ 异步日志
* @ brief
    1、前端Append()接口，用以输入数据，buf被写满时触发后台writeLoop
    2、后台writeLoop平时阻塞在"::SleepConditionVariableCS"处，等待"::WakeConditionVariable"的唤醒
    3、"::SleepConditionVariableCS"超时，及时记log
    4、若强杀Log进程，可能buf中的数据还没被写
    5、保证_curBuf、_nextBuf、spareBuf1、spareBuf2一直在bufVec的头部
    6、最好别声明成全局or静态，否则主线程结束才触发析构，子线程结束太晚，进程僵死

* @ race condition
    1、子线程的创建，必须在AsyncLog构造完毕之后
        第一版用的thread对象，放初始化列表中，有风险
        第一版还碰到个bug，_running声明在_thread之后，_thread先初始化，进入_WriteLoop就直接不循环了~囧
        在初始化列表中调用函数，不是好习惯~~/(ㄒoㄒ)/~~

    2、AsyncLog对象析构时，必须保证子线程执行完毕后，再才执行_mutex、_curBuf...共享资源的回收
        否则，子线程唤醒时，可能访问到无效数据、内存 —— 所以Stop()中必须使用_thread->join();

    3、构造、析构的多线程安全性，应该是一类大问题了，编码时要额外注意！！！
        ctor/dtor中途，避免对象被其它线程访问
        共享资源，应等到所有持有线程解引用后，由最后一个线程负责回收

    4、后台_WriteLoop做IO时，跳至主线程，Append数据，并触发dtor或主动调Stop，_running false；
        再回到后台线程时，_WriteLoop循环中断，_bufVec中的数据就丢失了

* @ 条件变量
    1、条件变量必须在 mutex 锁住的时候 wait（必须锁住是wait本身接受mutex的時候。wait自己會release它）
        传递进wait()的mutex对条件变量进行保护
        猜想：如果线程被挂起后，mutex还被锁着，那其它线程被调度时跑到临界区就会挂起了，跟条件变量wait的目的不符

    2、等待函数里面要传入一个互斥量，调用时它会发生如下变化：
        先把当前线程放到等待条件的线程列表上，然后对互斥量解锁，再挂起（解锁后其他线程才有机会获得这个锁）
        当某个线程调用通知函数时，等待函数收到通知后，又把互斥量加锁，然后继续向下操作临界区。

    3、条件变量的等待函数用while循环包围。原因：
        等待的条件变量收到通知，它下一步就是要锁住这个互斥量，但在这个极小的时间差里面，其他线程抢先获取了这互斥量并进入临界区把某个状态改变了。
        此时这个条件变量应该继续判断别人刚刚抢先修改的状态，即继续执行while的判断。
        还有一个原因时防止虚假通知，收到虚假通知后，只要while里面的条件为真，就继续休眠！

    4、pthread_cond_signal的时间可能早于pthread_cond_wait，这样pthread_cond_wait就会一直等下去

* @ author zhoumf
* @ date 2016-8-17
************************************************************************/
#pragma once
#include "tool/cLock.h"
#include "Buffer/buffer.h"
#include <condition_variable>
#include <thread>

class AsyncLog{
public:
    typedef net::Buffer Buffer;
    typedef std::shared_ptr<Buffer> BufferPtr;
    typedef std::vector<BufferPtr> BufferVec;
    typedef std::function<void(const BufferVec&)> WriteLogFunc; //Notice：声明成std::function可用带捕获的lambda
    //typedef void(*WriteLogFunc)(const BufferVec&);

    AsyncLog(size_t maxSize, const WriteLogFunc& func);
    ~AsyncLog();

    void Append(const void* data, size_t len);

    void Stop(); //Notice：这玩意声明成全局or静态时，要找机会主动Stop，否则主线程结束，子线程未被回收，进程僵死
private:
    void _WriteLoop();
private:
    cMutex                  _mutex;
    std::condition_variable _cond;

    //Notice：因子线程函数依赖_running，所以它要声明在_thread之前，确保正确的初始化顺序
    bool _running = true;
    std::thread* _thread = NULL; //有了c++11，指针成员最好声明时即指定NULL，防止ctor中漏掉，出现野指针

    size_t    _maxSize;
    BufferPtr _curBuf;
    BufferPtr _nextBuf;
    BufferVec _bufVec;

    WriteLogFunc _writeLogFunc;
};


/************************************************************************/
// 示例
#ifdef _MY_Test
void test_AsyncLog(){
    cout << "—————————————异步日志—————————————" << endl;
	AsyncLog log(1, [](const AsyncLog::BufferVec& vec){
        printf("Start IO\n");
        Sleep(3000); //Bug：这里跳到主线程，执行了"Ex Append"后触发AsyncLog的dtor，会丢一波数据，注释【race condition 4】可验证

        printf("Log :\n    ");
        static int i = 0; string str;
        for (auto& it : vec){
            str.assign(it->beginRead(), it->readableBytes());
            if (!str.empty()) printf("(%d)%s - ", ++i, str.c_str());
        }
        printf("\n");
    });

    for (int i = 0; i < 15; ++i)
    {
        log.Append("aa", 3);
        Sleep(100);
    }
    log.Append("aa", 3);
    log.Append("bb", 3);

    printf("Ex Append\n");
    log.Append("cc", 3);
    log.Append("dd", 3);
}
#endif
