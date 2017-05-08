/***********************************************************************
* @ 绑定内建型变量，自动同步客户端
* @ brief
	1、FIXME【不支持位域变量的绑定】


* @ author zhoumf
* @ date 2017-5-8
***********************************************************************/
#pragma once
#include <string>
#include <vector>
#include <assert.h>

class NetPack;
class PlayerDataSend {
    enum ValueEnum {
        v_uint8,
        v_uint16,
        v_uint32,
        v_uint64,
        v_int8,
        v_int16,
        v_int32,
        v_int64,
        v_float,
        v_double,
        v_string,
    };
    struct stKeyInfo {
        ValueEnum type;
        const char* pObj = NULL;

        stKeyInfo(ValueEnum typ) : type(typ){}
    };
    vector<stKeyInfo> m_vecKey;

public:
    void DataToBuf(NetPack& buf);

    PlayerDataSend& operator<<(const uint8& value) {
        stKeyInfo info(v_uint8);
        info.pObj = (char*)&value;
        m_vecKey.push_back(info);
        return *this;
    }
    PlayerDataSend& operator<<(const uint16& value) {
        stKeyInfo info(v_uint16);
        info.pObj = (char*)&value;
        m_vecKey.push_back(info);
        return *this;
    }
    PlayerDataSend& operator<<(const uint32& value) {
        stKeyInfo info(v_uint32);
        info.pObj = (char*)&value;
        m_vecKey.push_back(info);
        return *this;
    }
    PlayerDataSend& operator<<(const uint64& value) {
        stKeyInfo info(v_uint64);
        info.pObj = (char*)&value;
        m_vecKey.push_back(info);
        return *this;
    }
    PlayerDataSend& operator<<(const int8& value) {
        stKeyInfo info(v_int8);
        info.pObj = (char*)&value;
        m_vecKey.push_back(info);
        return *this;
    }
    PlayerDataSend& operator<<(const int16& value) {
        stKeyInfo info(v_int16);
        info.pObj = (char*)&value;
        m_vecKey.push_back(info);
        return *this;
    }
    PlayerDataSend& operator<<(const int32& value) {
        stKeyInfo info(v_int32);
        info.pObj = (char*)&value;
        m_vecKey.push_back(info);
        return *this;
    }
    PlayerDataSend& operator<<(const int64& value) {
        stKeyInfo info(v_int64);
        info.pObj = (char*)&value;
        m_vecKey.push_back(info);
        return *this;
    }
    PlayerDataSend& operator<<(const bool& value) {
        stKeyInfo info(v_int8);
        info.pObj = (char*)&value;
        m_vecKey.push_back(info);
        return *this;
    }
    PlayerDataSend& operator<<(const float& value) {
        stKeyInfo info(v_float);
        info.pObj = (char*)&value;
        m_vecKey.push_back(info);
        return *this;
    }
    PlayerDataSend& operator<<(const double& value) {
        stKeyInfo info(v_double);
        info.pObj = (char*)&value;
        m_vecKey.push_back(info);
        return *this;
    }
    PlayerDataSend& operator<<(char* value) {
        stKeyInfo info(v_string);
        info.pObj = value;
        m_vecKey.push_back(info);
        return *this;
    }
    PlayerDataSend& operator<<(const char* value) {
        stKeyInfo info(v_string);
        info.pObj = value;
        m_vecKey.push_back(info);
        return *this;
    }
};