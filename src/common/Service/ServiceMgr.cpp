#include "stdafx.h"
#include "ServiceMgr.h"
#include "Service.h"
#include "tool/GameApi.h"

void _Service_Patch(void* p, uint)
{
    if ((size_t)p > 100) ServiceMgr::UnRegister(Service_Test_Patch, p);

    printf("Service Path(%p)...\n", p);
}
void _Service_List(void* p, uint time_elapse)
{
    if ((size_t)p > 100) ServiceMgr::UnRegister(Service_Test_List, p);

    printf("Service List(%p)...\n", p);
}
void _Service_Vec(void* p, uint time_elapse)
{
    if ((size_t)p > 100) ServiceMgr::UnRegister(Service_Test_Vec, p);

    printf("Service Vec(%p)...\n", p);
}
void _Service_Map(void* p, uint time_elapse)
{
    if ((size_t)p > 100) ServiceMgr::UnRegister(Service_Test_Map, p);

    printf("Service Map(%p)...\n", p);
}
extern void _Service_Room_GameLoop(void* p, uint time_elapse);
extern void _Service_Net_Stats(void* p, uint time_elapse);
extern void _Service_Robot(void* p, uint time_elapse);
extern void _Service_TrapPoison(weak<GameObject> p, uint time_elapse);
extern void _Service_GroupTreat(weak<GameObject> p, uint time_elapse);
extern void _Service_Position(weak<GameObject> p, uint time_elapse);
extern void _Service_Player_Position(weak<GameObject> p, uint time_elapse);
//extern uint _Service_Buff(void* p);

//////////////////////////////////////////////////////////////////////////
// ServiceMgr
static iService<void*>* g_service[] = {
    new ServicePatch<void*>(_Service_Patch, 10000),
    new ServiceList<void*>(_Service_List, 2000),
    new ServiceVec<void*>(_Service_Vec, 2000),
    new ServiceMap<void*>(_Service_Map, 2000),
    new ServiceVec<void*>(_Service_Room_GameLoop, 20),
    new ServiceVec<void*>(_Service_Net_Stats, 2000),
    new ServiceVec<void*>(_Service_Robot, 500),
};
static iService<weak<GameObject>>* g_service2[] = {
    new ServiceList<weak<GameObject>>(_Service_TrapPoison, 1000),
    new ServiceList<weak<GameObject>>(_Service_GroupTreat, 1000),
    new ServiceList<weak<GameObject>>(_Service_Position, 100),
    new ServiceList<weak<GameObject>>(_Service_Player_Position, 100),
    //new cServiceVec<weak<GameObject>>(_Service_Buff),
};
void ServiceMgr::RunAllService(uint time_elapse, time_t timenow)
{
    for (auto& it : g_service) it->RunSevice(time_elapse, timenow);
    for (auto& it : g_service2) it->RunSevice(time_elapse, timenow);
}
bool ServiceMgr::Register(ServiceEnum typ, void* pObj)
{
    return g_service[typ]->Register(pObj, GameApi::TimeMS());
}
void ServiceMgr::UnRegister(ServiceEnum typ, void* pObj)
{
    g_service[typ]->UnRegister(pObj);
}
bool ServiceMgr::Register(ServiceEnum2 typ, weak<GameObject> pObj)
{
    return g_service2[typ]->Register(pObj, GameApi::TimeMS());
}
void ServiceMgr::UnRegister(ServiceEnum2 typ, weak<GameObject> pObj)
{
    g_service2[typ]->UnRegister(pObj);
}

//////////////////////////////////////////////////////////////////////////
// unit test
#include "gtest/gtest.h"

