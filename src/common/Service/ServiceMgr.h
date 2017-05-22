#pragma once

enum ServiceEnum {
    Service_Buff,
    Service_NPCAI,
    Service_PlayerSave,
    Service_Sync_Position,

    _Service_Max
};
class iService;
class ServiceMgr {
    static iService* m_aService[_Service_Max];
public:
    static void RunAllService(uint time_elapse, uint timenow);
    static void UnRegister(ServiceEnum typ, void* pObj);
    static bool Register(ServiceEnum typ, void* pObj);
};