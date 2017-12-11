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
#include <cmath>
#include <memory>
#include <random>
#include "tool/ScopeGuard.h"
#include "Log/LogFile.h"
#include "Rand/rand.h"


#define STATIC_ASSERT_ARRAY_LENGTH(arr, len) static_assert(sizeof(arr)/sizeof(arr[0])==(len), #arr)
#define STATIC_ASSERT_ARRAY_ARRAY(arrA, arrB) static_assert(sizeof(arrA)/sizeof(arrA[0])==sizeof(arrB)/sizeof(arrB[0]), #arrA)

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

// define smart pointer types for easier typing
template<typename T> using shared = std::shared_ptr<T>;
template<typename T> using unique = std::unique_ptr<T>;
template<typename T> using weak = std::weak_ptr<T>;

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
