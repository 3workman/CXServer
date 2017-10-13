#include "stdafx.h"
#include "DataDiff.h"
#include "NetPack.h"

void DataDiff::diff(NetPack& buf)
{
    _bit = 0; int bitPosInPack = (int)buf.wpos();
    buf << _bit; //先写入位标记，diff完毕后重设

    const char* val;
    int         len;
    int         i = 0;

    for (auto& it : _runtimeRef)
    {
        val = (const char*)it.first;
        len = it.second;

        while (len)
        {
            if (*val != _cache[i])
            {
                _cache[i] = *val;
                _bit |= (1 << i);
                buf << *val;
            }

            val += 1; ++i; --len;
        }
    }
    buf.put(bitPosInPack, _bit);//重设位标记
}