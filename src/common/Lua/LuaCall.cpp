#include "stdafx.h"
#include "LuaCall.h"
#include "tolua.h"

LuaCall* G_Lua = new LuaCall("../script/test.lua");

LuaCall::LuaCall(const char* szFile) : m_szFile(szFile)
{
    ZeroMemoryArray(m_FuncIdx);
    ZeroMemoryArray(m_TableIdx);

    m_pL = luaL_newstate();
    if (!m_pL) {
        LOG_ERROR("create lua state error.");
        return;
    }
    luaL_openlibs(m_pL); //载入Lua基本库，否则脚本里的print内置函数会报错

    tolua::InitLuaReg(m_pL); //载入c++接口

    //DoLuaFile("common.lua") -- in lua
    luabridge::getGlobalNamespace(m_pL).addFunction("DoLuaFile", &LuaCall::DoLuaFile);

    if (m_szFile) DoFile(m_szFile); //载入入口脚本，此脚本内可加载其它所需脚本
}
LuaCall::~LuaCall()
{
    if (m_pL)
    {
        lua_close(m_pL);
        m_pL = NULL;
    }
}
void LuaCall::GC() { lua_gc(m_pL, LUA_GCCOLLECT, 0); }

void LuaCall::PrintStack()
{
	lua_getglobal(m_pL, "debug");
	lua_getfield(m_pL, -1, "traceback");
	if (lua_pcall(m_pL, 0, 1, 0) == 0)
		LOG_DEBUG(lua_tostring(m_pL, -1));
	else
		LOG_ERROR("PrintTraceStack call lua debug.traceback error.");
}

bool LuaCall::Call(const char* szFunc, const char *sig, ...)
{
    va_list vl;
    va_start(vl, sig);
    bool ret = _Call(szFunc, sig, vl);
    va_end(vl);
    return ret;
}
bool LuaCall::_Call(const char* szFunc, const char *sig, va_list vl)
{
	int	nTop = lua_gettop(m_pL);
	int	narg = 0;
	int	nres = 0;

	lua_getglobal(m_pL, szFunc);

	bool bError = false;

	try
	{
		while (*sig)
		{
			switch (*sig++) {
			case 'd':
				lua_pushnumber(m_pL, va_arg(vl, double));
				break;
			case 'i':
				lua_pushnumber(m_pL, va_arg(vl, int));
				break;
			case 's':
				lua_pushstring(m_pL, va_arg(vl, char*));
				break;
            case 'p':
                luabridge::push(m_pL, va_arg(vl, Player*));
                break;
			case '>':
				goto endwhile;
			default:
				LOG_ERROR("invalid option (%c)", *(sig - 1));
				bError = true;
				break;
			}
			++narg;
			luaL_checkstack(m_pL, 1, "too many arguments");
		}
endwhile:
		if (bError == false)
		{
			nres = (int)strlen(sig);
			if (lua_pcall(m_pL, narg, nres, 0))
			{
				LOG_ERROR("error running function `%s': %s",szFunc, lua_tostring(m_pL, -1));
				bError = true;
			}
		}
		if (bError == false) 
		{
			nres = -nres;
			while (*sig && nres < 0)
			{
				switch (*sig++) {
				case 'd':
					if (!lua_isnumber(m_pL, nres))
					{
						LOG_ERROR("wrong result type");
						bError = true;
						break;
					}
					*va_arg(vl, double *) = lua_tonumber(m_pL, nres);
					break;
				case 'i':
					if (!lua_isnumber(m_pL, nres))
					{
						LOG_ERROR("wrong result type");
						bError = true;
						break;
					}
					*va_arg(vl, int *) = (int)lua_tonumber(m_pL, nres);
					break;
				case 's':
					if (!lua_isstring(m_pL, nres))
					{
						const char* szType = lua_typename(m_pL, nres);
						LOG_ERROR("wrong result type");
						LOG_ERROR("wrong result type(%s)(%d)", szType,nres);
						bError = true;
						break;
					}
					strncpy(va_arg(vl, char*), lua_tostring(m_pL, nres), 512);
					break;
				case 'z':
					{
						if (!lua_isstring(m_pL, nres))
						{
							const char* szType = lua_typename(m_pL, nres);
							LOG_ERROR("wrong result type");
							LOG_ERROR("wrong result type(%s)(%d)", szType,nres);
							bError = true;
							break;
						}
						const char* pTmp = lua_tostring(m_pL, nres);
						size_t nLen = strlen(pTmp) + 1;
						if (nLen <= 102400)
						{
							if (nLen >= 20480)
							{
                                LOG_WARN("warning lua read update string too long.(%d)(%s)", nLen,pTmp);
							}
							strncpy(va_arg(vl, char*), pTmp, nLen);
						}
						else
						{
							LOG_ERROR("error lua read update string too long.(%d)(%s)", nLen,pTmp);
						}
					}
					break;
				default:
					LOG_ERROR("invalid option (%c)", *(sig - 1));
					LOG_ERROR("invalid option");
					bError = true;
					break;
				}
				if (bError) break;
				++nres;
			}
		}
	}
	catch(...)
	{
		Call("throw", "s", "C++异常");
		lua_settop(m_pL, nTop);
		LOG_ERROR("lua 调用错误");
	}

	lua_settop(m_pL, nTop);
	return bError == false;
}

