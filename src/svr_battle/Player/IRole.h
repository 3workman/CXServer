#pragma once

class GameObject;

class IRole { //可以是玩家或机器人
public:
    enum Type : uint8 { // 不要改变顺序
        ROLE_HERO = 0,
        ROLE_BOSS = 1,
        ROLE_MINION = 2,
    };
public:
    std::string     m_name;
    std::string     m_prefab;
    uint32          m_roomId = 0;
    uint8           m_teamId = 0;//敌对关系，1起始
    Type            m_roleType = ROLE_HERO;

    //用shared的话，gameObject被删除后，逻辑层的引用依然有效，导致需额外判断 IsDestroyed()
    weak<GameObject> m_gameObject;

public:
    virtual ~IRole() {}

    shared<GameObject> GetMyObj() { return m_gameObject.lock(); }

    // Player/Robot
    template<typename T> T* Get() { return dynamic_cast<T*>(this); }
};
