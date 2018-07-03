#pragma once

struct THeroInfo { //英雄成长数值
    uint8           heroId;
    uint8           starLv;
    //AttrsCalc::Data data;
};

class Player;
class NetPack;
class DBData {
    Player&     _owner;
    uint32      m_diamond;      //钻石
    uint32      m_exp;          //玩家经验
    uint32      m_level;        //玩家等级
    std::vector<THeroInfo>  m_heros;//英雄成长数值

public:
    DBData(Player& o) : _owner(o) {}
    void BufToData(NetPack& buf);
    void DataToBuf(NetPack& buf) const;
    void WriteDB() const;

public: //数据接口
    uint32  GetDiamond() { return m_diamond; }
    void    SetDiamond(uint32 v) { m_diamond = v; }

    uint32& GetExp() { return m_exp; }
    uint32& GetLevel() { return m_level; }

    const THeroInfo* GetHeroInfo(uint8 heroId);


};