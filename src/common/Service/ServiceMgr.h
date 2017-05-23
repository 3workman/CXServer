#pragma once

enum ServiceEnum {
    Service_Buff,
    Service_NPCAI,
    Service_PlayerSave,
    Service_Sync_Position,

    _Service_Max
};
namespace ServiceMgr {
    void RunAllService(uint time_elapse, uint timenow);
    void UnRegister(ServiceEnum typ, void* pObj);
    bool Register(ServiceEnum typ, void* pObj);
};