/***********************************************************************
* @ 内存池
* @ brief
	1、预先申请一大块内存，按固定大小分页，页头地址给外界使用
	2、多涉及operator new、operator delete

* @ Notice
    1、cPool_Index 内存池里的对象，有m_index数据，实为内存索引
    2、被外界保存时，可能对象已经历消亡、复用，那么通过保存的idx找到的就是错误指针了
    3、比如保存NpcID，那个Npc已经死亡，内存恰好被新Npc复用，此时通过NpcID找到的就不知道是啥了
    4、可以在对象里加个自增变量，比如“uint16 autoId;”，同m_index合并成uint32，当唯一id。避免外界直接使用m_index
    5、空STL容器调front()、pop()直接宕机

    void MyClass::_CreateUniqueId() //仅在对象新建时调用，其它地方直接用 m_unique_id
    {
        static uint16 s_auto_id = 0;

        m_unique_id = ((++s_auto_id) << 16) + m_index; //对象数16位就够用，不够的话封个64位的union吧
    }
    MyClass* MyClass::FindGroup(uint32 uniqueId)
    {
        int idx = uniqueId & 0xFFFF;

        if (MyClass* ret = FindByIdx(idx))
            if (ret->m_unique_id == uniqueId)
                return ret;
        return NULL;
    }

* @ author zhoumf
* @ date 2014-11-21
************************************************************************/
#pragma once
#include "SafeQueue.h"

