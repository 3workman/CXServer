function table.copy_ex(src)
    if type(src) ~= "table" then return src end
    local ret = {}
    for k,v in pairs(src) do
        if type(v) == "table" then
            ret[k] = table.copy_ex(v)
        else
            ret[k] = v
        end
    end
    return ret
end

function table.u2t(src)
    local srctype = type(src)
    if srctype ~= "table" and srctype ~= "userdata" then
        return src
    end
    local ret = {}
    for k,v in pairs(src) do
        local vtype = type(v)
        if vtype == "table" or vtype == "userdata" then
            ret[k] = table.u2t(v)
        else
            ret[k] = v
        end
    end
    return ret
end