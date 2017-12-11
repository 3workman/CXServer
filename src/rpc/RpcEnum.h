#pragma once

#undef Rpc_Declare
#define Rpc_Declare(typ) void HandleRpc_##typ(NetPack&, NetPack&);

#define Rpc_For_Player\
    Rpc_Declare(rpc_battle_login)\
    Rpc_Declare(rpc_battle_logout)\
    Rpc_Declare(rpc_battle_reconnect)\
    Rpc_Declare(rpc_battle_exit_room)\
    Rpc_Declare(rpc_battle_direct_join_room)\
    Rpc_Declare(rpc_battle_handle_move_input)\
    Rpc_Declare(rpc_battle_handle_fire_input)\
    Rpc_Declare(rpc_battle_handle_trigger_enter)\
    Rpc_Declare(rpc_battle_handle_trigger_exit)\
    Rpc_Declare(rpc_battle_forward_anim_state)\
    Rpc_Declare(rpc_battle_pick_weapon)\
    Rpc_Declare(rpc_battle_drop_weapon)\


#define Rpc_For_Cross\
    Rpc_Declare(rpc_battle_handle_player_data)\


#define Rpc_For_Zookeeper\
    Rpc_Declare(rpc_svr_node_join)\


#define Rpc_For_Client\

