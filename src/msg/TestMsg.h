#pragma once
#include "MsgEnum.h"

struct TestMsg : public stMsg
{
    char data[128];
    int size(){ return offsetof(TestMsg, data) + strlen(data) + 1; }
};