#include "StdAfx.h"
#include "StringHash.h"

using namespace std;

StringHash::StringHash(const long nTableLength /*= MAXTABLELEN*/)
{
    InitCryptTable();
    m_tablelength = nTableLength;
    //初始化hash表
    m_HashIndexTable = new HASHTABLE[nTableLength];
    for (int i = 0; i < nTableLength; i++)
    {
        m_HashIndexTable[i].nHashA = -1;
        m_HashIndexTable[i].nHashB = -1;
        m_HashIndexTable[i].bExists = false;
    }
}

StringHash::~StringHash(void)
{
    if (m_HashIndexTable)
    {
        delete[]m_HashIndexTable;
        m_HashIndexTable = NULL;
        m_tablelength = 0;
    }
}

/************************************************************************/
/*功  能：对哈希索引表预处理
/*返回值：无
/************************************************************************/
void StringHash::InitCryptTable()
{
    unsigned long seed = 0x00100001, index1 = 0, index2 = 0, i;

    for (index1 = 0; index1 < 0x100; index1++)
    {
        for (index2 = index1, i = 0; i < 5; i++, index2 += 0x100)
        {
            unsigned long temp1, temp2;
            seed = (seed * 125 + 3) % 0x2AAAAB;
            temp1 = (seed & 0xFFFF) << 0x10;
            seed = (seed * 125 + 3) % 0x2AAAAB;
            temp2 = (seed & 0xFFFF);
            cryptTable[index2] = (temp1 | temp2);
        }
    }
}

/************************************************************************/
/*功  能：求取哈希值
/*返回值：返回hash值
/************************************************************************/
unsigned long StringHash::HashString(const string& lpszString, unsigned long dwHashType)
{
    unsigned char *key = (unsigned char *)(const_cast<char*>(lpszString.c_str()));
    unsigned long seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;
    int ch;

    while (*key != 0)
    {
        ch = toupper(*key++);

        seed1 = cryptTable[(dwHashType << 8) + ch] ^ (seed1 + seed2);
        seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
    }
    return seed1;
}

/************************************************************************/
/*功  能：检测一个字符串是否被hash过
/*返回值：如果存在，返回位置；否则，返回-1
/************************************************************************/
unsigned long StringHash::Hashed(string lpszString)
{
    const unsigned long HASH_OFFSET = 0, HASH_A = 1, HASH_B = 2;
    //不同的字符串三次hash还会碰撞的几率无限接近于不可能
    unsigned long nHash = HashString(lpszString, HASH_OFFSET);
    unsigned long nHashA = HashString(lpszString, HASH_A);
    unsigned long nHashB = HashString(lpszString, HASH_B);
    unsigned long nHashStart = nHash % m_tablelength, nHashPos = nHashStart;

    while (m_HashIndexTable[nHashPos].bExists)
    {
        if (m_HashIndexTable[nHashPos].nHashA == nHashA && m_HashIndexTable[nHashPos].nHashB == nHashB)
            return nHashPos;
        else
            nHashPos = (nHashPos + 1) % m_tablelength;

        if (nHashPos == nHashStart) break;
    }
    return -1; //没有找到  
}

/************************************************************************/
/*功  能：hash一个字符串
/*返回值：成功，返回true；失败，返回false
/************************************************************************/
bool StringHash::Hash(string lpszString)
{
    const unsigned long HASH_OFFSET = 0, HASH_A = 1, HASH_B = 2;
    unsigned long nHash = HashString(lpszString, HASH_OFFSET);
    unsigned long nHashA = HashString(lpszString, HASH_A);
    unsigned long nHashB = HashString(lpszString, HASH_B);
    unsigned long nHashStart = nHash % m_tablelength, nHashPos = nHashStart;

    while (m_HashIndexTable[nHashPos].bExists)
    {
        nHashPos = (nHashPos + 1) % m_tablelength;
        if (nHashPos == nHashStart) //一个轮回  
        {
            return false; //hash表中没有空余的位置了,无法完成hash
        }
    }
    m_HashIndexTable[nHashPos].bExists = true;
    m_HashIndexTable[nHashPos].nHashA = nHashA;
    m_HashIndexTable[nHashPos].nHashB = nHashB;

    return true;
}