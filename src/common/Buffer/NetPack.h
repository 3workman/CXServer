/***********************************************************************
* @ 业务层使用的网络包
* @ brief
    1、应设计为可替换【网络库】和【协议】的

* @ author zhoumf
* @ date 2016-3-21
************************************************************************/
#pragma once
#include "bytebuffer.h"
#include "tool/Mempool.h"
#include "tool/UnionID.h"
#include "flatbuffers/flatbuffers.h"

template<typename T> using FlatVector = std::vector<flatbuffers::Offset<T>>;

class NetPack : public ByteBuffer
{
    Pool_Obj_Define(NetPack, 4096)
public:
    static const size_t HEADER_SIZE     = sizeof(uint8)+sizeof(uint16)+sizeof(uint32); // packetType & Opcode & reqIdx
    static const size_t TYPE_INDEX      = 0;
    static const size_t OPCODE_INDEX    = 1;
    static const size_t REQ_IDX_INDEX   = 3;

    static const uint8 TYPE_TCP = 135;
    static const uint8 TYPE_UDP = 136;
    static const uint8 TYPE_UNRELIABLE = 137;

    NetPack(int size = 128 - HEADER_SIZE) : ByteBuffer(size + HEADER_SIZE) {
        resize(HEADER_SIZE);
        Type(TYPE_TCP);
    }
    NetPack(const void* pData, int size) : ByteBuffer(size) {
        append(pData, size);
        rpos(HEADER_SIZE);
    }
    NetPack(const NetPack& other) : ByteBuffer(other) {}

    void Clear() { clear(HEADER_SIZE); OpCode(0); Type(TYPE_TCP); }
    void ResetHead(const NetPack& other) {
        clear();
        append(other.contents(), HEADER_SIZE);
        rpos(HEADER_SIZE);
    }
public: // header
    static  uint16 GetOpCode(const void* pMsg) { return *(uint16*)((char*)pMsg + OPCODE_INDEX); }
    void    OpCode(uint16 opCode) { put(OPCODE_INDEX, opCode); }
    uint16  OpCode() const { return show<uint16>(OPCODE_INDEX); }
    void    Type(uint8 packType) { put(TYPE_INDEX, packType); }
    uint8   Type() const { return show<uint8>(TYPE_INDEX); }
    void    ReqIdx(uint32 idx) { put(REQ_IDX_INDEX, idx); }
    uint32  ReqIdx() const { return show<uint32>(REQ_IDX_INDEX); }
public:
    uint16 BodySize() const { return (uint16)(size() - HEADER_SIZE); }
    const uint8* Body() const { return contents() + HEADER_SIZE; }
    uint64 GetReqKey() { return (uint64(OpCode()) << 32) | ReqIdx(); }

// for logic
public:
    void    WriteBool(bool val) { append(val); }
    void    WriteInt8(int8 val) { append(val); }
    void    WriteInt16(int16 val) { append(val); }
    void    WriteInt32(int32 val) { append(val); }
    void    WriteInt64(int64 val) { append(val); }
    void    WriteUInt8(uint8 val) { append(val); }
    void    WriteUInt16(uint16 val) { append(val); }
    void    WriteUInt32(uint32 val) { append(val); }
    void    WriteUInt64(uint64 val) { append(val); }
    void    WriteFloat(float val) { append(val); }
    void    WriteDouble(double val) { append(val); }
    void    WriteString(const std::string& val) { append(val); }
    void    WriteString(const char* val) { append(val); }
    void    WriteBuf(const void* buf, size_t len) { append(buf, len); }
    void    WriteUuid(const UID<3>& uid) { append(uid.data, uid.kSize); }

    void    WriteStringZipped(const std::string& str);

    bool    ReadBool() { return read<bool>(); }
    int8    ReadInt8() { return read<int8>(); }
    int16   ReadInt16() { return read<int16>(); }
    int32   ReadInt32() { return read<int32>(); }
    int64   ReadInt64() { return read<int64>(); }
    uint8   ReadUInt8() { return read<uint8>(); }
    uint16  ReadUInt16() { return read<uint16>(); }
    uint32  ReadUInt32() { return read<uint32>(); }
    uint64  ReadUInt64() { return read<uint64>(); }
    float   ReadFloat() { return read<float>(); }
    double  ReadDouble() { return read<double>(); }
    std::string  ReadString() { 
        return read<std::string>();
        //return std::move(str);
        //return str; // c++11 右值引用，移动构造
    }

    UID<3>  ReadUuid() {
        UID<3> ret;
        read(ret.data, ret.kSize);
        return ret;
    }

    void Absorb(flatbuffers::FlatBufferBuilder& builder) {
        if (builder.GetSize()) {
            WriteBuf(builder.GetBufferPointer(), builder.GetSize());
            builder.Clear();
        }
    }

    void   SetPos(int pos, uint32 v){ put(HEADER_SIZE + pos, v); }
    uint32 GetPos(int pos) { return show<uint32>(HEADER_SIZE + pos); }
};
