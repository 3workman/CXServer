/***********************************************************************
* @ 组合式ID
* @ brief
	1、在索引基础上，另配些数据，组成唯一ID，解决索引复用的区分问题
	2、如Mempool.h索引内存池中的对象，回收再利用，仅通过m_index，外部保存的旧idx可能定位到新对象

* @ author zhoumf
* @ date 2016-12-9
************************************************************************/
#pragma once

union ID64 {
public:
    ID64() : unique_id(0)
    {}
    ID64(const uint64& id) : unique_id(id)
    {}
    explicit ID64(uint32 typ, uint32 idx) : _type(typ), _index(idx)
    {}
    explicit ID64(uint8 mainTyp, uint32 idx, uint8 typ1 = 0, uint8 typ2 = 0, uint8 typ3 = 0)
        : _mainTyp(mainTyp)
        , _index(idx)
        , _typ1(typ1)
        , _typ2(typ2)
        , _typ3(typ3)
    {}

    inline uint32& Idx() { return _index; }
    inline uint32& Typ() { return _type; }
    inline bool operator < (const ID64 &rhs) { return unique_id < rhs.unique_id; }
    inline bool operator == (const ID64 &rhs) { return unique_id == rhs.unique_id; }

public:
    uint64 unique_id;
private:
    struct
    {
        uint32 _index; // 低
        union {
            uint32 _type;
            struct {
            uint8  _mainTyp;
            uint8  _typ1;
            uint8  _typ2;
            uint8  _typ3; // 高
            };
        };
    };
};

union ID32 {
public:
    ID32() : unique_id(0)
    {}
    ID32(const uint32& id) : unique_id(id)
    {}
    explicit ID32(uint16 typ, uint16 idx) : _type(typ), _index(idx)
    {}

    inline uint16& Idx() { return _index; }
    inline uint16& Typ() { return _type; }
    inline bool operator < (const ID32 &rhs) { return unique_id < rhs.unique_id; }
    inline bool operator == (const ID32 &rhs) { return unique_id == rhs.unique_id; }

public:
    uint32 unique_id;
private:
    struct
    {
        uint16 _index; // 低
        uint16 _type;
    };
};

template <int N> struct UID
{
    static const int kSize = N;
    uint8 data[kSize];

    static const uint32& _GenUid() {
        static uint32 UIDGen = 0;
        static_assert(sizeof(UIDGen) >= kSize);
        if (++UIDGen >= (1 << kSize*8)) UIDGen = 0;
        return UIDGen;
    }

    UID() { memcpy(data, &_GenUid(), kSize); }
    void Reset(uint32 v) { memcpy(data, &v, kSize); }
    bool operator< (UID const& rhs) { return memcmp(data, rhs.data, kSize) < 0; }
    bool operator== (UID const& rhs) { return memcmp(data, rhs.data, kSize) == 0; }
};