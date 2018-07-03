#pragma once

class GameObject;

enum ServiceEnum { // void*
    Service_Test_Patch,
    Service_Test_List,
    Service_Test_Vec,
    Service_Test_Map,
    Service_Room_GameLoop,
    Service_Net_Stats,
    Service_Robot,
};

// 强引用的shared_ptr会出现GameWorld被销毁，GameObject仍有效的情况，Service_Func中须额外判断GameWorld非空
// Service内部有比较操作，weak_ptr满足编译要求
enum ServiceEnum2 { // shared<GameObject>
    Service_TrapPoison,
    Service_GroupTreat,
    Service_Position,
    Service_Player_Position, //有客户端预测
    //Service_Buff,
};

namespace ServiceMgr {
    void RunAllService(uint time_elapse, time_t timenow);

    bool Register(ServiceEnum typ, void* pObj);
    void UnRegister(ServiceEnum typ, void* pObj);

    bool Register(ServiceEnum2 typ, weak<GameObject> pObj);
    void UnRegister(ServiceEnum2 typ, weak<GameObject> pObj);
};
