function InitGValue(k,v)
    if _G[k] == nil then
        _G[k] = v
    end
    return _G[k]
end
function InitThisValue(mod,k,v)
    if mod[k] == nil then
        mod[k] = v
    end
    return mod[k]
end

function GetFileName(str) --获取文件名
    if _WIN then
        str = str:match(".+\\([^\\]*%.%w+)$") or str
    else
        str = str:match(".+/([^/]*%.%w+)$") or str
    end
    return str
end
function GetFileSuffix(str) --获取扩展名
    return str:match(".+%.(%w+)$")
end
function DelFileSuffix(str) --去除扩展名
    local idx = str:match(".+()%.%w+$")
    if idx then str = str:sub(1, idx-1) end
    return str
end

-----------------------------------------------------------
-- 加载lua模块
InitGValue("Module", {})

local function newModule(name, baseMod)
    if Module[name] == nil then
        Module[name] = {}
        baseMod = baseMod or _G
        baseMod[name] = Module[name]
    end
    return Module[name]
end

local function scanDir(dir) --递归子目录
    local file, ret = nil, {}
    if _WIN then
        file = io.popen('dir "../lua/'..dir..'\\*.lua" /b/s') --Windows
    else
        file = io.popen('find "../lua/'..dir..'" -name "*.lua"') --Linux
    end
    for name in file:lines() do
        ret[#ret+1] = DelFileSuffix(GetFileName(name))
    end
    file:close()
    return ret
end

function DoFileDir(dir, baseMod, noNewMod) --同目录模块可互相访问（先构建的Module）【不同目录的须考虑先后顺序】
    local files = scanDir(dir)
    if not noNewMod then
        for _,v in ipairs(files) do newModule(v, baseMod) end
    end
    for _,v in ipairs(files) do
        -- print(string.format('Reload-------%s/%s',dir,v))
        Reload(string.format('%s/%s',dir,v))
    end
end

function CallAllModFunc(func, ...)
    for _,mod in pairs(Module) do
        if mod[func] then 
            mod[func](...)
        end
    end
end

-----------------------------------------------------------
-- 
