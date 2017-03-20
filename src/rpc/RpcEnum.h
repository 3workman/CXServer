#pragma once

#undef Rpc_Declare
#undef Rpc_Enum
#define Rpc_Declare(typ, n) typ = n,
#define Rpc_Enum\
    Rpc_Declare(rpc_login, 0)           \
    Rpc_Declare(rpc_reconnect, 1)       \
    Rpc_Declare(rpc_echo, 2)            \

enum RpcEnum 
{
    Rpc_Enum
    
    _RPC_MAX_CNT
};