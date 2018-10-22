--[[
* @ Lua脚本系统
* @ brief
    1、目前全局只有一个lua堆栈（见LuaCall.h结尾）以 main.lua 作入口

    2、common目录下是些纯函数式脚本，提供基础功能；其它业务型脚本（如Logic目录中的）均被模块化（注册在Module表中）
        * 同目录中的脚本，加载时即可互相访问（见DoFileDir实现）
        * 不同目录间的脚本，须考虑加载顺序，确保依赖对象在自己之前加载
        * 模块中的变量、函数，尽量定义成本模块内部的，如：this.name, function this.add(a,b)... 

    3、可指定同批次模块(同个目录中的)位于哪个域中（DoFileDir参数二，默认在全局_G表）
        * 减少_G表的元素，将模块分层，有利于提高健壮性

* @ 初始化流程
    1、初始化须与脚本加载分离（初始化可能有各种依赖）

    2、C++调用cppCallMain()作为初始化流程入口，依次调用各模块的初始化函数Init、OnAwake、OnStart
        * Init      仅起服时调用一次，脚本均已加载完毕，初始化流程的首个函数
        * OnAwake   仅起服时调用一次，其它模块Init均已完成
        * OnStart   起服、脚本重载时调用，在OnAwake之后

* @ Notice 重载
    1、Lua重载可理解为再执行一次脚本内容，只不过是让相应"field"指向新值而已
        * 很多写法（如local）会让变量、函数重新生成一份
        * 为避免生成新变量，尽量用 InitGValue、InitThisValue 构造对象
            * lua table是引用语义。若table被另一个模块引用，假设本模块重载生成新的table，但其它模块引用的仍是旧table

    2、函数同table类似，重载时会再生成，所以须避免缓存函数地址（否则重载后仍然调用了旧的）
        * 缓存函数名，mod[func](...)形式调用

    3、【闭包】
        * lua闭包是引用捕获的，包括两个部分：function、upvalue
        * 重载后function部分更替了，upvalue 可能仍引用的旧值

    4、【返回值是函数的】
        * 此时返回的是函数对象，每一次调用生成的函数对象都不同（可能含有不同upvalue）
        * 重载后，之前返回的函数对象就无法更改了……可能带来逻辑bug
        * 避免该方式的广泛使用

* @ author zhoumf
* @ date 2018-10-17
]]--

function Reload(name)
    package.loaded[name] = nil
    require(name)
end

-- _WIN = true --C++层塞入全局变量_WIN
Reload("common/common")
DoFileDir("common", nil, true)

InitGValue("Config", {})
DoFileDir("Config", Config)
DoFileDir("Logic")

-- 初始化流程由外部驱动，不宜直接写在模块加载流程中
function cppCallMain()
    CallAllModFunc("Init")    --仅起服时调用
    CallAllModFunc("OnAwake") --仅起服时调用，其它模块init均已完成
    CallAllModFunc("OnStart") --起服、重载脚本时调用，在OnAwake之后
end
function cppCallReload(step)
    if step == 1 then
        --1、清理旧数据
    elseif step == 2 then
        --2、重载脚本
        CallAllModFunc("OnStart")
    else
        --3、脚本重载完毕
    end
end
