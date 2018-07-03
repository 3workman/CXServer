#include "stdafx.h"
#include "StringHash.h"

void StringHash::InitCRCTable()
{
    if (_init) return;

    uint c = 0, j = 0;
    for (uint i = 0; i < ARRAY_SIZE(_crcTable); ++i)
    {
        c = i << 24;
        for (j = 8; j != 0; --j)
        {
            if ((c & 0x80000000) != 0)
                c = (c << 1) ^ _crcPoly;
            else
                c = c << 1;

            _crcTable[i] = c;
        }
    }
    _init = true;
}

uint StringHash::Hash(std::string str)
{
    InitCRCTable();

    uint hash = 0, b = 0;
    for (size_t i = 0; i < str.size(); ++i)
    {
        b = (uint)(str[i]);
        hash = ((hash >> 8) & 0x00FFFFFF) ^ _crcTable[(hash ^ b) & 0x000000FF];
    }
    return hash;
}