TEST(Service, List)
{
    time_t now = GameApi::TimeMS();
    ServiceMgr::Register(Service_Test_List, (void*)101);
    ServiceMgr::RunAllService(0, now + 4000); printf("\n");

    ServiceMgr::Register(Service_Test_List, (void*)11);
    ServiceMgr::UnRegister(Service_Test_List, (void*)11);
    ServiceMgr::RunAllService(0, now); printf("\n");

    ServiceMgr::Register(Service_Test_List, (void*)12);
    ServiceMgr::RunAllService(0, now + 2000); printf("\n");
    ServiceMgr::UnRegister(Service_Test_List, (void*)12);

    ServiceMgr::Register(Service_Test_List, (void*)13);
    ServiceMgr::Register(Service_Test_List, (void*)14);
    ServiceMgr::RunAllService(0, now + 4000); printf("\n");

    ServiceMgr::UnRegister(Service_Test_List, (void*)14);
    ServiceMgr::RunAllService(0, now + 6000); printf("\n");
    ServiceMgr::UnRegister(Service_Test_List, (void*)13);
    ServiceMgr::UnRegister(Service_Test_List, (void*)14);

    ServiceMgr::Register(Service_Test_List, (void*)101);
    ServiceMgr::RunAllService(0, now + 4000); printf("\n");
    ServiceMgr::Register(Service_Test_List, (void*)102);
    ServiceMgr::Register(Service_Test_List, (void*)103);
    ServiceMgr::RunAllService(0, now + 4000); printf("\n");
}
TEST(Service, Vec)
{
    time_t now = GameApi::TimeMS();
    ServiceMgr::Register(Service_Test_Vec, (void*)11);
    ServiceMgr::UnRegister(Service_Test_Vec, (void*)11);
    ServiceMgr::RunAllService(0, now); printf("\n");

    ServiceMgr::Register(Service_Test_Vec, (void*)12);
    ServiceMgr::RunAllService(0, now + 2000); printf("\n");
    ServiceMgr::UnRegister(Service_Test_Vec, (void*)12);

    ServiceMgr::Register(Service_Test_Vec, (void*)13);
    ServiceMgr::Register(Service_Test_Vec, (void*)14);
    ServiceMgr::RunAllService(0, now + 4000); printf("\n");

    ServiceMgr::UnRegister(Service_Test_Vec, (void*)14);
    ServiceMgr::RunAllService(0, now + 6000); printf("\n");
    ServiceMgr::UnRegister(Service_Test_Vec, (void*)13);
    ServiceMgr::UnRegister(Service_Test_Vec, (void*)14);

    ServiceMgr::UnRegister(Service_Test_Vec, (void*)12);
    ServiceMgr::UnRegister(Service_Test_Vec, (void*)13);
    ServiceMgr::RunAllService(0, now + 4000); printf("\n");
    ServiceMgr::UnRegister(Service_Test_Vec, (void*)12);
    ServiceMgr::UnRegister(Service_Test_Vec, (void*)13);

    ServiceMgr::Register(Service_Test_Vec, (void*)101);
    ServiceMgr::RunAllService(0, now + 4000); printf("\n");
    ServiceMgr::Register(Service_Test_Vec, (void*)102);
    ServiceMgr::Register(Service_Test_Vec, (void*)103);
    ServiceMgr::RunAllService(0, now + 4000); printf("\n");
}
TEST(Service, Map)
{
    time_t now = GameApi::TimeMS();
    ServiceMgr::Register(Service_Test_Map, (void*)11);
    ServiceMgr::UnRegister(Service_Test_Map, (void*)11);
    ServiceMgr::RunAllService(0, now); printf("\n");

    ServiceMgr::Register(Service_Test_Map, (void*)12);
    ServiceMgr::RunAllService(0, now + 2000); printf("\n");

    ServiceMgr::Register(Service_Test_Map, (void*)13);
    ServiceMgr::Register(Service_Test_Map, (void*)14);
    ServiceMgr::RunAllService(0, now + 4000); printf("\n");

    ServiceMgr::UnRegister(Service_Test_Map, (void*)14);
    ServiceMgr::RunAllService(0, now + 6000); printf("\n");

    ServiceMgr::UnRegister(Service_Test_Map, (void*)12);
    ServiceMgr::UnRegister(Service_Test_Map, (void*)13);
    ServiceMgr::RunAllService(0, now + 4000); printf("\n");

    ServiceMgr::Register(Service_Test_Map, (void*)101);
    ServiceMgr::Register(Service_Test_Map, (void*)102);
    ServiceMgr::RunAllService(0, now + 4000); printf("\n");
    ServiceMgr::Register(Service_Test_Map, (void*)103);
    ServiceMgr::RunAllService(0, now + 4000); printf("\n");
}
TEST(Service, Patch) //10s内平均调用
{
    time_t now = GameApi::TimeMS();
    ServiceMgr::Register(Service_Test_Patch, (void*)11);
    ServiceMgr::Register(Service_Test_Patch, (void*)12);
    ServiceMgr::Register(Service_Test_Patch, (void*)13);
    ServiceMgr::Register(Service_Test_Patch, (void*)14);
    ServiceMgr::Register(Service_Test_Patch, (void*)15);

    ServiceMgr::Register(Service_Test_Patch, (void*)101);
    ServiceMgr::Register(Service_Test_Patch, (void*)102);
    ServiceMgr::RunAllService(0, now + 4000); printf("\n");
    ServiceMgr::Register(Service_Test_Patch, (void*)103);
    ServiceMgr::RunAllService(0, 4000); printf("\n");
}