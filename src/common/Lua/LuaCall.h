/***********************************************************************
* @ 与Lua交互
* @ brief
    1、Lua/C++ 间的交互，已接口参数传递的方式，禁止保存对方生成的数据

    2、传递对象指针给脚本 ---- luabridge::push(m_pL, pChar)

    3、获取脚本返回的指针 ---- NetPack* buf = luabridge::Userdata::get<NetPack>(L, 1, false);
        * 从 Lua 拿指针风险很高，它是 Lua 生成的，生命周期由后者处理，C++ 调用时保不准被gc或是什么的

* @ author zhoumf
* @ date 2017-7-24
************************************************************************/
#pragma once

class Player;
class NetPack;
struct lua_State;
class LuaCall {
public:
    enum ELuaCall
    {
        eMain,
        eRecv,
        eSend,
        eExit,
        eDestroy,
        eReload,
        ePath,
        eMax
    };
    enum ELuaTable
    {
        eTPath,
        eTMax
    };
private:
    lua_State*	m_pL;
    int			m_FuncIdx[eMax];
    int			m_TableIdx[eTMax];
    const char* m_szFile;

public:
    ~LuaCall();
    LuaCall(const char* szFile);
    lua_State*	L() { return m_pL; }
    void		GC();
    bool		Call(const char* szFunc, const char *sig, ...);
    bool		_Call(const char* szFunc, const char *sig, va_list vl);
    bool		DoFile(const char* szFile); //各模块初始化时载入一次脚本即可
    void        ReloadFile(const char* szFile = NULL);
    void		PrintStack();

// for lua logic
    int		    RegLuaCall();
    bool		RegLuaCall(int& ref, const char* szName);
    void		UnRegLuaCall();
    int 		RegLuaTable();
    bool		RegLuaTable(int& ref, const char* szName);
    void		UnRegLuaTable();
    bool		GetTable(ELuaTable eT);

    bool		CallMain();
    bool		CallExit(int nExitCode);
    bool		CallDestroy();
    bool		CallReload(int nStep);
    bool		CallPathUpdate();
    bool		CallRecv(NetPack* buf);
    bool		CallSend();
};
/*
    Notice：有需要的话，可分为几大业务单元，每单元一个lua_State
    extern LuaCall* G_LuaPlayer;
    extern LuaCall* G_LuaActivity;
*/
extern LuaCall* G_Lua;