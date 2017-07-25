#pragma once
extern "C"
{
#include "lua.h"  
#include "lualib.h"  
#include "lauxlib.h"  
};
#include "LuaBridge.h"
#include "RefCountedPtr.h"

#define ToLua_Class\
    ToLua(ByteBuffer)\
    ToLua(NetPack)\
    ToLua(Player)\

#undef ToLua
#define ToLua(typ) void tolua_##typ(lua_State* L);

namespace tolua {
using namespace std;
using namespace luabridge;
ToLua_Class
inline void InitLuaReg(lua_State* L) {
#undef ToLua
#define ToLua(typ) tolua_##typ(L);
    ToLua_Class
}
}

#undef ToLua
#define ToLua(typ) void tolua_##typ(lua_State* L)