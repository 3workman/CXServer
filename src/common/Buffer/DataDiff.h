/***********************************************************************
* @ 自动打包引用数据中的变动
* @ brief
	1、
	2、
* @ example
    1、声明 DataDiff，用组合的方式，保证同绑定数据的生命周期一致
        class Player
        {
            struct HpConf
            {
                int hp;
                int mp;
            }           m_hp;
            struct PosConf
            {
                int x;
                int y;
            }           m_pos;

            DataDiff    m_diff;
        };

    2、构造函数中绑定关心的数据块，绑定顺序服务器、客户端需一致
        Player::Player()
        {
            m_diff << m_conf << m_pos;
        }

    3、Service里遍历对象的diff，讲差异发送给client；这里可以一次发送所有房间对象的diff，包括其它玩家、npc
        void CRoom::SyncPlayersDiff()
        {
            for (auto& it : m_players)
            {
                it.second->m_conf.hp = Rand::rand();
                it.second->m_conf.mp = Rand::rand();

                printf("--- hp(%d), mp(%d) \n", it.second->m_conf.hp, it.second->m_conf.mp);

                it.second->CallRpc(rpc_client_data_diff, [&](NetPack& buf)
                {
                    it.second->m_diff.diff(buf);
                });
            }
        }

* @ author zhoumf
* @ date 2017-8-3
************************************************************************/
#pragma once
#include <vector>
#include <assert.h>

class NetPack;
class DataDiff
{
    typedef std::pair<const void* const, int> RefData;

    int32                   _bit = 0; //位标记，表示变动情况
    char                    _cache[sizeof(DataDiff::_bit)*8];
    std::vector<RefData>    _runtimeRef;
    int                     _wpos = 0;

public:
    template <typename T> DataDiff& operator<<(const T& ref)
    {
        if (sizeof(_cache) >= _wpos + sizeof(ref))
        {
            _runtimeRef.push_back(RefData(&ref, sizeof(ref)));
            memcpy(&_cache[_wpos], &ref, sizeof(ref));
            _wpos += sizeof(ref);
        }
        else
        {
            assert(0 && "cache size isn't enough !");
        }
        return *this;
    }

    void diff(NetPack& buf);
};

//void CRoom::WriteObjectDiff(NetPack& buf, shared<GameObject> obj)
//{
//    buf.WriteUuid(obj->Get<NetID>()->uid);
//
//    if (auto ptr = obj->Get<DataDiff>())
//    {
//        ptr->diff(buf);
//    }
//}
