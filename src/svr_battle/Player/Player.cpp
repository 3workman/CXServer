#include "stdafx.h"
#include "Player.h"
#ifdef _USE_RAKNET
#include "PacketPriority.h"
#include "raknet/server/UdpClientAgent.h"
#elif defined(_USE_IOCP)
#include "iocp/server/ServLink.h"
#elif defined(_USE_HANDY)
#include "handy/handy.h"
#undef info
#elif defined(_USE_LIBEVENT)
#include "libevent/server/TcpClientAgent.h"
#endif
#include "Buffer/NetPack.h"
#include "Room/Room.h"
#include "Room/MathHelper.hpp"
#include "Room/Bullet/Bullet.hpp"
#include "Room/NetInterface.h"
//#include "Lua/LuaCall.h"
#include "tool/compress.h"
#include "Room/FlatHelper.h"
#include "Room/Controller/Hero.hpp"
#include "Room/Utility/HelperStuff.hpp"

static Byte g_compress_buf[1024] = { 0 };
static const uint G_Compress_Limit_Size = 128;
static const uint G_Compress_Flag = 0x80000000;

Player::_RpcFunc Player::_rpc[rpc_enum_cnt] = {0};
std::map<uint32, Player*> Player::G_PlayerList;

Player::Player(uint32 pid) : m_pid(pid)
{
    if (G_PlayerList.empty())
    {
#undef Rpc_Declare
#define Rpc_Declare(typ) _rpc[typ] = &Player::HandleRpc_##typ;
        Rpc_For_Player;
    }
    G_PlayerList[pid] = this;
}

Player::~Player()
{
    if (CRoom* p = CRoom::FindByUniqueId(m_roomId))
    {
        p->ExitRoom(*this);
    }
    SetNetLink(NULL);

    G_PlayerList.erase(m_pid); m_pid = 0;
}

void Player::SetNetLink(NetLinkPtr p)
{
    if (_clientNetLink)
    {
        _clientNetLink->CloseLink();
    }
    _clientNetLink = p;
}

void Player::SendMsg(const NetPack& pack)
{
    const uint8 type = pack.Type();
    const void* buf = NULL; int bufLen = 0;

    if (pack.size() < G_Compress_Limit_Size) {
        buf = pack.contents();
        bufLen = pack.size();
    } else {
        //前四个字节写压缩标记
        //蛋疼啊，udp第一个字节得预留给RakNet
        uLong size = sizeof(g_compress_buf) - 5;
        *g_compress_buf = *pack.contents();
        *(uint*)(g_compress_buf + 1) = G_Compress_Flag;
        gzcompress((Bytef*)pack.contents(), pack.size(), g_compress_buf + 5, &size);
        buf = g_compress_buf;
        bufLen = size + 5;
    }

    assert(buf && bufLen && bufLen < 1024);
    switch (type) {
    case NetPack::TYPE_UDP:
        //_clientNetLink->SendUdpMsg(buf, bufLen);
        //break;
    case NetPack::TYPE_UNRELIABLE:
        //_clientNetLink->SendReliablyMsg(buf, bufLen);
        //break;
    default:
        _clientNetLink->SendMsg(buf, bufLen);
        break;
    }
}

uint64 Player::CallRpc(RpcEnum rid, const ParseRpcParam& sendFun)
{
    return sRpcClient._CallRpc(rid, sendFun, std::bind(&Player::SendMsg, this, std::placeholders::_1));
}
void Player::CallRpc(RpcEnum rid, const ParseRpcParam& sendFun, const ParseRpcParam& recvFun)
{
    auto reqKey = CallRpc(rid, sendFun);
    sRpcClient.RegistResponse(reqKey, recvFun);
}
void Player::SendRpcReplyImmediately()
{
    sRpcClient.SendBackBuffer(this);
}

Player* Player::FindByPid(uint32 pid)
{
    auto it = G_PlayerList.find(pid);
    if (it == G_PlayerList.end()) return NULL;
    return it->second;
}

//////////////////////////////////////////////////////////////////////////
// rpc
Rpc_Realize(rpc_battle_login)
{
    printf("rpc_login: %s\n", m_name.c_str());

    if (m_canJoinRoom)
    {
        printf("can join room %d\n", m_canJoinRoom);
        NotifyClientJoinRoom();
    }

    ack << m_index;

    //G_Lua->Call("rpc_client_test", "i", m_index);
//    G_Lua->Call("rpc_client_test2", "p", this);
}

Rpc_Realize(rpc_battle_logout)
{
    printf("rpc_logout\n");
    m_isLogin = false;
}

