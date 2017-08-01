/***********************************************************************
* @ ��Lua����
* @ brief
    1��Lua/C++ ��Ľ������ѽӿڲ������ݵķ�ʽ����ֹ����Է����ɵ�����

    2�����ݶ���ָ����ű� ---- luabridge::push(m_pL, pChar)

    3����ȡ�ű����ص�ָ�� ---- NetPack* buf = luabridge::Userdata::get<NetPack>(L, 1, false);
        * �� Lua ��ָ����պܸߣ����� Lua ���ɵģ����������ɺ��ߴ���C++ ����ʱ����׼��gc����ʲô��

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
    bool		DoFile(const char* szFile); //��ģ���ʼ��ʱ����һ�νű�����
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
    Notice������Ҫ�Ļ����ɷ�Ϊ����ҵ��Ԫ��ÿ��Ԫһ��lua_State
    extern LuaCall* G_LuaPlayer;
    extern LuaCall* G_LuaActivity;
*/
extern LuaCall* G_Lua;