#pragma once
struct stMsg
{
    uint16 msgId;
};
enum MsgEnum {
    C2S_Login,
    C2S_ReConnect,
    C2S_Echo,

    MSG_MAX_CNT
};
