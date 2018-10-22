#include "stdafx.h"
#include "LuaCall.h"
#include "tolua.h"
#include "tool/GameApi.h"

using namespace std;

#define LUA_PATH  "../lua/"
#define LUA_PATH2 "package.path = '../lua/?.lua;'..package.path"

LuaCall* G_Lua = nullptr; //进入main函数后立即赋值

LuaCall::LuaCall(const char* szFile) : m_mainFile(szFile)
{
    ZeroMemoryArray(m_FuncIdx);
    ZeroMemoryArray(m_TableIdx);

    m_pL = luaL_newstate();
    if (!m_pL) {
        LOG_ERROR("create lua state error.");
        return;
    }
    luaL_openlibs(m_pL); //载入Lua基本库，否则脚本里的print内置函数会报错
    luaL_dostring(m_pL, LUA_PATH2);

    tolua::InitLuaReg(m_pL); //载入c++接口

#ifdef WIN32
    lua_pushboolean(m_pL, true);
    lua_setglobal(m_pL, "_WIN");
#endif
    DoFile(m_mainFile); //载入入口脚本，此脚本内可加载其它所需脚本
    RegLuaCall();
}
LuaCall::~LuaCall()
{
    if (m_pL) {
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
		LOG_ERROR("PrintTraceStack call lua debug.traceback error: %s", lua_tostring(m_pL, -1));
    lua_pop(m_pL, 1);
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

    PushLuaVal(szFunc);

	bool bError = false;
    int	 narg = 0;
    int	 nres = 0;
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
	return !bError;
}

AnyTable LuaCall::ReadLuaTable(const char* path)
{
    int n = PushLuaVal(path); //变量值入栈，位于栈顶-1
    auto& ret = ReadLuaTable(n);
    lua_pop(m_pL, 1);
    return ret;
}
AnyTable LuaCall::ReadLuaTable(int tablePlusIdx)
{
    AnyTable ret;
    std::string key, value;

    lua_pushnil(m_pL); //现在的栈：-1 => nil; index => table
    /*
        1、先从栈顶弹出一个 key
        2、从栈指定位置的 table 里取下一对 key-value（相对1中弹出的key而言），先将 key 入栈再将 value 入栈
        3、table 里第一对 key-value 的前面没有数据，所以先用 lua_pushnil() 压入一个 nil 充当初始 key
        4、如果第2步成功则返回非 0 值，否则返回 0，并且不向栈中压入任何值
        *、遍历表时除非明确知道key是string：lua_tostring会改变对应索引位置的key的值，使下一次lua_next无效
    */
    while (lua_next(m_pL, tablePlusIdx))
    {
        //现在的栈：-1 => value; -2 => key; index => table
        switch (lua_type(m_pL, -2)) {
        case LUA_TNUMBER:
            char buf[8]; sprintf(buf, "%d", (int)lua_tonumber(m_pL, -2));
            key = buf;
            break;
        case LUA_TSTRING:
            key = lua_tostring(m_pL, -2);
            break;
        default:
            break;
        }
        switch (lua_type(m_pL, -1)) {
        case LUA_TNUMBER:
            ret[key] = (float)lua_tonumber(m_pL, -1);
            break;
        case LUA_TSTRING:
            value = lua_tostring(m_pL, -1);
            ret[key] = value;
            break;
        case LUA_TTABLE:
            ret[key] = ReadLuaTable(lua_gettop(m_pL));
            break;
        default:
            break;
        }
        //弹出 value，留下原始的 key 作为下一次 lua_next 的参数
        lua_pop(m_pL, 1); //现在的栈：-1 => key; index => table
    }
    //现在的栈：index => table （最后 lua_next 返回 0 的时候它已经把上一次留下的 key 给弹出了）
    //所以栈已经恢复到进入这个函数时的状态
    return ret;
}

int LuaCall::PushLuaVal(const char* path)
{
    int	nTop = lua_gettop(m_pL);
    std::vector<std::string> keys; GameApi::SplitStr(path, keys, '.');
    for (size_t i = 0; i < keys.size(); ++i) {
        if (i == 0) {
            lua_getglobal(m_pL, keys[i].c_str());
        } else {
            lua_getfield(m_pL, -1, keys[i].c_str());
        }
    }
    nTop += 1; //比之调用前，增加一个值入栈
    lua_insert(m_pL, nTop);
    lua_settop(m_pL, nTop);
    return nTop;
}

bool LuaCall::DoFile(const char* szFile)
{
    string name(LUA_PATH); name.append(szFile);

    if (luaL_dofile(m_pL, name.c_str()))
    {
        LOG_ERROR("do lua file error. `%s': %s", szFile, lua_tostring(m_pL, -1));
        lua_pop(m_pL, 1);
        return false;
    }
	return true;
}
void LuaCall::ReloadFile(const char* szFile /* = NULL */)
{
    if (szFile == NULL) szFile = m_mainFile;

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

    if (!lua_istable(m_pL, -1)) {
        lua_pop(m_pL, -1);
        return false;
    }
    ref = luaL_ref(m_pL, LUA_REGISTRYINDEX); //将栈顶元素放到t对应的table中
    lua_pop(m_pL, -1);
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

    if (!lua_isfunction(m_pL, -1)) {
        lua_pop(m_pL, -1);
        return false;
    }
    ref = luaL_ref(m_pL, LUA_REGISTRYINDEX);
    lua_pop(m_pL, -1);
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
        LOG_ERROR("Server error running function `%s': %s", "CallSend", lua_tostring(m_pL, -1));
        lua_pop(m_pL, 1);
        return false;
    }
    return true;
}