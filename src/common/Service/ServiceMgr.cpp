#include "stdafx.h"
#include "ServiceMgr.h"
#include "Service.h"

uint _Service_Buff(void* p){ printf("Service List Buff(%d)...\n", p); return 2000; }
uint _Service_NPCAI(void* p){ return printf("Service Path NPCAI(%d)...\n", p); }
uint _Service_PlayerSave(void* p){ return printf("Service Path PlayerSave(%d)...\n", p); }
extern uint _Service_Sync_Position(void* p);

iService* ServiceMgr::m_aService[_Service_Max] = {
    new cServiceList(_Service_Buff),
    new cServicePatch(_Service_NPCAI, 1000),
    new cServicePatch(_Service_PlayerSave, 3 * 1000),
    new cServiceList(_Service_Sync_Position),
};
void ServiceMgr::RunAllService(uint time_elapse, uint timenow)
{
    for (auto& it : m_aService) it->RunSevice(time_elapse, timenow);
}
bool ServiceMgr::UnRegister(ServiceEnum typ, ServiceObj* pObj)
{
    return m_aService[typ]->UnRegister(pObj);
}
bool ServiceMgr::Register(ServiceEnum typ, ServiceObj* pObj)
{
    return m_aService[typ]->Register(pObj);
}