/***********************************************************************
* @ �ڴ��
* @ brief
	1��Ԥ������һ����ڴ棬���̶���С��ҳ��ҳͷ��ַ�����ʹ��
	2�����漰operator new��operator delete

* @ Notice
    1��cPool_Index �ڴ����Ķ�����m_index���ݣ�ʵΪ�ڴ�����
    2������籣��ʱ�����ܶ����Ѿ������������ã���ôͨ�������idx�ҵ��ľ��Ǵ���ָ����
    3�����籣��NpcID���Ǹ�Npc�Ѿ��������ڴ�ǡ�ñ���Npc���ã���ʱͨ��NpcID�ҵ��ľͲ�֪����ɶ��
    4�������ڶ�����Ӹ��������������硰uint16 autoId;����ͬm_index�ϲ���uint32����Ψһid���������ֱ��ʹ��m_index
    5����STL������front()��pop()ֱ��崻�

    void MyClass::_CreateUniqueId() //���ڶ����½�ʱ���ã������ط�ֱ���� m_unique_id
    {
        static uint16 s_auto_id = 0;

        m_unique_id = ((++s_auto_id) << 16) + m_index; //������16λ�͹��ã������Ļ����64λ��union��
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

// ���һ���ڴ��Ƿ�Խ��(ͷβ����)
#define CHECKNU 6893    // ��0������ֵ
#define PRECHECK_FIELD(i) int __precheck##i;
#define POSCHECK_FIELD(i) int __poscheck##i;
#define INIT_CHECK(o, i) { (o)->__precheck##i = CHECKNU; (o)->__poscheck##i = CHECKNU; }
#define CHECK(o, i){\
if ((o)->__precheck##i != CHECKNU || (o)->__poscheck##i != CHECKNU){\
	printf("%s:%d, memory access out of range with checknu pre %d,pos %d", \
	__FILE__, __LINE__, (o)->__precheck##i, (o)->__poscheck##i);}\
}

class CPoolPage{//�̰߳�ȫ��
	const size_t	  m_pageSize;
	const size_t	  m_pageCnt;
    SafeQueue<void*>  m_queue;

    bool Double() // ������Double��������
    {
        // �޳�ʼ�������Ҫoperator new�����new(ptr)
        char* p = (char*)malloc(m_pageSize * m_pageCnt); // ������գ�m_pageSize * m_pageNum
        if (!p) return false;

        for (size_t i = 0; i < m_pageCnt; ++i) {
            m_queue.push(p);
            p += m_pageSize;
        }
        return true;
    }
public:
	CPoolPage(size_t pageSize, size_t pageCnt) : m_pageSize(pageSize), m_pageCnt(pageCnt){
        /*  ��������̰߳�ȫ��
            1����Ҫ��ctor��ע��ص�
            2����ʱҲ��Ҫ��this�������̵߳Ķ���
            3��ע����õĳ�Ա����Ҳ���ܸɻ���
            4�������ڹ��캯�������һ��Ҳ����
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
        MinBlock = 32,  //��С�ڴ��ߴ�
        MaxLevel = 5,   //ÿ��һ�����ߴ�ӱ�����������
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

        char* p = (char*)malloc(pageSize * pageCnt); // ������գ�m_pageSize * m_pageNum
        if (!p) return false;

        for (size_t i = 0; i < pageCnt; ++i) {
            m_queue[lv].push(p);
            p += pageSize;
        }
        return true;
    }
    bool _Append(uint lv)
    {
        //�ͼ��Ĳ�������߼���Ҫ
        return (lv < MaxLevel) ? _Split(lv+1) : _Double(lv);
    }
    bool _Split(uint lv) //�������ڴ���֣��ӵ�Сһ���ڴ����
    {
        if (m_queue[lv].empty())
        {
            if (lv == MaxLevel) { //��Notice�����Ӵ����ţ�if elseƥ������
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

// Ǳ��Bug��m_index����籣�棬�����Ѿ����������ڴ����ǡ�ɱ��¶����á�����ʱ�߼��㶨λ����ָ��ʹ�����
#define VOID_POOL_ID -1
template <class T>
class CPoolIndex{ // �Զ���ţ����ڹ���(����Ҫ����m_index��������¼���ڴ�id)�������̰߳�ȫ��
	T**    m_arrPtr;
    uint   m_cnt;
    std::queue<uint> m_queue;
public:
    CPoolIndex(uint num) : m_cnt(num){
		m_arrPtr = (T**)malloc(m_cnt * sizeof(T*));
		if (!m_arrPtr) return;

        //T* pObj = ::new T[m_num]; // ����ûoperator new������ȫ�ֵ�new(ȷ����ʼ��)
        T* pObj = (T*)malloc(m_cnt * sizeof(T));
        if (!pObj) return;	         // ����operator new���˴���new�����ε��ù��캯��

        for (uint i = 0; i < m_cnt; ++i) {
            m_arrPtr[i] = pObj++;
            m_queue.push(i);
        }
	}
	bool Double(){
		T** temp = (T**)malloc(m_cnt * 2 * sizeof(T*)); //��ָ������飬������
		if (!temp) return false;

        T* pObj = (T*)malloc(m_cnt * sizeof(T)); //�������ڴ�飬����󣬿�ɢ��
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
        T::_StaticIndex() = id; // ����ʱ�����ڴ�id
        //m_arrPtr[id]->m_index = id; //����������operator new��autoclassinit2����0����
		return m_arrPtr[id];
	}
	void Dealloc(T* p){
		m_queue.push(p->m_index);
		p->m_index = VOID_POOL_ID; // ���պ��ÿ��ڴ�id
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

// ��ֹ�����ڴ�أ���λ���ң���������Npc����������npc�ѱ����գ��ٶ�λ���Ŀ�������npc��
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
// ʾ��
#ifdef _MY_Test
	CPoolPage g_pool(4/*sizeof(cObj_Pool)*/, 2);
	class cObj_Pool{
		int _a = -1;
	public:
		cObj_Pool() { cout << "Ĭ�Ϲ��캯����" << _a << endl; }
		cObj_Pool(int a) : _a(a) { cout << "���ι��캯����" << _a << endl; }

		void* operator new(size_t){ return g_pool.Alloc(); }
		void* operator new(size_t, void* p){ return p; } // Ĭ�ϵ�new(ptr)�������ʵ��һ������return p;
	};

	void test_Mempool(){
		cout << "�����������������������������ڴ�ء���������������������������" << endl;

 		cObj_Pool* p = (cObj_Pool*)g_pool.Alloc(); // �޳�ʼ���ڴ��
		new(p)cObj_Pool(5);		// ���ذ汾������ptr��ָ��ַ�Ϲ���һ������(�����캯��)
		::new(p)cObj_Pool(10);	// ȫ�ְ汾������ptr��ָ��ַ�Ϲ���һ������(�����캯��)

		cObj_Pool* pp = new cObj_Pool;
	}
#endif