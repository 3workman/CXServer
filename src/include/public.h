#pragma once
#include <iostream>
#include <assert.h>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <queue>
#include <stack>
#include <string>
#include <cstring>
#include <time.h>
#include <chrono>
#include <cmath>
#include <memory>
#include <random>
#include <unordered_map>
#include "tool/ScopeGuard.h"
#include "Log/LogFile.h"
#include "Rand/rand.h"


#define STATIC_ASSERT_ARRAY_LENGTH(arr, len) static_assert(sizeof(arr)/sizeof(arr[0])==(len))
#define STATIC_ASSERT_ARRAY_ARRAY(arrA, arrB) static_assert(sizeof(arrA)/sizeof(arrA[0])==sizeof(arrB)/sizeof(arrB[0]))

#define ZeroMemoryThis          memset(this, 0, sizeof(*this))
#define ZeroMemoryArray(arr)    memset(arr, 0, sizeof(arr))
#define ARRAY_SIZE(x)           (sizeof(x) / sizeof(*x))
#define BIND_THIS(func)         std::bind(&func, this, std::placeholders::_1)


template <typename T> inline int SUM_ARR(T* arr, int size){
    int sum(0);
    for (int j = 0; j < size; ++j) sum += arr[j];
    return sum;
}

typedef int64_t		int64;
typedef int32_t		int32;
typedef int16_t		int16;
typedef int8_t		int8;
typedef uint64_t	uint64;
typedef uint32_t	uint32;
typedef uint16_t	uint16;
typedef uint8_t		uint8;
typedef unsigned 	uint;
typedef std::vector< std::pair<int, int> > IntPairVec;

template<typename T> using shared = std::shared_ptr<T>;
template<typename T> using unique = std::unique_ptr<T>;
template<typename T> using weak   = std::weak_ptr<T>;
//---------------------------------------------------------
// std::weak_ptr does not support equality comparison for
// good reasons. In order to compare weak_ptrs, we have to
// lock them first, which incurs lock overhead. This could
// be very bad for container usages. fast weak_ptr does not
// suffer this problem as lock is very cheap. Thus the
// overloaded equality is really just for fast weak_ptrs.
//---------------------------------------------------------
template<typename T, typename U>
inline bool operator == (const weak<T>& lhs, const weak<U>& rhs) noexcept { return lhs.lock() == rhs.lock(); }
template<typename T, typename U>
inline bool operator == (const shared<T>& lhs, const weak<U>& rhs) noexcept { return lhs == rhs.lock(); }
template<typename T, typename U>
inline bool operator == (const weak<T>& lhs, const shared<U>& rhs) noexcept { return lhs.lock() == rhs; }

template<typename T, typename U>
inline bool operator != (const weak<T>& lhs, const weak<U>& rhs) noexcept { return lhs.lock() != rhs.lock(); }
template<typename T, typename U>
inline bool operator != (const shared<T>& lhs, const weak<U>& rhs) noexcept { return lhs != rhs.lock(); }
template<typename T, typename U>
inline bool operator != (const weak<T>& lhs, const shared<U>& rhs) noexcept { return lhs.lock() != rhs; }

template<typename T>
inline bool operator == (const weak<T>& lhs, nullptr_t) noexcept { return lhs.lock() == nullptr; }
template<typename T>
inline bool operator == (nullptr_t, const weak<T>& rhs) noexcept { return rhs.lock() == nullptr; }

template<typename T>
weak<T> weak_null() { return weak<T>(); }



#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#undef min
#undef max
#undef CreateWindow

inline int random() { return Rand::rand(); }

#else

#endif
