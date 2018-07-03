#include "stdafx.h"
#include "DBData.h"
#include "Player.h"
#include "Buffer/NetPack.h"
#include "Zookeeper/Zookeeper.h"

void DBData::BufToData(NetPack& buf)
{
    m_diamond = buf.ReadUInt32();
    m_exp = buf.ReadUInt32();
    m_level = buf.ReadUInt32();
    uint8 cnt = buf.ReadUInt8(); 
    m_heros.clear(); THeroInfo info;
    for (uint8 i = 0; i < cnt; ++i) {
        info.heroId = buf.ReadUInt8();
        info.starLv = buf.ReadUInt8();
        //info.data.hp = buf.ReadUInt8();
        //info.data.energy = buf.ReadUInt8();
        //info.data.armor = buf.ReadUInt8();
        m_heros.push_back(std::move(info));
    }
}
void DBData::DataToBuf(NetPack& buf) const
{
    buf.WriteUInt32(m_diamond);
    buf.WriteUInt32(m_exp);
    buf.WriteUInt32(m_level);
    buf.WriteUInt8(m_heros.size());
    for (const auto& it : m_heros) {
        buf.WriteUInt8(it.heroId);
        buf.WriteUInt8(it.starLv);
        //buf.WriteUInt8(it.data.hp);
        //buf.WriteUInt8(it.data.energy);
        //buf.WriteUInt8(it.data.armor);
    }
}

void DBData::WriteDB() const
{
    if (auto pCross = sZookeeper.GetCross())
    {
        pCross->CallRpc(rpc_cross_relay_to_game, [&](NetPack& buf) {
            buf.WriteInt32(_owner.m_svrId);
            buf.WriteUInt16(rpc_game_write_db_battle_info);
            buf.WriteUInt32(_owner.m_pid);
            DataToBuf(buf);
        });
    }
}

const THeroInfo* DBData::GetHeroInfo(uint8 heroId)
{
    for (auto& it : m_heros)
    {
        if (it.heroId == heroId) return &it;
    }
    return nullptr;
}