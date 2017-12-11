-- 
-- Author: zhoumf
-- Date: 2015-12-15 14:20
--
local this            = LogicMap
local ShareData       = ShareData
local syncPidCall     = Thread.syncPidCall


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
        local KEY_STR_1 = modname..'_map_list'
        this.data[modname][1] = ShareData.get( KEY_STR_1 ) or ShareData.create( KEY_STR_1 )
    end

    local map = this.data[modname][1]
    map.g_map_user_max = maxUserCnt or 10    -- 同地图人数限制
    
    map.g_map_list = {}    -- { [mapIdx]= oneMap }     -- oneMap { [pid] = data }

    -- { [1]={mapIdx1, mapIdx2}, [2]={mapIdx3, mapIdx4} } 数组下标表示：此地图空余人数
    map.g_free_idx = {}
    for i = 1, map.g_map_user_max do
        map.g_free_idx[i] = {}
    end
    
    map.g_user_mapIdx = {}     -- { [mypid] = mapIdx }

    setmetatable(this.data[modname], this)  -- lua中无法设置userdata的metatable，蛋疼
    return this.data[modname]
end


function this:enter_map(pid, data, otherPid)
    local thisdata = self[1]
    local mapIdx = thisdata.g_user_mapIdx[otherPid]
    mapIdx = mapIdx or thisdata:get_free_mapIdx()

    if thisdata:add_user_to_map(mapIdx, pid, data) then
        thisdata.g_user_mapIdx[pid] = mapIdx
        return true
    end
end
function this:exit_map(pid)
    local thisdata = self[1]
    local mapIdx = thisdata.g_user_mapIdx[pid]

    if thisdata:del_user_to_map(mapIdx, pid) then
        thisdata.g_user_mapIdx[pid] = nil
        return true
    end
end
function this:get_user_data(pid)
    local thisdata = self[1]
    local mapIdx = thisdata.g_user_mapIdx[pid]
    local oneMap = thisdata.g_map_list[mapIdx]
    if oneMap then return oneMap[pid] end
end
function this:set_user_data(pid, data)    -- 若data是table结构，可直接更改里面的k-v值
    local thisdata = self[1]
    local mapIdx = thisdata.g_user_mapIdx[pid]
    local oneMap = thisdata.g_map_list[mapIdx]
    if oneMap then oneMap[pid] = data end
end
function this:get_map_info(pid)
    local thisdata = self[1]
    local mapIdx = thisdata.g_user_mapIdx[pid]
    local oneMap = thisdata.g_map_list[mapIdx]
    return oneMap
end
function this:for_each(pid, func)
    local thisdata = self[1]
    local mapIdx = thisdata.g_user_mapIdx[pid]
    local oneMap = thisdata.g_map_list[mapIdx]
    if oneMap then
        for _pid,_data in pairs(oneMap) do
            func(_pid, _data)
        end
    end
end


-------------------------------------
-- 内部函数
function this:get_free_mapIdx()
    for i, idxList in ipairs(self.g_free_idx) do
        for k, index in pairs(idxList) do
            return index
        end
    end
    local newIdx = #self.g_map_list+1
    self.g_map_list[newIdx] = {}
    self:add_free_idx(newIdx)
    return newIdx
end
-- 改变地图玩家数量，先从空闲表删除后加入
function this:add_user_to_map(mapIdx, pid, data)
    local oneMap = self.g_map_list[mapIdx]
    if oneMap == nil or oneMap[pid] then -- 已在其中
        return nil
    end
    if getTableCount(oneMap) >= self.g_map_user_max then -- 满员
        return nil 
    end 

    self:del_free_idx(mapIdx)
    
    oneMap[pid] = data -- 加入

    self:add_free_idx(mapIdx)

    return true
end
function this:del_user_to_map(mapIdx, pid)
    local oneMap = self.g_map_list[mapIdx]
    if not oneMap then return nil end

    self:del_free_idx(mapIdx)

    oneMap[pid] = nil

    self:add_free_idx(mapIdx)

    return true
end

function this:add_free_idx(mapIdx)
    local oneMap = self.g_map_list[mapIdx]
    if oneMap then
        local nFree = self.g_map_user_max - getTableCount(oneMap)
        if nFree > 0 then
            local idxList = self.g_free_idx[nFree]
            idxList[#idxList+1] = mapIdx
        end
    end
end
function this:del_free_idx(mapIdx)
    local oneMap = self.g_map_list[mapIdx]
    if oneMap then
        local nFree = self.g_map_user_max - getTableCount(oneMap)
        if nFree > 0 then
            local idxList = self.g_free_idx[nFree]
            for k,index in pairs(idxList) do
                if index == mapIdx then
                    idxList[k] = nil
                    break
                end
            end
        end
    end
end