function print_r(t)
    local oldTable = {} --已被打印过的table
    local function sub_print(t,indent)
        if type(t) == "table" then
            oldTable[tostring(t)] = true
            for k,v in pairs(t) do
                if type(v) == "table" then
                    if oldTable[tostring(v)] then --循环引用的table
                        print(indent.."["..k.."] = *"..tostring(v))
                    else
                        print(indent.."["..k.."] = "..tostring(v).." {")
                        sub_print(v,indent..string.rep(" ",string.len(k)+8))
                        print(indent..string.rep(" ",string.len(k)+6).."}")
                    end
                elseif type(v) == "string" then
                    print(indent.."["..k..'] = "'..v..'"')
                else
                    print(indent.."["..k.."] = "..tostring(v))
                end
            end
        elseif type(t) == "string" then
            print(indent..'"'..t..'"')
        else
            print(indent..tostring(t))
        end
    end
    if type(t) == "table" then
        print(tostring(t).." {")
        sub_print(t,"  ")
        print("}")
    else
        sub_print(t,"  ")
    end
end
