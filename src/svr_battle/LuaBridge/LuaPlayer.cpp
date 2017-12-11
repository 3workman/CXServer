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
            .addFunction("SendMsg", &Player::SendMsg)
        .endClass()
        ;
}

}