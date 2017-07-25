#pragma once
#include "Lua/tolua.h"
#include "public.h"

void TestLua1(lua_State *L)
{
    //luaopen_base(L);    
    //luaopen_io(L);                //替换老版本的lua_iolibopen(L);    
    //luaopen_string(L);            //替换老版本的lua_strlibopen(L);    
    //luaopen_math(L);              //替换老版本的lua_mathlibopen(L);    
    //luaopen_debug(L);             //替换老版本的lua_dblibopen(L);    
    /*加载test.lua文件并运行*/
    luaL_dofile(L, "../lua/test.lua");

    getchar();
}

static int average(lua_State *L)
{
    /* 得到参数个数 */
    int n = lua_gettop(L);
    double sum = 0;

    for (int i = 1; i <= n; i++) sum += lua_tonumber(L, i);

    /* 压入平均值 */
    lua_pushnumber(L, sum / n);
    /* 压入和 */
    lua_pushnumber(L, sum);
    /* 返回返回值的个数 */
    return 2;
}
void TestLua2(lua_State *L)
{
    /* 注册函数 */
    lua_register(L, "average", average);
    /* 运行脚本 */
    luaL_dofile(L, "../lua/test.lua");

    /* 暂停 */
    printf("Press enter to exit…");
    getchar();
}

void TestLua3(lua_State *L)
{
    //2.加载Lua文件  
    int bRet = luaL_loadfile(L, "../lua/test.lua");
    if (bRet) {
        cout << "load file error" << endl;
        return;
    }

    //3.运行Lua文件  
    bRet = lua_pcall(L, 0, 0, 0);
    if (bRet) {
        cout << "pcall error" << endl;
        return;
    }

    //4.读取变量  
    lua_getglobal(L, "str");
    string str = lua_tostring(L, -1);
    cout << "str = " << str.c_str() << endl;        //str = I am so cool~  

                                                    //5.读取table  
    lua_getglobal(L, "tbl");
    lua_getfield(L, -1, "name");
    str = lua_tostring(L, -1);
    cout << "tbl:name = " << str.c_str() << endl; //tbl:name = shun  

                                                  //6.读取函数  
    lua_getglobal(L, "add");        // 获取函数，压入栈中  
    lua_pushnumber(L, 10);          // 压入第一个参数  
    lua_pushnumber(L, 20);          // 压入第二个参数  
    int iRet = lua_pcall(L, 2, 1, 0);// 调用函数，调用完成以后，会将返回值压入栈中，2表示参数个数，1表示返回结果个数。  
    if (iRet) {
        const char *pErrorMsg = lua_tostring(L, -1);
        cout << pErrorMsg << endl;
        return;
    }
    if (lua_isnumber(L, -1)) {
        double fValue = lua_tonumber(L, -1);
        cout << "Result is " << fValue << endl;
    }

    //至此，栈中的情况是：  
    //=================== 栈顶 ===================   
    //  索引  类型      值  
    //   4   int：      30   
    //   3   string：   shun   
    //   2   table:     tbl  
    //   1   string:    I am so cool~  
    //=================== 栈底 ===================   
    getchar();
}

void TestLuaCall(lua_State *L, uint32 idx)
{
    //加载Lua文件  
    int bRet = luaL_loadfile(L, "../lua/test.lua");
    if (bRet) {
        cout << "load file error" << endl;
        return;
    }
    //运行Lua文件  
    //bRet = lua_pcall(L, 0, 0, 0);
    //if (bRet) {
    //    cout << "pcall error" << endl;
    //    return;
    //}

    lua_getglobal(L, "rpc_client_test");    // 获取函数，压入栈中
    lua_pushnumber(L, idx);          // 压入第一个参数  
    int iRet = lua_pcall(L, 1, 0, 0);
    if (iRet) {
        const char *pErrorMsg = lua_tostring(L, -1);
        cout << pErrorMsg << endl;
        return;
    }
}
