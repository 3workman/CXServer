print("hello world")

str = "I am so cool"
tbl = {name = "shun", id = 20114442}
function add(a,b)
    return a + b
end

-- avg, sum = average(10, 20, 30, 40, 50)  
-- print("The average is ", avg)  
-- print("The sum is ", sum)

local buf = ByteBuffer(32)
buf:WriteString("--- test buffer ---")
print( buf:ReadString() )

function rpc_client_test(idx)
    print("--- lua:rpc_client_test ---")
    local ptr = Player.FindByIdx(idx)
    print("--- FindByIdx Success ---")
    -- ptr:CallRpc3("rpc_client_lua_test", -- lua里生成的闭包，在c++里调比较艰难
    --     function (buf)
    --         buf:WriteString("lua_test")
    --     end,
    --     function (buf)
    --         print( buf:ReadString() )
    --     end)
    local data = NetPack(64)
    data:WriteString("lua_test")
    data.OpCode = 15001
    ptr:SendMsg(data)
    print("--- CallRpc3 Success ---")
end
function rpc_client_test2(player)
    local data = NetPack(64)
    data:WriteString("lua_test")
    data.OpCode = 15001
    player:SendMsg(data)
    print("--- CallRpc3 Success ---")
end