Rpc_Realize(rpc_battle_reconnect)
{
    printf("rpc_reconnect\n");
    //TODO:到这里已经重连成功了，重发战场信息
}


static CRoom* gTestRoom = nullptr;

Rpc_Realize(rpc_battle_exit_room)
{
    if (CRoom* pRoom = CRoom::FindByUniqueId(m_roomId))
    {
        pRoom->ExitRoom(*this);
        if(pRoom == gTestRoom)
        {
            this->SetNetLink(nullptr);
        }
    }
}

Rpc_Realize(rpc_battle_direct_join_room)
{
    if(!gTestRoom || gTestRoom->m_index == -1)
    {
        gTestRoom = new CRoom();
        printf("created test room\n");
    }
    
    if(gTestRoom)
    {
        gTestRoom->JoinRoom(*this);
    }

    auto& build = BackBuild();
    
    FlatVector<flat::PlayerInfo> players;
    FlatVector<flat::NetId>      netIds;
    FlatVector<flat::Vec2>       positions;

    for (auto& it : gTestRoom->All())
    {
        Player* ptr = it.second;
        auto data = flat::CreatePlayerInfo(build,
                                           ptr->m_pid,
                                           build.CreateString(ptr->m_name),
                                           ptr->m_index,
                                           ptr->m_teamId,
                                           ptr->m_roleType);
        players.push_back(data);
        
        netIds.push_back(toFlat(build, ptr->gameObject->Get<NetID>()->uid));
        positions.push_back(toFlat(build, ptr->gameObject->M().Position()));
    }
    
    auto obj = flat::CreatePlayerDirectJoinRoom(build,
                                                gTestRoom->GetUniqueId(),
                                                build.CreateVector(players),
                                                build.CreateVector(netIds),
                                                build.CreateVector(positions),
                                                this->m_index);
    build.Finish(obj);
}

Rpc_Realize(rpc_battle_handle_move_input)
{
    MoveInput input;
    input.h = req.ReadFloat();
    input.v = req.ReadFloat();
    
//    if(input.h != 0.0f || input.v != 0.0f)
//        LOG_DEBUG("player %d, handle move input %f %f", m_index, input.h, input.v);
    
    gameObject->Get<INetMoveHandler>()->HandleMoveInput(input);
}

Rpc_Realize(rpc_battle_handle_fire_input)
{
    NetID::ID sourceId = req.ReadUuid();
    
    FireInput input;
    input.key = req.ReadInt32();
    input.state = req.ReadInt32();
    input.position = Vec2f(req.ReadFloat(), req.ReadFloat());
    input.dir = Vec2f(req.ReadFloat(), req.ReadFloat());
    
    if (CRoom* pRoom = CRoom::FindByUniqueId(m_roomId))
    {
        gameObject->Get<INetFireHandler>()->HandleFireInput(input, pRoom, sourceId);
    }
}

Rpc_Realize(rpc_battle_handle_trigger_enter)
{
    if (CRoom* pRoom = CRoom::FindByUniqueId(m_roomId))
    {
        NetID::ID selfId = req.ReadUuid();
        NetID::ID otherId = req.ReadUuid();
        
        printf("Trigger Enter(%s %s)\n", uuidToNumStr(selfId).c_str(), uuidToNumStr(otherId).c_str());
        
        shared<GameObject> self = pRoom->NetTable().Get(selfId).lock();
        shared<GameObject> other = pRoom->NetTable().Get(otherId).lock();
        
        if(self != nullptr && other != nullptr)
        {
            // forward trigger message to all other players except the source player
            // maybe after actual trigger action?
            // pRoom->SyncTriggerEnter(pRoom->AllBut(this), selfId, otherId);
            
            // send to all
            // client collision handling cannot tell when it should use net trigger and
            // when it's local trigger. So this is handled universally.
            // In the future, collision should be handled on server.
            pRoom->SyncTriggerEnter(pRoom->All(), selfId, otherId);
            if(auto receiver = self->Get<ITriggerReceiver>())
            {
                receiver->OnTriggerEnter(other);
            }
        }
        
        if(self == nullptr || other == nullptr)
            printf("collision enter has null object %p %p\n", self.get(), other.get());
    }
}

