#pragma once
#include "tool/Mempool.h"

class Player;
class CRoom {
    Pool_Index_Define(CRoom, 50);
    Pool_Index_UniqueID32(CRoom);
    static std::map<uint32, CRoom*>     RoomList;
private:
    std::map<uint, Player*>             m_players;
    std::map<uint8, vector<Player*>>    m_waitLst;//各战队的等待列表长度相差不大于1，才加入战斗
    const uint8                         m_kTeamCntMax;//本房间允许的战队数目

public:
    CRoom(uint8 teamCnt = 2);
    void DestroyRoom();
    bool JoinRoom(Player& player);
    bool ExitRoom(Player& player);

    const std::map<uint, Player*>& GetPlayerLst(){ return m_players; }
    Player* FindBattlePlayer(uint idx);

    void ForEachTeammate(uint8 teamId, std::function<void(Player&)>& func);
    void ForEachPlayer(std::function<void(Player&)>& func);

    bool TryToJoinWaitLst(const std::vector<Player*>& lst);
    void _FlushWaitLst(const std::map<uint8, uint>& teamInfos);

    void SyncPlayerPosition();
};
