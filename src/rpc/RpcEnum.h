#pragma once

#define Rpc_For_Player\
    Rpc_Declare(rpc_echo)\
    Rpc_Declare(rpc_login)\
    Rpc_Declare(rpc_logout)\
    Rpc_Declare(rpc_reconnect)\
    Rpc_Declare(rpc_create_room)\
    Rpc_Declare(rpc_join_room)\
    Rpc_Declare(rpc_exit_room)\
    Rpc_Declare(rpc_move_delta)\
    Rpc_Declare(rpc_client_load_battle_scene_ok)\


#define Rpc_For_Cross\
    Rpc_Declare(rpc_echo)\
    Rpc_Declare(rpc_svr_accept)\
    Rpc_Declare(rpc_handle_battle_data)\


#define Rpc_For_Client\

