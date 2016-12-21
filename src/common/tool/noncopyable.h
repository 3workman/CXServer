#pragma once
namespace boost {
    class noncopyable{
    protected:
        noncopyable() = default;
        ~noncopyable() = default;
    private:
        noncopyable(const noncopyable&) = delete;
        noncopyable& operator=(const noncopyable&) = delete;
    };
}