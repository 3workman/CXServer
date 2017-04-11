/***********************************************************************
* @ �첽��־
* @ brief
    1��ǰ��Append()�ӿڣ������������ݣ�buf��д��ʱ������̨writeLoop
    2����̨writeLoopƽʱ������"::SleepConditionVariableCS"�����ȴ�"::WakeConditionVariable"�Ļ���
    3��"::SleepConditionVariableCS"��ʱ����ʱ��log
    4����ǿɱLog���̣�����buf�е����ݻ�û��д
    5����֤_curBuf��_nextBuf��spareBuf1��spareBuf2һֱ��bufVec��ͷ��
    6����ñ�������ȫ��or��̬���������߳̽����Ŵ������������߳̽���̫�����̽���

* @ race condition
    1�����̵߳Ĵ�����������AsyncLog�������֮��
        ��һ���õ�thread���󣬷ų�ʼ���б��У��з���
        ��һ�滹������bug��_running������_thread֮��_thread�ȳ�ʼ��������_WriteLoop��ֱ�Ӳ�ѭ����~��
        �ڳ�ʼ���б��е��ú��������Ǻ�ϰ��~~/(��o��)/~~

    2��AsyncLog��������ʱ�����뱣֤���߳�ִ����Ϻ��ٲ�ִ��_mutex��_curBuf...������Դ�Ļ���
        �������̻߳���ʱ�����ܷ��ʵ���Ч���ݡ��ڴ� ���� ����Stop()�б���ʹ��_thread->join();

    3�����졢�����Ķ��̰߳�ȫ�ԣ�Ӧ����һ��������ˣ�����ʱҪ����ע�⣡����
        ctor/dtor��;��������������̷߳���
        ������Դ��Ӧ�ȵ����г����߳̽����ú������һ���̸߳������

    4����̨_WriteLoop��IOʱ���������̣߳�Append���ݣ�������dtor��������Stop��_running false��
        �ٻص���̨�߳�ʱ��_WriteLoopѭ���жϣ�_bufVec�е����ݾͶ�ʧ��

* @ ��������
    1���������������� mutex ��ס��ʱ�� wait��������ס��wait�������mutex�ĕr��wait�Լ���release����
        ���ݽ�wait()��mutex�������������б���
        ���룺����̱߳������mutex�������ţ��������̱߳�����ʱ�ܵ��ٽ����ͻ�����ˣ�����������wait��Ŀ�Ĳ���

    2���ȴ���������Ҫ����һ��������������ʱ���ᷢ�����±仯��
        �Ȱѵ�ǰ�̷߳ŵ��ȴ��������߳��б��ϣ�Ȼ��Ի������������ٹ��𣨽����������̲߳��л������������
        ��ĳ���̵߳���֪ͨ����ʱ���ȴ������յ�֪ͨ���ְѻ�����������Ȼ��������²����ٽ�����

    3�����������ĵȴ�������whileѭ����Χ��ԭ��
        �ȴ������������յ�֪ͨ������һ������Ҫ��ס��������������������С��ʱ������棬�����߳����Ȼ�ȡ���⻥�����������ٽ�����ĳ��״̬�ı��ˡ�
        ��ʱ�����������Ӧ�ü����жϱ��˸ո������޸ĵ�״̬��������ִ��while���жϡ�
        ����һ��ԭ��ʱ��ֹ���֪ͨ���յ����֪ͨ��ֻҪwhile���������Ϊ�棬�ͼ������ߣ�

    4��pthread_cond_signal��ʱ���������pthread_cond_wait������pthread_cond_wait�ͻ�һֱ����ȥ

* @ author zhoumf
* @ date 2016-8-17
************************************************************************/
#pragma once
#include <thread>
#include <memory> // std::shared_ptr
#include <condition_variable>
#include "../tool/cLock.h"
#include "../Buffer/buffer.h"

class AsyncLog{
public:
    typedef net::Buffer Buffer;
    typedef std::shared_ptr<Buffer> BufferPtr;
    typedef std::vector<BufferPtr> BufferVec;
    typedef std::function<void(const BufferVec&)> WriteLogFunc; //Notice��������std::function���ô������lambda
    //typedef void(*WriteLogFunc)(const BufferVec&);

    AsyncLog(size_t maxSize, const WriteLogFunc& func);
    ~AsyncLog();

    void Append(const void* data, size_t len);

    void Stop(); //Notice��������������ȫ��or��̬ʱ��Ҫ�һ�������Stop���������߳̽��������߳�δ�����գ����̽���
private:
    void _WriteLoop();
private:
    cMutex                  _mutex;
    std::condition_variable _cond;

    //Notice�������̺߳�������_running��������Ҫ������_thread֮ǰ��ȷ����ȷ�ĳ�ʼ��˳��
    bool _running = true;
    std::thread* _thread = NULL; //����c++11��ָ���Ա�������ʱ��ָ��NULL����ֹctor��©��������Ұָ��

    size_t    _maxSize;
    BufferPtr _curBuf;
    BufferPtr _nextBuf;
    BufferVec _bufVec;

    WriteLogFunc _writeLogFunc;
};


/************************************************************************/
// ʾ��
#ifdef _MY_Test
void test_AsyncLog(){
    cout << "���������������������������첽��־��������������������������" << endl;
	AsyncLog log(1, [](const AsyncLog::BufferVec& vec){
        printf("Start IO\n");
        Sleep(3000); //Bug�������������̣߳�ִ����"Ex Append"�󴥷�AsyncLog��dtor���ᶪһ�����ݣ�ע�͡�race condition 4������֤

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