bool LuaCall::DoLuaFile(const char* szFile, lua_State* L)
{
/*
#if (!defined(WIN32) || !defined(_DEBUG))
	int nLen = 0;
	if (const char* pBuffer = g_pServerManager->GetLuaBuffer(szFile, nLen))
	{
		if (luaL_loadbuffer(m_pL, pBuffer, nLen, szFile) || lua_pcall(m_pL, 0, 1, 0)) //编译buff中的Lua代码
		{
			LOG_ERROR("do lua file error. `%s': %s", szFile, lua_tostring(m_pL, -1));
			lua_pop(m_pL, 1);
			lua_settop(m_pL, 0);
			return false;
		}
	}
	else
#endif // _DEBUG
*/
	{
		if (luaL_dofile(L, szFile))
		{
			LOG_ERROR("do lua file error. `%s': %s", szFile, lua_tostring(L, -1));
			lua_pop(L, 1);
			lua_settop(L, 0);
			return false;
		}
	}
	return true;	
}

void LuaCall::ReloadFile(const char* szFile /* = NULL */)
{
    if (szFile == NULL) szFile = m_szFile;

    CallReload(1);      //ReloadBegin，清旧脚本数据等
    UnRegLuaCall();

    if (DoFile(szFile) && RegLuaCall()) {
        CallReload(2);  //ReloadEnd，重新调用脚本初始化流程
        GC();
    }
}

int LuaCall::RegLuaTable()
{
    int nCount = (int)RegLuaTable(m_TableIdx[eTPath], "GPathUpdate");

    return nCount;
}
bool LuaCall::RegLuaTable(int& ref, const char* szName)
{
    lua_getglobal(m_pL, szName);

    if (lua_istable(m_pL, -1) == 0) return false;

    ref = luaL_ref(m_pL, LUA_REGISTRYINDEX);

    lua_pop(m_pL, -1);
    lua_settop(m_pL, 0);
    return true;
}
void LuaCall::UnRegLuaTable()
{
    for (auto& it : m_TableIdx) {
        if (it) {
            luaL_unref(m_pL, LUA_REGISTRYINDEX, it);
            it = 0;
        }
    }
}
bool LuaCall::GetTable(ELuaTable eT)
{
    if (m_TableIdx[eT])
    {
        lua_rawgeti(m_pL, LUA_REGISTRYINDEX, m_TableIdx[eT]);
        return true;
    }
    return false;
}

int LuaCall::RegLuaCall()
{
    int nCount = (int)RegLuaCall(m_FuncIdx[eMain], "cppCallMain");
    nCount += (int)RegLuaCall(m_FuncIdx[eRecv], "cppCallRecv");
    nCount += (int)RegLuaCall(m_FuncIdx[eSend], "cppCallSend");
    nCount += (int)RegLuaCall(m_FuncIdx[eExit], "cppCallExit");
    nCount += (int)RegLuaCall(m_FuncIdx[eDestroy], "cppCallDestroy");
    nCount += (int)RegLuaCall(m_FuncIdx[eReload], "cppCallReload");
    nCount += (int)RegLuaCall(m_FuncIdx[ePath], "cppCallPathUpdate");

    return nCount + RegLuaTable();
}
bool LuaCall::RegLuaCall(int& ref, const char* szName)
{
    lua_getglobal(m_pL, szName);

    if (lua_isfunction(m_pL, -1) == 0) return false;

    ref = luaL_ref(m_pL, LUA_REGISTRYINDEX);

    lua_pop(m_pL, -1);
    lua_settop(m_pL, 0);
    return true;
}
void LuaCall::UnRegLuaCall()
{
    for (auto& it : m_FuncIdx) {
        if (it) {
            luaL_unref(m_pL, LUA_REGISTRYINDEX, it);
            it = 0;
        }
    }
    UnRegLuaTable();
}

