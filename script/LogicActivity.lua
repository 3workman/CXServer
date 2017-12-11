local this 			= LogicZombie
local Logic 		= Logic
local Time 			= Time
local LogicUser		= LogicUser
local LogicChat 	= LogicChat
local LogicZombieDB = LogicZombieDB

local TIME_ACT = {19, 50}		-- 活动开始时间(时:分)
local TIME_STATE = {10, 5}		-- 各个状态的持续时间(分钟)
local StateEndFuns = {} 		-- 各个状态的结束回调

InitThisValue(this,'STATE_NOW',0)			-- 活动当前状态，0表示不在活动期
InitThisValue(this,'STATE_NOW_END_TIME',0)	-- 当前状态的结束时间点

local Combat_State = 2			-- 哪个状态用于战斗

----------------------------------------------------------------------
-- 框架区
----------------------------------------------------------------------
function this.initLocal() -- 起服、重载脚本时调用
	-- 状态函数绑定
	StateEndFuns[0] = this.OnNoneStateEnd		-- 非活动期结束，进入活动预备期(TIME_STATE配置的第一项)
	StateEndFuns[1] = this.OnPreStartStateEnd	-- 预备期结束，进入正式战斗期
	StateEndFuns[2] = this.OnOver				-- 活动期结束，重新进入非活动期
end
function this.onReady()
	local list = Logic.getServerIdList()
	for _,serverId in pairs(list) do
		Logic.setServerId(serverId)
		this.reset_to_none()
		AddThisTimer(this, 'update', 20*1000)
	end
end


function this.get_state()
	return this.STATE_NOW, this.STATE_NOW_END_TIME
end
function this.set_state(state, endTime)
	this.STATE_NOW = state
	this.STATE_NOW_END_TIME = endTime
end

-- 检查当前阶段是否结束，结束触发事件并切换下一阶段
function this.update()
	local curSec = Time.time()
	local state, endSec = this.get_state()
	if curSec >= endSec then
		local nextState = state + 1
	    if (nextState > #TIME_STATE) then
	        nextState = 0
	    end

        local stateEndFun = StateEndFuns[state];
        if stateEndFun ~= nil then
            stateEndFun(curSec)
        end

        if (nextState == 0) then
            -- 切换到无活动阶段
            this.reset_to_none(curSec)
        else
            -- 正常切换阶段
            this.set_state(nextState, curSec+TIME_STATE[nextState]*60)
        end

        state = this.get_state() -- 取新的状态阶段
	end
end

-- 计算下次活动时刻(当前时刻, {小时, 分})
function this.calc_next_start_time(curSec, dayTime)
	-- 现在几点
	local hour = tonumber(os.date("%H", curSec));
	-- 现在这个小时经过几秒
	local hourPassSec = curSec % 3600;
	-- 今天经过多少秒
	local dayRunSec = hour * 3600 + hourPassSec
	-- 今天的起始时刻
	local dayStart = curSec - dayRunSec

	-- 今天的活动开启的秒数
	local actStart = dayTime[1] * 3600 + dayTime[2] * 60

	if dayRunSec < actStart then -- 活动尚未开始
		return dayStart + actStart
	else  -- 活动已经开启过了，算下一天的
		return dayStart + actStart + 24*3600
	end
end
function this.reset_to_none(curSec)
	curSec = curSec or Time.time()
	local nextStartTime = this.calc_next_start_time(curSec, TIME_ACT)
	this.set_state(0, nextStartTime)

	this.ClearOneRoundData() -- 清活动数据
end
function this.reset_to_next()
	local state = this.get_state()
	local nextState = state + 1
    if (nextState > #TIME_STATE) then
        nextState = 0
    end

    local curSec = Time.time()
    local stateEndFun = StateEndFuns[state];
    if stateEndFun ~= nil then
        stateEndFun(curSec)
    end

    if (nextState == 0) then
        -- 切换到无活动阶段
        this.reset_to_none(curSec)
    else
        -- 正常切换阶段
        this.set_state(nextState, curSec+TIME_STATE[nextState]*60)
    end
end

----------------------------------------------------------------------
-- 逻辑区：示例
----------------------------------------------------------------------
local AD_PRESTART1 		= ""
local AD_PRESTART2		= ""
local AD_START 			= ""
local AD_STOP_MONSTER_WIN	= ""

function this.OnNoneStateEnd(curSec) -- 活动开启，进入预备期
	LogicChat.noticeWorld(nil, 0, AD_PRESTART1)

	AddThisTimer(this, 'NoticeWorld', 300*1000)

	-- 领主累计天数增加
	local lordPid = this.GetCurLordPid()
	local userLord = LogicUser.seekUser(lordPid)
	if userLord then
		LogicZombieDB.AddLordDayCnt(userLord, 1)
	end
end
function this.OnPreStartStateEnd(curSec) -- 正式开打
	LogicZombieDB.ClearRankAndDB('zombieOneRoundRank')
	LogicChat.noticeWorld(nil, 0, AD_START)
end
function this.OnOver(curSec) -- 活动到期结束
	LogicChat.noticeWorld(nil, 0, AD_STOP_MONSTER_WIN)
	this.RewardOneRound()
	this.ClearOneRoundData()
end

function this.NoticeWorld()
	LogicChat.noticeWorld(nil, 0, AD_PRESTART2)
	DelThisTimer(this, 'NoticeWorld')
end
function this.ClearOneRoundData() -- 清活动数据
	ClearLuaTable(g_supportPids)
	ClearLuaTable(g_have_buff_pids)
	ClearLuaTable(g_have_buff_armys)
	ClearLuaTable(g_add_buff_times)
	ClearLuaTable(g_pid_to_dieTime)
	g_add_monster_cnt = 0
end