-- 
-- Author: zhoumf
-- Date: 2015-12-18 15:20
--
local this            = LogicRoom
local ShareData       = ShareData
local syncPidCall     = Thread.syncPidCall
local getTableCount   = getTableCount


-------------------------------------
-- 框架接口
InitThisValue(this,'data',{})
InitThisValue(this,'__index',this)


-------------------------------------
-- API
-- onReady()中调，否则ShareData未初始化
function this.new(modname, maxUserCnt)
    if this.data[modname] ~= nil then
        return this.data[modname]
    else
        this.data[modname] = {}
        local KEY_STR_1 = modname..'_room_list'
        this.data[modname][1] = ShareData.get( KEY_STR_1 ) or ShareData.create( KEY_STR_1 )
    end

    local room = this.data[modname][1]
    room.g_room_user_max = maxUserCnt or 10    -- 同地图人数限制

    room.g_room_list = {}    -- { [roomIdx]= room }     -- room { [pid] = data }

    room.g_user_roomIdx = {}     -- { [pid] = roomIdx }

    setmetatable(this.data[modname], this)  -- lua中无法设置userdata的metatable，蛋疼
    return this.data[modname]
end

function this:create_room(pid, data)
    local thisdata = self[1]
    local newIdx = #thisdata.g_room_list+1
    thisdata.g_room_list[newIdx] = { [pid] = data }
    thisdata.g_user_roomIdx[pid] = newIdx
end
function this:join_room(pid, data, otherPid)
    local thisdata = self[1]
    local roomIdx = thisdata.g_user_roomIdx[otherPid]
    if roomIdx == nil then return nil end
    local room = thisdata.g_room_list[roomIdx]
    if room == nil or room[pid] then -- 已在其中
        return nil
    end
    if getTableCount(room) >= thisdata.g_room_user_max then -- 满员
        return nil 
    end
    room[pid] = data -- 加入
    thisdata.g_user_roomIdx[pid] = roomIdx
    return true
end
function this:exit_room(pid)
    local thisdata = self[1]
    local roomIdx = thisdata.g_user_roomIdx[pid]
    if roomIdx == nil then return nil end
    local room = thisdata.g_room_list[roomIdx]
    if room then 
        room[pid] = nil
        thisdata.g_user_roomIdx[pid] = nil
        if getTableCount(room) == 0 then
            thisdata.g_room_list[roomIdx] = nil
        end
    end
end
function this:get_user_data(pid)
    local thisdata = self[1]
    local roomIdx = thisdata.g_user_roomIdx[pid]
    if roomIdx == nil then return nil end
    local room = thisdata.g_room_list[roomIdx]
    if room then return room[pid] end
end
function this:set_user_data(pid, data)    -- 若data是table结构，可直接更改里面的k-v值
    local thisdata = self[1]
    local roomIdx = thisdata.g_user_roomIdx[pid]
    if roomIdx == nil then return nil end
    local room = thisdata.g_room_list[roomIdx]
    if room then room[pid] = data end
end
function this:get_room_info(pid)
    local thisdata = self[1]
    local roomIdx = thisdata.g_user_roomIdx[pid]
    if roomIdx == nil then return nil end
    local room = thisdata.g_room_list[roomIdx]
    return room
end
function this:for_each(pid, func)
    local thisdata = self[1]
    local roomIdx = thisdata.g_user_roomIdx[pid]
    if roomIdx == nil then return nil end
    local room = thisdata.g_room_list[roomIdx]
    if room then
        for _pid,_data in pairs(room) do
            func(_pid, _data)
        end
    end
end