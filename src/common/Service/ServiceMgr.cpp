#include "stdafx.h"
#include "ServiceMgr.h"
#include "Service.h"

uint _Service_Patch(void* p) { return printf("Service Path(%p)...\n", p); }
uint _Service_List(void* p) { printf("Service List(%p)...\n", p); return 2000; }
uint _Service_Vec(void* p) { printf("Service Vec(%p)...\n", p); return 2000; }
uint _Service_Map(void* p) { printf("Service Map(%p)...\n", p); return 2000; }

extern uint _Service_Room_GameLoop(void* p);

static iService* g_aService[_Service_Max] = {
    new cServicePatch(_Service_Patch, 10000),
    new cServiceList(_Service_List),
    new cServiceVec(_Service_Vec),
    new cServiceMap(_Service_Map),

    new cServiceVec(_Service_Room_GameLoop),
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