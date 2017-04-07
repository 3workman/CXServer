#pragma once

#undef Rpc_Declare
#undef Rpc_Enum
#define Rpc_Declare(typ) typ,
#define Rpc_Enum\
    Rpc_Declare(rpc_echo)\
    Rpc_Declare(rpc_login)\
    Rpc_Declare(rpc_logout)\
    Rpc_Declare(rpc_reconnect)\
    Rpc_Declare(rpc_create_room)\
    Rpc_Declare(rpc_join_room)\
    Rpc_Declare(rpc_exit_room)\
    Rpc_Declare(rpc_move_delta)\

enum RpcEnum 
{
    Rpc_Enum
};