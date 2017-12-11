#pragma once
#include "RpcClient.h"

class Zookeeper : public RpcClient {
public:
    Rpc_For_Zookeeper;
public:
    static Zookeeper& Instance() { static Zookeeper T; return T; }
    Zookeeper();
protected:
    void _OnConnect() override;

private:
    std::vector<shared<RpcClient>> m_nodes;
};
#define sZookeeper Zookeeper::Instance()