bool LuaCall::CallMain()
{
	if (!m_FuncIdx[eMain]) return false;

	lua_rawgeti(m_pL, LUA_REGISTRYINDEX, m_FuncIdx[eMain]);

	if (lua_pcall(m_pL, 0, 0, 0))
	{
		LOG_ERROR("Server error running function `%s': %s", "CallMain", lua_tostring(m_pL, -1));
		//PrintStack();
		lua_pop(m_pL, 1);
		lua_settop(m_pL, 0);
		return false;
	}
	return true;
}
bool LuaCall::CallExit(int nExitCode)
{
	if (!m_FuncIdx[eExit]) return false;

	lua_rawgeti(m_pL, LUA_REGISTRYINDEX, m_FuncIdx[eExit]);
	lua_pushinteger(m_pL, nExitCode);

	if (lua_pcall(m_pL, 1, 0, 0))
	{
		LOG_ERROR("Server error running function `%s': %s","CallExit", lua_tostring(m_pL, -1));
		lua_pop(m_pL, 1);
		lua_settop(m_pL, 0);
		return false;
	}
	return true;
}
bool LuaCall::CallDestroy()
{
	if (!m_FuncIdx[eDestroy]) return false;

	lua_rawgeti(m_pL, LUA_REGISTRYINDEX, m_FuncIdx[eDestroy]);

	if (lua_pcall(m_pL, 0, 0, 0))
	{
		LOG_ERROR("Server error running function `%s': %s","CallDestroy", lua_tostring(m_pL, -1));
		lua_pop(m_pL, 1);
		lua_settop(m_pL, 0);
		return false;
	}
	return true;
}
bool LuaCall::CallReload(int nStep)
{
	if (!m_FuncIdx[eReload]) return false;

	lua_rawgeti(m_pL, LUA_REGISTRYINDEX, m_FuncIdx[eReload]);
	lua_pushinteger(m_pL, nStep);

	if (lua_pcall(m_pL, 1, 0, 0))
	{
		LOG_ERROR("Server error running function `%s': %s","CallReload", lua_tostring(m_pL, -1));
		lua_pop(m_pL, 1);
		lua_settop(m_pL, 0);
		return false;
	}
	return true;
}
bool LuaCall::CallPathUpdate()
{
	if (!m_FuncIdx[ePath]) return false;

	lua_rawgeti(m_pL, LUA_REGISTRYINDEX, m_FuncIdx[ePath]);

	if (lua_pcall(m_pL, 0, 0, 0))
	{
		LOG_ERROR("Server error running function `%s': %s","CallPathUpdate", lua_tostring(m_pL, -1));
		lua_pop(m_pL, 1);
		lua_settop(m_pL, 0);
		return false;
	}
	return true;
}
bool LuaCall::CallRecv(NetPack* buf)
{
	if (!m_FuncIdx[eRecv] || !buf) return false;

	lua_rawgeti(m_pL, LUA_REGISTRYINDEX, m_FuncIdx[eRecv]);
    luabridge::push(m_pL, buf);

	if (lua_pcall(m_pL, 1, 0, 0))
	{
		LOG_ERROR("Server error running function `%s': %s","CallRecv", lua_tostring(m_pL, -1));
		lua_pop(m_pL, 1);
		lua_settop(m_pL, 0);
		return false;
	}
	return true;
}
bool LuaCall::CallSend()
{
	if (!m_FuncIdx[eSend]) return false;

	lua_rawgeti(m_pL, LUA_REGISTRYINDEX, m_FuncIdx[eSend]);

	if (lua_pcall(m_pL, 0, 0, 0))
	{
		LOG_ERROR("Server error running function `%s': %s","CallSend", lua_tostring(m_pL, -1));
		lua_pop(m_pL, 1);
		lua_settop(m_pL, 0);
		return false;
	}
	return true;
}