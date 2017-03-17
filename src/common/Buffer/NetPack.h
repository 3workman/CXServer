#pragma once
#include "bytebuffer.h"
#include "..\tool\Mempool.h"


enum PacketTypeEnum
{
    PACKET_TYPE_CLIENT = 0,

    PACKET_TYPE_SERVER_LOGIN = 19,
    PACKET_TYPE_SERVER_GAME = 23,
    PACKET_TYPE_SERVER_GATEWAY = 35,
    PACKET_TYPE_SERVER_DBPROXY = 38,
    PACKET_TYPE_SERVER_CENTER = 50,
    PACKET_TYPE_SERVER_EVENT = 60,
    PACKET_TYPE_SERVER_WEB = 77,

    _PACKET_TYPE_END
};


class NetPack
{
private:
    Pool_Obj_Define(NetPack, 1024)

    static const size_t HEADER_SIZE     = sizeof(uint16) + sizeof(char); // Opcode & packetType
    static const size_t OPCODE_INDEX    = 0;
    static const size_t TYPE_INDEX      = 2;

private:
    ByteBuffer  m_buf;

public:
    static size_t GetHeaderSize() { return HEADER_SIZE; }

    NetPack(uint16 opCode, int size = 256)
        :m_buf(size + HEADER_SIZE) {
        m_buf.resize(HEADER_SIZE);
        m_buf.rpos(HEADER_SIZE);
        SetOpCode(opCode);
    }
    NetPack(char type, uint16 opCode, uint16 bodySize)
        :m_buf(bodySize + HEADER_SIZE) {
        m_buf.resize(HEADER_SIZE);
        m_buf.rpos(HEADER_SIZE);
        SetPacketType(type);
        SetOpCode(opCode);
    }
    NetPack(const NetPack& other)
        :m_buf(other.m_buf) {
    }
    void Clear() { 
        m_buf.clear();
        m_buf.resize(HEADER_SIZE);
        m_buf.rpos(HEADER_SIZE);
    }
public:
    void SetOpCode(uint16 opCode) { m_buf.put(OPCODE_INDEX, opCode); }
    uint16 GetOpcode() const { return m_buf.show<uint16>(OPCODE_INDEX); }

    void SetPacketType(char packType) { m_buf.put(TYPE_INDEX, packType); }
    char GetPacketType() const { return m_buf.show<char>(TYPE_INDEX); }

    void Resize(size_t size) {
        m_buf.resize(size);
        m_buf.rpos(HEADER_SIZE);
    }
    uint16 BodyBytes() const { return m_buf.size() - HEADER_SIZE; }

	template<class T> NetPack& operator << (const T& data) {
		m_buf << data;
		return *this;
	}
	template<class T> NetPack& operator >> (T& data) {
		m_buf >> data;
		return *this;
	}
	template<class T> NetPack& AppendStruct(const T& data) {
        m_buf.append(reinterpret_cast<const uint8*>(&data), sizeof(data));
        return *this;
	}
	template<class T> NetPack& ReadStruct(T& data) {
        m_buf.read(reinterpret_cast<uint8*>(&data), sizeof(data));
        return *this;
	}
	NetPack& Append(const void* data, size_t len) {
        m_buf.append(reinterpret_cast<const uint8*>(data), len);
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

    // lua 那边单线程使用的，所以用静态
    const char* GetString() { static std::string str; m_buf >> str; return str.c_str(); }
    void SetString(const char* pStr) { m_buf << pStr; }
};
