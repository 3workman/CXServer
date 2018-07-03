#pragma once

class StringHash
{
private:
    static bool _init;
    static uint _crcTable[256];
    static const uint _crcPoly = 0x04c11db7;

    static void InitCRCTable();
public:
    static uint Hash(std::string str);
};