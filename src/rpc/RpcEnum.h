#pragma once

#undef Rpc_Declare
#define Rpc_Declare(typ) void HandleRpc_##typ(NetPack&, NetPack&);

#define Rpc_For_Player\
    Rpc_Declare(Rpc_battle_login)\
    Rpc_Declare(Rpc_battle_logout)\
    Rpc_Declare(Rpc_battle_reconnect)\
    Rpc_Declare(Rpc_battle_exit_room)\


#define Rpc_For_Cross\
    Rpc_Declare(Rpc_battle_handle_player_data)\


#define Rpc_For_Zookeeper\
    Rpc_Declare(Rpc_svr_node_join)\


#define Rpc_For_Client\

