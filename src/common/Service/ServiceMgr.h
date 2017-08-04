#pragma once

enum ServiceEnum {
    Service_Test_Patch,
    Service_Test_List,
    Service_Test_Vec,
    Service_Test_Map,

    Service_Sync_Position,

    _Service_Max
};
namespace ServiceMgr {
    void RunAllService(uint time_elapse, time_t timenow);
    void UnRegister(ServiceEnum typ, void* pObj);
    bool Register(ServiceEnum typ, void* pObj);
};