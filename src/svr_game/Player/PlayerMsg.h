/***********************************************************************
* @ 玩家消息函数声明
* @ brief
    1、使用宏：Msg_Declare、Msg_Realize，按模块划分文件，如：FuBenHandler.h、FuBenHandler.cpp
    2、本文件#include上述各模块（Player.h就只需#include "PlayerMsg.h"了）

* @ author zhoumf
* @ date 2016-12-12
************************************************************************/

Msg_Declare(C2S_Login);
Msg_Declare(C2S_ReConnect);
Msg_Declare(C2S_Echo);
