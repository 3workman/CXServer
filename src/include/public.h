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

#define USE_FAST_SMART_PTR 1

// define smart pointer types for easier typing
#if USE_FAST_SMART_PTR
#include "shared_ptr.h"
template<typename T> using unique = std::unique_ptr<T>;
template<typename T> using shared = cr::shared_ptr<T>;
template<typename T> using weak   = cr::weak_ptr<T>;

//using cr::make_shared;
using cr::enable_shared_from_this;
using cr::dynamic_pointer_cast;
using cr::static_pointer_cast;
using cr::const_pointer_cast;

template<typename T, typename ...Args>
inline shared<T> make_shared(Args&& ... args)
{
    return shared<T>(new T(std::forward<Args>(args)...));
}

using cr::owner_less;

template<typename T>
using weak_set = std::set<weak<T>, owner_less<>>;

#else
template<typename T> using shared = std::shared_ptr<T>;
template<typename T> using unique = std::unique_ptr<T>;
template<typename T> using weak   = std::weak_ptr<T>;

using std::make_shared;
using std::enable_shared_from_this;
using std::dynamic_pointer_cast;
using std::static_pointer_cast;

using std::owner_less;
#endif


#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#undef min
#undef max
#undef CreateWindow
#undef Yield

inline int random() { return Rand::rand(); }

#else

#endif
