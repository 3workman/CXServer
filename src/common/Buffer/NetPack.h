/***********************************************************************
* @ 业务层使用的网络包
* @ brief
    1、应设计为可替换【网络库】和【协议】的

* @ author zhoumf
* @ date 2016-3-21
************************************************************************/
#pragma once
#include "bytebuffer.h"
#include "../tool/Mempool.h"

enum PacketFromEnum
{
    Packet_From_Client  = 0,
    Packet_From_Game    = 1,
    Packet_From_Battle  = 2,
    Packet_From_Cross   = 3,
};

class NetPack {
    Pool_Obj_Define(NetPack, 4096)
private:
    ByteBuffer  m_buf;
    const uint8 Udp_Flag = 135;
public:
    static const size_t HEADER_SIZE     = sizeof(uint8)+sizeof(uint16)+sizeof(uint32); // packetType & Opcode & reqIdx
    static const size_t TYPE_INDEX      = 0;
    static const size_t OPCODE_INDEX    = 1;
    static const size_t REQ_IDX_INDEX   = 3;

    NetPack(int size = 128 - HEADER_SIZE)
        :m_buf(size + HEADER_SIZE) {
        m_buf.resize(HEADER_SIZE);
        m_buf.wpos(HEADER_SIZE);
        m_buf.rpos(HEADER_SIZE);
        Type(Udp_Flag);
    }
    NetPack(const void* pData, int size)
        :m_buf(size) {
        m_buf.append(pData, size);
        m_buf.rpos(HEADER_SIZE);
    }
    NetPack(const NetPack& other)
        :m_buf(other.m_buf) {
    }
    void Clear() { m_buf.clear(HEADER_SIZE); OpCode(0); Type(Udp_Flag); }
    void ResetHead(const NetPack& other) {
        m_buf.clear();
        m_buf.append(other.m_buf.contents(), HEADER_SIZE);
        m_buf.rpos(HEADER_SIZE);
    }
public: // header
    void    OpCode(uint16 opCode) { m_buf.put(OPCODE_INDEX, opCode); }
    uint16  OpCode() const { return m_buf.show<uint16>(OPCODE_INDEX); }
    void    Type(uint8 packType) { m_buf.put(TYPE_INDEX, packType); }
    uint8   Type() const { return m_buf.show<uint8>(TYPE_INDEX); }
    void    ReqIdx(uint32 idx) { m_buf.put(REQ_IDX_INDEX, idx); }
    uint32  ReqIdx() const { return m_buf.show<uint32>(REQ_IDX_INDEX); }
public:
    uint16 Size() const { return m_buf.size(); }
    uint16 BodySize() const { return m_buf.size() - HEADER_SIZE; }
    const uint8* Buffer() const { return m_buf.contents(); }
    uint64 GetReqKey() { return (uint64(OpCode()) << 32) | ReqIdx(); }

	template<class T> NetPack& operator << (const T& data) {
		m_buf << data;
		return *this;
	}
	template<class T> NetPack& operator >> (T& data) {
		m_buf >> data;
		return *this;
	}
	template<class T> NetPack& WriteStruct(const T& data) {
        m_buf.append(reinterpret_cast<const void*>(&data), sizeof(data));
        return *this;
	}
	template<class T> NetPack& ReadStruct(T& data) {
        m_buf.read(reinterpret_cast<void*>(&data), sizeof(data));
        return *this;
	}

// for logic
public:
    void    WriteBool(bool val) { m_buf.append(val); }
    void    WriteInt8(int8 val) { m_buf.append(val); }
    void    WriteInt16(int16 val) { m_buf.append(val); }
    void    WriteInt32(int32 val) { m_buf.append(val); }
    void    WriteInt64(int64 val) { m_buf.append(val); }
    void    WriteUInt8(uint8 val) { m_buf.append(val); }
    void    WriteUInt16(uint16 val) { m_buf.append(val); }
    void    WriteUInt32(uint32 val) { m_buf.append(val); }
    void    WriteUInt64(uint64 val) { m_buf.append(val); }
    void    WriteFloat(float val) { m_buf.append(val); }
    void    WriteDouble(double val) { m_buf.append(val); }
    void    WriteString(const std::string& val) { m_buf.append(val); }
    void    WriteString(const char* val) { m_buf.append(val); }
    void    WriteBuf(const void* buf, size_t len) { m_buf.append(buf, len); }

    bool    ReadBool() { return m_buf.read<bool>(); }
    int8    ReadInt8() { return m_buf.read<int8>(); }
    int16   ReadInt16() { return m_buf.read<int16>(); }
    int32   ReadInt32() { return m_buf.read<int32>(); }
    int64   ReadInt64() { return m_buf.read<int64>(); }
    uint8   ReadUInt8() { return m_buf.read<uint8>(); }
    uint16  ReadUInt16() { return m_buf.read<uint16>(); }
    uint32  ReadUInt32() { return m_buf.read<uint32>(); }
    uint64  ReadUInt64() { return m_buf.read<uint64>(); }
    float   ReadFloat() { return m_buf.read<float>(); }
    double  ReadDouble() { return m_buf.read<double>(); }
    std::string  ReadString() { 
        std::string str;
        m_buf.read(str);
        //return std::move(str);
        return str; // c++11 右值引用，移动构造
    }

    void   SetPos(int pos, uint32 v){ m_buf.put(HEADER_SIZE + pos, v); }
    uint32 GetPos(int pos) { return m_buf.show<uint32>(HEADER_SIZE + pos); }
};
