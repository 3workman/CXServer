print("--- test_player ---")

function rpc_client_test2(player)
    local data = NetPack(64)
    data:WriteString("lua_test")
    data.OpCode = 15001
    player:SendMsg(data)
    print("--- rpc_client_test_player Success ---")
end