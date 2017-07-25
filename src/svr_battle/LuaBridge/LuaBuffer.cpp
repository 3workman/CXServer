#include "stdafx.h"
#include "Lua/tolua.h"
#include "Buffer/bytebuffer.h"
#include "Buffer/NetPack.h"

namespace tolua {

ToLua(ByteBuffer)
{
    getGlobalNamespace(L)
        .beginClass<ByteBuffer>("ByteBuffer")
            .addConstructor<void(*)(size_t), RefCountedPtr<ByteBuffer>>()
            .addFunction("ReadString", static_cast<string(ByteBuffer::*)()>(&ByteBuffer::read))
            .addFunction("WriteString", static_cast<void(ByteBuffer::*)(const char*)>(&ByteBuffer::append))
            .addFunction("clear", &ByteBuffer::clear)
            .addFunction("size", &ByteBuffer::size)
        .endClass()
        ;
}

ToLua(NetPack)
{
    getGlobalNamespace(L)
        .beginClass<NetPack>("NetPack")
            .addConstructor<void(*)(int), RefCountedPtr<NetPack>>()
            .addProperty("OpCode", &NetPack::OpCode, &NetPack::OpCode)
            .addFunction("ReadString", &NetPack::ReadString)
            .addFunction("WriteString", static_cast<void(NetPack::*)(const char*)>(&NetPack::WriteString))
        .endClass()
        ;
}

}