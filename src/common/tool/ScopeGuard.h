/***********************************************************************
* @ ��Դ���գ�defer
* @ brief
    1������Դ���������յĴ����ٽ���һ�𣬱�����֦�����µ���Դй¶

    2��������
        File* file = CreateFile(...);
        ON_SCOPE_EXIT([&]{ CloseFile(file); }); // Lambda���ʽ�����ô���

    3��rollbackģʽ
        CRollBack onFailRollback([&]{ "rollback code" });
        {
            // do something that could fail return
            // if this scope fail return, "rollback code" will be executed
        }
        onFailRollback.Invalid();

* @ author zhoumf
* @ date 2016-8-17
************************************************************************/
#pragma once
#include <functional>

class ScopeGuard : boost::noncopyable{
    std::function<void()> _onExitScope;
public:
    explicit ScopeGuard(const std::function<void()>& func) : _onExitScope(func){}
    ~ScopeGuard(){ _onExitScope(); }
};
class CRollBack : boost::noncopyable{
    std::function<void()> _onExitScope;
    bool _valid;
public:
    explicit CRollBack(const std::function<void()>& func)
        : _onExitScope(func)
        , _valid(true)
    { }

    ~CRollBack()
    {
        if (_valid) _onExitScope();
    }
    void Invalid(){ _valid = false; }
};
#define _SCOPEGUARD_LINENAME(name, line) name##line
#define ON_SCOPE_EXIT(callback) ScopeGuard _SCOPEGUARD_LINENAME(EXIT, __LINE__)(callback)