// 检查一段内存是否越界(头尾设标记)
#define CHECKNU 6893    // 除0外任意值
#define PRECHECK_FIELD(i) int __precheck##i;
#define POSCHECK_FIELD(i) int __poscheck##i;
#define INIT_CHECK(o, i) { (o)->__precheck##i = CHECKNU; (o)->__poscheck##i = CHECKNU; }
#define CHECK(o, i){\
if ((o)->__precheck##i != CHECKNU || (o)->__poscheck##i != CHECKNU){\
	printf("%s:%d, memory access out of range with checknu pre %d,pos %d", \
	__FILE__, __LINE__, (o)->__precheck##i, (o)->__poscheck##i);}\
}

class CPoolPage{//线程安全的
	const size_t	  m_pageSize;
	const size_t	  m_pageCnt;
    SafeQueue<void*>  m_queue;

    bool Double() // 可设置Double次数限制
    {
        // 无初始化，外界要operator new或调用new(ptr)
        char* p = (char*)malloc(m_pageSize * m_pageCnt); // 溢出风险：m_pageSize * m_pageNum
        if (!p) return false;

        for (size_t i = 0; i < m_pageCnt; ++i) {
            m_queue.push(p);
            p += m_pageSize;
        }
        return true;
    }
public:
	CPoolPage(size_t pageSize, size_t pageCnt) : m_pageSize(pageSize), m_pageCnt(pageCnt){
        /*  对象构造的线程安全：
            1、不要在ctor中注册回调
            2、此时也不要把this传给跨线程的对象
            3、注意调用的成员函数也可能干坏事
            4、即便在构造函数的最后一行也不行
        */
		Double();
	}
	void* Alloc(){
        void* ret = NULL;
        if (!m_queue.pop(ret))
        {
            if (Double()) m_queue.pop(ret);
        }
        return ret;
	}
	void Dealloc(void* p){
		m_queue.push(p);
	}
};

class CPoolLevel {
    enum {
        MinBlock = 32,  //最小内存块尺寸
        MaxLevel = 5,   //每高一级，尺寸加倍，数量减半
    };
#define LevelSize(lv) (MinBlock << (lv))
#define LevelCnt(lv)  (m_MaxLvCnt << (MaxLevel-(lv)))

    const size_t	  m_MaxLvCnt;
    SafeQueue<void*>  m_queue[MaxLevel+1];

public:
    CPoolLevel(size_t maxLvCnt) : m_MaxLvCnt(maxLvCnt)
    {
        for (int lv = 0; lv <= MaxLevel; ++lv)
        {
            _Double(lv);
        }
    }
    void* Alloc(size_t size)
    {
        int lv = _GetLevel(size);

        if (lv == -1) return malloc(size);

        void* ret = NULL;
        if (!m_queue[lv].pop(ret))
        {
            if (_Append(lv)) m_queue[lv].pop(ret);
        }
        return ret;
    }
    void Dealloc(void* p, size_t size)
    {
        int lv = _GetLevel(size);
        if (lv == -1) {
            free(p);
            return;
        }
        m_queue[lv].push(p);
    }
private:
    bool _Double(uint lv)
    {
        size_t pageSize = LevelSize(lv);
        size_t pageCnt  = LevelCnt(lv);

        char* p = (char*)malloc(pageSize * pageCnt); // 溢出风险：m_pageSize * m_pageNum
        if (!p) return false;

        for (size_t i = 0; i < pageCnt; ++i) {
            m_queue[lv].push(p);
            p += pageSize;
        }
        return true;
    }
    bool _Append(uint lv)
    {
        //低级的不够，向高级的要
        return (lv < MaxLevel) ? _Split(lv+1) : _Double(lv);
    }
    bool _Split(uint lv) //将本级内存块拆分，加到小一级内存池中
    {
        if (m_queue[lv].empty())
        {
            if (lv == MaxLevel) { //【Notice】不加大括号，if else匹配歧义
                if (!_Double(lv)) return false;
            } else {
                if (!_Split(lv + 1)) return false;
            }
        }

        size_t size = m_queue[lv].size()/2+1;
        void* tmp = NULL;
        do  {
            m_queue[lv].pop(tmp);
            m_queue[lv-1].push(tmp);
            m_queue[lv-1].push((char*)tmp + LevelSize(lv - 1));
        } while (--size);
        return true;
    }
    int _GetLevel(int size)
    {
        for (int i = 0; i <= MaxLevel; ++i)
        {
            if (size <= LevelSize(i)) return i;
        }
        assert(0);
        return -1;
    }
#undef LevelSize
#undef LevelCnt
};

// 潜在Bug：m_index被外界保存，但象已经消亡，该内存块又恰巧被新对象复用……此时逻辑层定位到的指针就错乱了
#define VOID_POOL_ID -1
template <class T>
class CPoolIndex{ // 自动编号，便于管理(对象要含有m_index变量，记录其内存id)，【非线程安全】
	T**    m_arrPtr;
    uint   m_cnt;
    std::queue<uint> m_queue;
public:
    CPoolIndex(uint num) : m_cnt(num){
		m_arrPtr = (T**)malloc(m_cnt * sizeof(T*));
		if (!m_arrPtr) return;

        //T* pObj = ::new T[m_num]; // 若类没operator new，就用全局的new(确保初始化)
        T* pObj = (T*)malloc(m_cnt * sizeof(T));
        if (!pObj) return;	         // 若类operator new，此处用new，会多次调用构造函数

        for (uint i = 0; i < m_cnt; ++i) {
            m_arrPtr[i] = pObj++;
            m_queue.push(i);
        }
	}
	bool Double(){
		T** temp = (T**)malloc(m_cnt * 2 * sizeof(T*)); //存指针的数组，须连续
		if (!temp) return false;

        T* pObj = (T*)malloc(m_cnt * sizeof(T)); //开辟新内存块，存对象，可散列
        if (!pObj) return false;

		memcpy(temp, m_arrPtr, m_cnt * sizeof(T*));
		free(m_arrPtr);	m_arrPtr = temp;

        for (uint i = 0; i < m_cnt; ++i) {
            m_arrPtr[m_cnt + i] = pObj++;
            m_queue.push(m_cnt + i);
        }
		m_cnt *= 2;
		return true;
	}
	T* Alloc(){
		if (m_queue.empty() && !Double()) return NULL;
        uint id = m_queue.front();
		m_queue.pop();
        T::_StaticIndex() = id; // 分配时设置内存id
        //m_arrPtr[id]->m_index = id; //编译器可能operator new后autoclassinit2，置0数据
		return m_arrPtr[id];
	}
	void Dealloc(T* p){
		m_queue.push(p->m_index);
		p->m_index = VOID_POOL_ID; // 回收后置空内存id
	}
    T* GetByIdx(uint id){
		if (id >= m_cnt) return NULL;
		return VOID_POOL_ID == m_arrPtr[id]->m_index ? NULL : m_arrPtr[id];
	}
};
#define Pool_Index_Define(T, size) \
        static CPoolIndex<T>& _Pool(){ static CPoolIndex<T> pool(size); return pool; } \
        public: \
        static uint& _StaticIndex(){ static uint idx; return idx; } \
        uint m_index = _StaticIndex(); \
        void* operator new(size_t /*size*/){ return _Pool().Alloc(); }\
        void* operator new(size_t /*size*/, const char* file, int line){ return _Pool().Alloc(); }\
        void operator delete(void* p, const char* file, int line){ return _Pool().Dealloc((T*)p); }\
        void operator delete(void* p, size_t) { return _Pool().Dealloc((T*)p); }\
        static T* FindByIdx(uint idx){ return _Pool().GetByIdx(idx); }

// 防止索引内存池，定位错乱：如外界持有Npc索引，但该npc已被回收，再定位到的可能是新npc了
#define Pool_Index_UniqueID32(T) \
        private: \
        uint32 m_unique_id = _CreateUniqueId(); \
        uint32 _CreateUniqueId() { \
            assert(m_index <= 0xFFFF); \
            static uint16 s_auto_id = 0; \
            return ((++s_auto_id) << 16) | m_index; \
        } \
        public: \
        uint32 GetUniqueId(){ return m_unique_id; } \
        static T* FindByUniqueId(uint32 uniqueId) { \
            if (T* ret = T::FindByIdx(uniqueId & 0xFFFF)) \
                if (ret->m_unique_id == uniqueId) \
                    return ret; \
            return NULL; \
        }
#define Pool_Index_UniqueID64(T) \
        private: \
        uint64 m_unique_id = _CreateUniqueId(); \
        uint64 _CreateUniqueId() { \
            static uint32 s_auto_id = 0; \
            return ((uint64)(++s_auto_id) << 32) | m_index; \
        } \
        public: \
        uint64 GetUniqueId(){ return m_unique_id; } \
        static T* FindByUniqueId(uint64 uniqueId) { \
            if (T* ret = T::FindByIdx(uniqueId & 0xFFFFFFFF)) \
                if (ret->m_unique_id == uniqueId) \
                    return ret; \
            return NULL; \
        }

template <class T>
class CPoolObj{
	CPoolPage	m_pool;
public:
	CPoolObj(size_t num) : m_pool(sizeof(T), num){}
	T* Alloc(){ return (T*)m_pool.Alloc(); }
	void Dealloc(T* p){ m_pool.Dealloc(p); }
};
#define Pool_Obj_Define(T, size) \
        static CPoolObj<T>& _Pool(){ static CPoolObj<T> pool(size); return pool; } \
        public: \
		void* operator new(size_t /*size*/){ return _Pool().Alloc(); }\
		void* operator new(size_t /*size*/, const char* file, int line){ return _Pool().Alloc(); }\
		void operator delete(void* p, const char* file, int line){ return _Pool().Dealloc((T*)p); }\
		void operator delete(void* p, size_t) { return _Pool().Dealloc((T*)p); }


/************************************************************************/
// 示例
#ifdef _MY_Test
	CPoolPage g_pool(4/*sizeof(cObj_Pool)*/, 2);
	class cObj_Pool{
		int _a = -1;
	public:
		cObj_Pool() { cout << "默认构造函数：" << _a << endl; }
		cObj_Pool(int a) : _a(a) { cout << "含参构造函数：" << _a << endl; }

		void* operator new(size_t){ return g_pool.Alloc(); }
		void* operator new(size_t, void* p){ return p; } // 默认的new(ptr)跟这个的实现一样，仅return p;
	};

	void test_Mempool(){
		cout << "——————————————内存池——————————————" << endl;

 		cObj_Pool* p = (cObj_Pool*)g_pool.Alloc(); // 无初始化内存块
		new(p)cObj_Pool(5);		// 重载版本——在ptr所指地址上构建一个对象(调构造函数)
		::new(p)cObj_Pool(10);	// 全局版本——在ptr所指地址上构建一个对象(调构造函数)

		cObj_Pool* pp = new cObj_Pool;
	}
#endif