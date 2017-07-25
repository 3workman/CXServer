#pragma once
#include "Lua/tolua.h"
#include "public.h"

void TestLua1(lua_State *L)
{
    //luaopen_base(L);    
    //luaopen_io(L);                //�滻�ϰ汾��lua_iolibopen(L);    
    //luaopen_string(L);            //�滻�ϰ汾��lua_strlibopen(L);    
    //luaopen_math(L);              //�滻�ϰ汾��lua_mathlibopen(L);    
    //luaopen_debug(L);             //�滻�ϰ汾��lua_dblibopen(L);    
    /*����test.lua�ļ�������*/
    luaL_dofile(L, "../lua/test.lua");

    getchar();
}

static int average(lua_State *L)
{
    /* �õ��������� */
    int n = lua_gettop(L);
    double sum = 0;

    for (int i = 1; i <= n; i++) sum += lua_tonumber(L, i);

    /* ѹ��ƽ��ֵ */
    lua_pushnumber(L, sum / n);
    /* ѹ��� */
    lua_pushnumber(L, sum);
    /* ���ط���ֵ�ĸ��� */
    return 2;
}
void TestLua2(lua_State *L)
{
    /* ע�ắ�� */
    lua_register(L, "average", average);
    /* ���нű� */
    luaL_dofile(L, "../lua/test.lua");

    /* ��ͣ */
    printf("Press enter to exit��");
    getchar();
}

void TestLua3(lua_State *L)
{
    //2.����Lua�ļ�  
    int bRet = luaL_loadfile(L, "../lua/test.lua");
    if (bRet) {
        cout << "load file error" << endl;
        return;
    }

    //3.����Lua�ļ�  
    bRet = lua_pcall(L, 0, 0, 0);
    if (bRet) {
        cout << "pcall error" << endl;
        return;
    }

    //4.��ȡ����  
    lua_getglobal(L, "str");
    string str = lua_tostring(L, -1);
    cout << "str = " << str.c_str() << endl;        //str = I am so cool~  

                                                    //5.��ȡtable  
    lua_getglobal(L, "tbl");
    lua_getfield(L, -1, "name");
    str = lua_tostring(L, -1);
    cout << "tbl:name = " << str.c_str() << endl; //tbl:name = shun  

                                                  //6.��ȡ����  
    lua_getglobal(L, "add");        // ��ȡ������ѹ��ջ��  
    lua_pushnumber(L, 10);          // ѹ���һ������  
    lua_pushnumber(L, 20);          // ѹ��ڶ�������  
    int iRet = lua_pcall(L, 2, 1, 0);// ���ú�������������Ժ󣬻Ὣ����ֵѹ��ջ�У�2��ʾ����������1��ʾ���ؽ��������  
    if (iRet) {
        const char *pErrorMsg = lua_tostring(L, -1);
        cout << pErrorMsg << endl;
        return;
    }
    if (lua_isnumber(L, -1)) {
        double fValue = lua_tonumber(L, -1);
        cout << "Result is " << fValue << endl;
    }

    //���ˣ�ջ�е�����ǣ�  
    //=================== ջ�� ===================   
    //  ����  ����      ֵ  
    //   4   int��      30   
    //   3   string��   shun   
    //   2   table:     tbl  
    //   1   string:    I am so cool~  
    //=================== ջ�� ===================   
    getchar();
}

void TestLuaCall(lua_State *L, uint32 idx)
{
    //����Lua�ļ�  
    int bRet = luaL_loadfile(L, "../lua/test.lua");
    if (bRet) {
        cout << "load file error" << endl;
        return;
    }
    //����Lua�ļ�  
    //bRet = lua_pcall(L, 0, 0, 0);
    //if (bRet) {
    //    cout << "pcall error" << endl;
    //    return;
    //}

    lua_getglobal(L, "rpc_client_test");    // ��ȡ������ѹ��ջ��
    lua_pushnumber(L, idx);          // ѹ���һ������  
    int iRet = lua_pcall(L, 1, 0, 0);
    if (iRet) {
        const char *pErrorMsg = lua_tostring(L, -1);
        cout << pErrorMsg << endl;
        return;
    }
}
