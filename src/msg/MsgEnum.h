#pragma once


enum MyEnum
{
    Login,
    Echo,

    MSG_MAX_CNT
};
struct stMsg
{
    MyEnum msgId;
};