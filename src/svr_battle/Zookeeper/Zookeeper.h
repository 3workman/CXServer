#pragma once
#include "RpcClient.h"

class Zookeeper : public RpcClient {
public:
    Rpc_For_Zookeeper;
public:
    static Zookeeper& Instance() { static Zookeeper T; return T; }
    Zookeeper();

    RpcClient* GetCross() const;

protected:
    void _OnConnect() override;

private:
    std::map<int, shared<RpcClient>> m_cross;
};
#define sZookeeper Zookeeper::Instance()