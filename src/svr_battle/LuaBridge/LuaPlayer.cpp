#include "stdafx.h"
#include "Lua/tolua.h"
#include "Player/Player.h"

namespace tolua {

ToLua(Player)
{
    getGlobalNamespace(L)
        .beginClass<Player>("Player")
            .addStaticFunction("FindByIdx", &Player::FindByIdx)
            .addStaticFunction("FindByPid", &Player::FindByPid)
            .addFunction("CallRpc2", static_cast<uint64(Player::*)(const char*, const ParseRpcParam&)>(&Player::CallRpc))
            .addFunction("CallRpc3", static_cast<void(Player::*)(const char*, const ParseRpcParam&, const ParseRpcParam&)>(&Player::CallRpc))
            .addFunction("SendMsg", &Player::SendMsg)
        .endClass()
        ;
}

}