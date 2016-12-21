/***********************************************************************
* @ ���ʽID
* @ brief
	1�������������ϣ�����Щ���ݣ����ΨһID������������õ���������
	2����Mempool.h�����ڴ���еĶ��󣬻��������ã���ͨ��m_index���ⲿ����ľ�idx���ܶ�λ���¶���

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
        uint32 _index; // ��
        union {
            uint32 _type;
            struct {
            uint8  _mainTyp;
            uint8  _typ1;
            uint8  _typ2;
            uint8  _typ3; // ��
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
        uint16 _index; // ��
        uint16 _type;
    };
};