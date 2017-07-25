#pragma once
#include <queue>
#include "cLock.h"

template <typename T>
class SafeQueue {
    cMutex          m_csLock;
    std::queue<T>   m_queue;
public:
    void push(T&& v)
    {
        cLock lock(m_csLock);
        m_queue.push(v);
    }
    void push(const T& v)
    {
        cLock lock(m_csLock);
        m_queue.push(v);
    }
    void pop()
    {
        cLock lock(m_csLock);
        m_queue.pop();
    }
    bool pop(T& v)
    {
        cLock lock(m_csLock);
        if (m_queue.empty()) return false; // 空STL容器调front()、pop()直接宕机
        v = m_queue.front();
        m_queue.pop();
        return true;
    }
    bool front(T& v)
    {
        cLock lock(m_csLock);
        if (m_queue.empty()) return false;
        v = m_queue.front();
        return true;
    }
    int size()
    {
        cLock lock(m_csLock);
        return m_queue.size();
    }
    bool empty()
    {
        cLock lock(m_csLock);
        return m_queue.empty();
    }
};



#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
template <typename T>
class CSafeQueue {
private:
    mutable std::mutex      mut;
    std::queue<T>           m_queue;
    std::condition_variable m_cond;
public:
    void push(T new_value)
    {
        cLock lock(mut);
        m_queue.push(new_value);
        m_cond.notify_one();
    }
    bool try_pop(T& value)
    {
        cLock lock(mut);
        if (m_queue.empty()) return false;
        value = m_queue.front();
        m_queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop()
    {
        cLock lock(mut);
        if (m_queue.empty()) return false;
        std::shared_ptr<T> ret(std::make_shared<T>(m_queue.front()));
        m_queue.pop();
        return ret;
    }
    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lock(mut);
        m_cond.wait(lock, [this]{ return !m_queue.empty(); });
        value = m_queue.front();
        m_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lock(mut);
        m_cond.wait(lock, [this]{ return !m_queue.empty(); });
        std::shared_ptr<T> ret(std::make_shared<T>(m_queue.front()));
        m_queue.pop();
        return ret;
    }
    bool empty() const
    {
        cLock lock(mut);
        return m_queue.empty();
    }
};