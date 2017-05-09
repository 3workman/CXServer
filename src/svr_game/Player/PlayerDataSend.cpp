#include "stdafx.h"
#include "PlayerDataSend.h"
#include "Buffer/NetPack.h"

void PlayerDataSend::ToBuf(NetPack& buf)
{
    for (auto& info : m_vecKey) {
        switch (info.type) {
        case v_uint8:{
            buf << (*(uint8*)info.pObj);
        }break;
        case v_uint16:{
            buf << (*(uint16*)info.pObj);
        }break;
        case v_uint32:{
            buf << (*(uint32*)info.pObj);
        }break;
        case v_uint64:{
            buf << (*(uint64*)info.pObj);
        }break;
        case v_int8:{
           buf << (*(int8*)info.pObj);
        }break;
        case v_int16:{
            buf << (*(int16*)info.pObj);
        }break;
        case v_int32:{
            buf << (*(int32*)info.pObj);
        }break;
        case v_int64:{
            buf << (*(int64*)info.pObj);
        }break;
        case v_float:{
            buf << (*(float*)info.pObj);
        }break;
        case v_double:{
            buf << (*(double*)info.pObj);
        }break;
        case v_string:{
            buf << info.pObj;
        }break;
        default:assert(0); break;
        }
    }
}