#pragma once
#include "tool/Mempool.h"

class Player;
class CRoom {
    Pool_Index_Define(CRoom, 50);
    Pool_Index_UniqueID32(CRoom);
private:
    std::map<uint, Player*> m_players;

public:
    CRoom();
    static CRoom* CreateRoom(Player& player);
    void DestroyRoom();
    bool JoinRoom(Player& player);
    bool ExitRoom(Player& player);
    const std::map<uint, Player*>& GetPlayerLst(){ return m_players; }
    Player* GetPlayer(uint idx);

    void SyncPlayerPosition();
};
