#pragma once

#define Rpc_For_Player\
    Rpc_Declare(rpc_echo)\
    Rpc_Declare(rpc_battle_login)\
    Rpc_Declare(rpc_battle_logout)\
    Rpc_Declare(rpc_battle_reconnect)\
    Rpc_Declare(rpc_battle_exit_room)\
    Rpc_Declare(rpc_battle_move_delta)\


#define Rpc_For_Cross\
    Rpc_Declare(rpc_echo)\
    Rpc_Declare(rpc_svr_accept)\
    Rpc_Declare(rpc_battle_handle_player_data)\


#define Rpc_For_Client\

