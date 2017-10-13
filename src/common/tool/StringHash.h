#pragma once

#define MAXTABLELEN 1024    // 默认哈希索引表大小 
//////////////////////////////////////////////////////////////////////////  
// 哈希索引表定义  
typedef struct  _HASHTABLE
{
    long nHashA;
    long nHashB;
    bool bExists;
}HASHTABLE, *PHASHTABLE;

class StringHash
{
public:
    StringHash(const long nTableLength = MAXTABLELEN);
    ~StringHash(void);
private:
    unsigned long cryptTable[0x500];
    unsigned long m_tablelength;    // 哈希索引表长度  
    HASHTABLE *m_HashIndexTable;
private:
    void InitCryptTable();                                               // 对哈希索引表预处理 
    unsigned long HashString(const std::string &lpszString, unsigned long dwHashType); // 求取哈希值      
public:
    bool Hash(std::string url);
    unsigned long Hashed(std::string url);    // 检测url是否被hash过
};