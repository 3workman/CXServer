#include "stdafx.h"
#include "ServiceMgr.h"
#include "Service.h"

uint _Service_Patch(void* p) { return printf("Service Path(%d)...\n", (int)p); }
uint _Service_List(void* p) { printf("Service List(%d)...\n", (int)p); return 2000; }
uint _Service_Vec(void* p) { printf("Service Vec(%d)...\n", (int)p); return 2000; }
uint _Service_Map(void* p) { printf("Service Map(%d)...\n", (int)p); return 2000; }

extern uint _Service_Sync_Position(void* p);

static iService* g_aService[_Service_Max] = {
    new cServicePatch(_Service_Patch, 10000),
    new cServiceList(_Service_List),
    new cServiceVec(_Service_Vec),
    new cServiceMap(_Service_Map),

    new cServiceList(_Service_Sync_Position),
};
void ServiceMgr::RunAllService(uint time_elapse, time_t timenow)
{
    for (auto& it : g_aService) it->RunSevice(time_elapse, timenow);
}
void ServiceMgr::UnRegister(ServiceEnum typ, void* pObj)
{
    g_aService[typ]->UnRegister(pObj);
}
bool ServiceMgr::Register(ServiceEnum typ, void* pObj)
{
    return g_aService[typ]->Register(pObj);
}

//////////////////////////////////////////////////////////////////////////
// unit test
#include "tool/UnitTest.h"
TEST_CASE(ServiceList)
{
    ServiceMgr::Register(Service_Test_List, (void*)11);
    ServiceMgr::UnRegister(Service_Test_List, (void*)11);
    ServiceMgr::RunAllService(0, 0); printf("\n");

    ServiceMgr::Register(Service_Test_List, (void*)12);
    ServiceMgr::RunAllService(0, 2000); printf("\n");

    ServiceMgr::Register(Service_Test_List, (void*)13);
    ServiceMgr::Register(Service_Test_List, (void*)14);
    ServiceMgr::RunAllService(0, 4000); printf("\n");

    ServiceMgr::UnRegister(Service_Test_List, (void*)14);
    ServiceMgr::RunAllService(0, 6000); printf("\n");

    ServiceMgr::UnRegister(Service_Test_List, (void*)12);
    ServiceMgr::UnRegister(Service_Test_List, (void*)13);
}
TEST_CASE(ServiceVec)
{
    ServiceMgr::Register(Service_Test_Vec, (void*)11);
    ServiceMgr::UnRegister(Service_Test_Vec, (void*)11);
    ServiceMgr::RunAllService(0, 0); printf("\n");

    ServiceMgr::Register(Service_Test_Vec, (void*)12);
    ServiceMgr::RunAllService(0, 2000); printf("\n");

    ServiceMgr::Register(Service_Test_Vec, (void*)13);
    ServiceMgr::Register(Service_Test_Vec, (void*)14);
    ServiceMgr::RunAllService(0, 4000); printf("\n");

    ServiceMgr::UnRegister(Service_Test_Vec, (void*)14);
    ServiceMgr::RunAllService(0, 6000); printf("\n");

    ServiceMgr::UnRegister(Service_Test_Vec, (void*)12);
    ServiceMgr::UnRegister(Service_Test_Vec, (void*)13);
}
TEST_CASE(ServiceMap)
{
    ServiceMgr::Register(Service_Test_Map, (void*)11);
    ServiceMgr::UnRegister(Service_Test_Map, (void*)11);
    ServiceMgr::RunAllService(0, 0); printf("\n");

    ServiceMgr::Register(Service_Test_Map, (void*)12);
    ServiceMgr::RunAllService(0, 2000); printf("\n");

    ServiceMgr::Register(Service_Test_Map, (void*)13);
    ServiceMgr::Register(Service_Test_Map, (void*)14);
    ServiceMgr::RunAllService(0, 4000); printf("\n");

    ServiceMgr::UnRegister(Service_Test_Map, (void*)14);
    ServiceMgr::RunAllService(0, 6000); printf("\n");

    ServiceMgr::UnRegister(Service_Test_Map, (void*)12);
    ServiceMgr::UnRegister(Service_Test_Map, (void*)13);
}
TEST_CASE(ServicePatch) //10s内平均调用
{
    ServiceMgr::Register(Service_Test_Patch, (void*)11);
    ServiceMgr::Register(Service_Test_Patch, (void*)12);
    ServiceMgr::Register(Service_Test_Patch, (void*)13);
    ServiceMgr::Register(Service_Test_Patch, (void*)14);
    ServiceMgr::Register(Service_Test_Patch, (void*)15);
}