Rpc_Realize(rpc_battle_handle_trigger_exit)
{
    if (CRoom* pRoom = CRoom::FindByUniqueId(m_roomId))
    {
        NetID::ID selfId = req.ReadUuid();
        NetID::ID otherId = req.ReadUuid();
        
        printf("Trigger Exit(%s %s)\n", uuidToNumStr(selfId).c_str(), uuidToNumStr(otherId).c_str());
        
        shared<GameObject> self = pRoom->NetTable().Get(selfId).lock();
        shared<GameObject> other = pRoom->NetTable().Get(otherId).lock();
        
        if(self != nullptr && other != nullptr)
        {
            // forward trigger message to all other players except the source player
            // maybe after actual trigger action?
            //pRoom->SyncTriggerExit(pRoom->AllBut(this), selfId, otherId);
            
            pRoom->SyncTriggerExit(pRoom->All(), selfId, otherId);
            if(auto receiver = self->Get<ITriggerReceiver>())
            {
                receiver->OnTriggerExit(other);
            }
        }
        
        if(self == nullptr || other == nullptr)
            printf("collision exit has null object %p %p\n", self.get(), other.get());
    }
}

Rpc_Realize(rpc_battle_forward_anim_state)
{
    if(CRoom* room = CRoom::FindByUniqueId(m_roomId))
    {
        NetID::ID selfId = req.ReadUuid();
        std::string stateName = req.ReadString();
        printf("received forward anim state %s\n", stateName.c_str());
        
        shared<GameObject> self = room->NetTable().Get(selfId).lock();
        if(self != nullptr)
        {
            room->SyncAnimState(room->AllBut(this), self, stateName);
        }
        
        if(self == nullptr)
            printf("battle_forward_anim_state has null object %p\n", self.get());
    }
}

Rpc_Realize(rpc_battle_pick_weapon)
{
    if(CRoom* room = CRoom::FindByUniqueId(m_roomId))
    {
        NetID::ID selfId = req.ReadUuid();
        NetID::ID weaponId = req.ReadUuid();
        
        shared<GameObject> self = room->NetTable().Get(selfId).lock();
        shared<GameObject> weapon = room->NetTable().Get(weaponId).lock();
        
        if(self != nullptr && weapon != nullptr)
        {
            self->Get<HeroController>()->PickWeapon(weapon);
        }
        
        if(self == nullptr || weapon == nullptr)
            printf("rpc_battle_pick_weapon has null object self: %p, weapon %p\n", self.get(), weapon.get());
    }
}

Rpc_Realize(rpc_battle_drop_weapon)
{
    if(CRoom* room = CRoom::FindByUniqueId(m_roomId))
    {
        NetID::ID selfId = req.ReadUuid();
        
        shared<GameObject> self = room->NetTable().Get(selfId).lock();
        
        if(self != nullptr)
        {
            self->Get<HeroController>()->DropWeapon();
        }
        
        if(self == nullptr)
            printf("rpc_battle_pick_weapon has null object self: %p\n", self.get());
    }
}

void Player::NotifyClientJoinRoom()
{
    if (CRoom* pRoom = CRoom::FindByUniqueId(m_roomId))
    {
        pRoom->JoinRoom(*this); //后台必须先加进来，否则两个人同时进房间，有bug：不知道彼此加入

        CallRpc(rpc_client_stop_wait_and_load_battle_scene, [&](NetPack& buf) {

            auto& build = SendBuild();
            
            FlatVector<flat::PlayerInfo> players;
            FlatVector<flat::NetId> netIds;
            FlatVector<flat::Vec2> positions;

            for (auto& it : pRoom->All())
            {
                Player* ptr = it.second;
                auto playerInfo = flat::CreatePlayerInfo(build,
                    ptr->m_pid,
                    build.CreateString(ptr->m_name),
                    ptr->m_index,
                    ptr->m_teamId
                );
                players.push_back(playerInfo);
                netIds.push_back(toFlat(build, ptr->gameObject->Get<NetID>()->uid));
                positions.push_back(toFlat(build, ptr->gameObject->M().Position()));
            }
            
            const auto& obj = flat::CreateRoomPlayers(build,
                                                      pRoom->GetUniqueId(),
                                                      build.CreateVector(players),
                                                      build.CreateVector(netIds),
                                                      build.CreateVector(positions));
            build.Finish(obj);

            // 通知Client关闭等待界面，载入战斗场景，载入完毕后回复svr
            // 下发房间内玩家的数据，给client创建远程镜像(包含自己)
            //buf << pRoom->GetUniqueId();
            //buf.WriteUInt8(pRoom->GetPlayerLst().size());
            //for (auto& it : pRoom->GetPlayerLst()) {
            //    Player* ptr = it.second;
            //    buf.WriteUInt32(ptr->m_pid);
            //    buf.WriteString(ptr->m_name);
            //    buf.WriteUInt32(ptr->m_index);
            //    buf.WriteUInt8(ptr->m_teamId);
            //}
        });
    }
}
