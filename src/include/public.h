#pragma once
#include <iostream>
#include <assert.h>

#include <set>
#include <map>
#include <hash_map>
#include <list>
#include <vector>
#include <queue>
#include <stack>
#include <string>
#include "..\common\tool\noncopyable.h"
#include "..\common\tool\ScopeGuard.h"

using namespace std;

#define STATIC_ASSERT(exp, name) typedef int dummy##name [(exp) ? 1 : -1]
#define STATIC_ASSERT_ARRAY_LENGTH(arr, len) STATIC_ASSERT(sizeof(arr)/sizeof(arr[0])==(len), ___sizeErr##arr)
#define STATIC_ASSERT_ARRAY_ARRAY(arrA, arrB) STATIC_ASSERT(sizeof(arrA)/sizeof(arrA[0])==sizeof(arrB)/sizeof(arrB[0]), ___sizeErr##arr)

#define TimeElasped_Msec        GetTickCount()
#define ZeroMemoryThis          memset(this, 0, sizeof(*this))
#define ZeroMemoryArray(arr)    memset(arr, 0, sizeof(arr))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))
#define ONE_DAY_SEC   (24*3600)

template <typename T> int SUM_ARR(T* arr, int size){
    int sum(0);
    for (int j = 0; j < size; ++j) sum += arr[j];
    return sum;
}

typedef unsigned char       byte;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef unsigned int		uint;
typedef signed __int64		int64;
typedef signed __int32		int32;
typedef signed __int16		int16;
typedef signed __int8		int8;
typedef unsigned __int64	uint64;
typedef unsigned __int32	uint32;
typedef unsigned __int16	uint16;
typedef unsigned __int8		uint8;

typedef std::vector< std::pair<int, int> > IntPairVec;
