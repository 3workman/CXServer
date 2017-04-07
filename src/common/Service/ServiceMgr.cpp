#include "stdafx.h"
#include "ServiceMgr.h"
#include "Service.h"

DWORD Buff(void* p){ printf("Service List Buff(%d)...\n", p); return 2000; }
DWORD NPCAI(void* p){ return printf("Service Path NPCAI(%d)...\n", p); }
DWORD PlayerSave(void* p){ return printf("Service Path PlayerSave(%d)...\n", p); }
extern DWORD _Service_Sync_Position(void* p);

iService* ServiceMgr::m_aService[_Service_Max] = {
    /* Service_Buff */new cServiceList(Buff),
    /* Service_NPCAI */new cServicePatch(NPCAI, 1000),
    /* Service_PlayerSave */new cServicePatch(PlayerSave, 3 * 1000),
    /* Service_Sync_Position */new cServiceList(_Service_Sync_Position),
};
void ServiceMgr::RunAllService(DWORD time_elasped, DWORD timenow)
{
    for (auto& it : m_aService) it->RunSevice(time_elasped, timenow);
}
bool ServiceMgr::UnRegister(ServiceEnum typ, ServiceObj* pObj)
{
    return m_aService[typ]->UnRegister(pObj);
}
bool ServiceMgr::Register(ServiceEnum typ, ServiceObj* pObj)
{
    return m_aService[typ]->Register(pObj);
}