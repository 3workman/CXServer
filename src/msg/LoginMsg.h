#pragma once
#include "MsgEnum.h"

struct LoginMsg : public stMsg
{
};
struct ReConnectMsg : public stMsg
{
    DWORD playerIdx;
};