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
        return m_queue.push(v);
    }
    void push(const T& v)
    {
        cLock lock(m_csLock);
        return m_queue.push(v);
    }
    void pop()
    {
        cLock lock(m_csLock);
        return m_queue.pop();
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
        return m_queue.size();
    }
    int empty()
    {
        return m_queue.empty();
    }
};