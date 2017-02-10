/***********************************************************************
* @ 最短路径算法
* @ brief
    1、充当其它业务类的【static】组件，ctor中设置图数据

* @ author zhoumf
* @ date 2016-11-21
************************************************************************/
#pragma once

class Graph{
    enum {
        MAX_NUM = 5,
        INFINITE_LEN = 99999, //当做无穷大
    };
    int _curCnt;
    int _arr[MAX_NUM][MAX_NUM];
public:
    int Distance[MAX_NUM][MAX_NUM];
    int Path[MAX_NUM][MAX_NUM];

    Graph();
    std::vector<uint16> GetPath(uint16 from, uint16 to);
private:
    void _Floyd();
};

Graph::Graph()
{
    ZeroMemoryThis;

    //初始化图数据
    _curCnt = 4; assert(_curCnt <= MAX_NUM);
    _arr[1][0] = 1;
    _arr[0][2] = 1;
    _arr[2][3] = 1;
    _arr[0][3] = 5;

    _Floyd();
}
void Graph::_Floyd()
{
    int i, j, k;
    for (i = 0; i < _curCnt; ++i) //初始化
        for (j = 0; j < _curCnt; ++j)
        {
            if (i == j) {
                Distance[i][j] = 0;
            }
            else {
                Distance[i][j] = _arr[i][j] > 0 ? _arr[i][j] : INFINITE_LEN;
            }

            Path[i][j] = Distance[i][j] < INFINITE_LEN ? j : -1;
        }
    //将各个顶点顺次加入，并修改最短路径
    for (k = 0; k < _curCnt; ++k)
        for (i = 0; i < _curCnt; ++i) //在i，j之间加入k
            for (j = 0; j < _curCnt; ++j)
            {
                if (Distance[i][k] + Distance[k][j] < Distance[i][j]){
                    Distance[i][j] = Distance[i][k] + Distance[k][j];
                    Path[i][j] = Path[i][k]; //更改后继点
                }
            }
}
std::vector<uint16> Graph::GetPath(uint16 from, uint16 to)
{
    std::vector<uint16> retVec;

    if (from < MAX_NUM || to < MAX_NUM && Distance[from][to] < Graph::INFINITE_LEN)
    {
        while (from != to)
        {
            retVec.push_back(from);
            from = Path[from][to];
        }
        retVec.push_back(to);
    }
    return std::move(retVec);
}