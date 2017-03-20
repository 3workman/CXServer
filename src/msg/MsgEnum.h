#pragma once

struct stMsg
{
    uint16 msgId;
};

#undef Msg_Declare
#undef Msg_Enum
#define Msg_Declare(typ, n) typ = n,
#define Msg_Enum\
    Msg_Declare(C2S_Login, 0)           \
    Msg_Declare(C2S_ReConnect, 1)       \
    Msg_Declare(C2S_Echo, 2)            \

enum MsgEnum
{
    Msg_Enum

    _MSG_MAX_CNT
};