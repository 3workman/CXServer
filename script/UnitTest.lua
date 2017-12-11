-- 
-- 单元测试
-- Author: zhoumf
-- Date: 2015-11-30 18:02
--
local this          = Test
local ModuleInfo    = ModuleInfo
local g_ok = true

InitThisValue(this,'bufList',{})

-------------------------------------
-- API
function this.AssertEqual(realVal, expectVal)
	if type(realVal) == 'userdata' then realVal = table.u2t(realVal) end
	if type(expectVal) == 'userdata' then expectVal = table.u2t(expectVal) end
	if this.isEqual(realVal, expectVal) then
		g_ok = true
	else
		g_ok = false
		LogDebug('real value is %s', realVal)
		LogDebug('expect value is %s', expectVal)
		LogError()
	end
end

-- GM后台直接调用，测试某模块代码
-- 自己编写测试文件，名称前缀"test_"
-- 自动顺序调用测试文件中"test_"开头的函数
function this.test(modName, user_or_pid)
	if type(user_or_pid) == 'number' then
		return Thread.syncPidCall(user_or_pid, 'Test', 'TestByPid', modName, user_or_pid)
	end
	local name = 'test_'..modName
	for k,v in pairs(Main[name]) do
		if type(v) == 'function' and string.sub(k,1,5) == 'test_' then
			v(user_or_pid)
			if false == g_ok then break end
		end
	end
	this.FreeBuffer()
	if g_ok then LogDebug('Test OK !') end
end


-------------------------------------
-- 辅助函数
function this.TestByPid(modName, pid)
	local user = LogicUser.findUser(pid)
    if user then
    	this.test(modName, user)
    end
end

function this.isEqual(a, b)
	local ta, tb = type(a), type(b)
	if ta ~= tb then 
		return false 
	end
	if ta == 'table' then 
		return table.t2s(a) == table.t2s(b) 
	end
	return a == b
end

-- 避免测试函数自己释放内存
-- 记录new出的指针，单元测试后统一释放
function this.NewBuffer()
	local buf = ModuleInfo:NewBuffer()
	-- buf:BeginWrite()
	-- buf:BeginRead()
	table.insert(this.bufList, buf)
	return buf
end
function this.FreeBuffer()
	for k,buf in pairs(this.bufList) do
		ModuleInfo:FreeBuffer(buf)
		this.bufList[k] = nil
	end
end