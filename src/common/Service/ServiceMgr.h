#pragma once

enum ServiceEnum {
    Service_Buff,
    Service_NPCAI,
    Service_PlayerSave,

    Service_Max
};
class iService;
typedef void ServiceObj;
class ServiceMgr {
    static iService* m_aService[Service_Max];
public:
    static void RunAllService(DWORD time_elasped, DWORD timenow);
    static bool UnRegister(ServiceEnum typ, ServiceObj* pObj);
    static bool Register(ServiceEnum typ, ServiceObj* pObj);
};