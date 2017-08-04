/***********************************************************************
* @ 批量执行函数调用，【单线程】循环框架
* @ brief
	1、cServicePatch：多长时间跑完一轮，会动态平衡（如：实现十五分钟全服玩家入库），且会补帧（卡顿后的会多次执行）

	2、cServiceList/cServiceMap：间隔多久执行，不补帧，每帧最多跑一次
        *、注册的回调函数返回值即CD，0-中断，能动态指定间隔周期
        *、cServiceList按逻辑先后顺序执行，cServiceMap则是按时间顺序
        *、均是将执行过的对象挪至队尾，所以List的Run性能比Map好，但频繁增删的话，Map就优势了

    3、Service是为不同对象计时执行同一函数，定时器是计时执行各种不同的注册函数。
        *、它们设计思路一致的，构造exeTime有序表，逐帧检查执行
        *、一旦某个节点未超时，那后面的就都未超时，随即结束循环

* @ author zhoumf
* @ date 2014-12-16
************************************************************************/
#pragma once
#include <set>
#include <map>
#include <list>
#include <vector>

class iService{
public:
    typedef uint(*RefreshFun)(void*);
    virtual void UnRegister(void* pObj) = 0;
    virtual bool Register(void* pObj, uint exeTime = 0) = 0; //哪个时刻执行
    virtual void RunSevice(uint time_elapse, time_t timenow) = 0; //循环内的回调函数(m_func)可能调到Register、UnRegister
protected:
    typedef std::multimap<time_t, void*>    mapTimer;
    typedef std::pair<time_t, void*>        TimerPair;
    typedef std::list<TimerPair>            listTimer;
    typedef listTimer::iterator             itListTimer;
    typedef std::set<void*>::iterator       ItSet;

	bool m_bRun;
	RefreshFun m_func;	//回调函数
    iService(RefreshFun func) : m_bRun(false) { m_func = func; }
	virtual ~iService(){};
};

class cServicePatch: public iService{
    int  m_runPos = 0;
    uint m_timeWait = 0;
    const uint c_timeAll;  //多少ms全部运行一次
    std::vector<void*> m_aObj;
public:
    cServicePatch(RefreshFun func, uint timeAll) : iService(func), c_timeAll(timeAll){}

    void UnRegister(void* pObj){
        for (int i = 0; i < (int)m_aObj.size(); ++i) {
            if (m_aObj[i] == pObj) {
                m_aObj.erase(m_aObj.begin() + i);
                if (i < m_runPos) --m_runPos;
                break;
            }
        }
    }
    bool Register(void* pObj, uint /*exeTime*/){
        m_aObj.push_back(pObj);
        return true;
    }
    void RunSevice(uint time_elapse, time_t /*timenow*/)
    {
        if (m_aObj.size() <= 0) return;

        /******************************负载均衡算法********************************/
        m_timeWait += time_elapse;
        const int runNum = m_timeWait * m_aObj.size() / c_timeAll; //单位时长里要处理的个数，可能大于列表中obj总数，比如服务器卡顿很久，得追帧
        if (runNum == 0) return;

        int temp = c_timeAll / m_aObj.size();//处理一个的时长
        m_timeWait = temp ? m_timeWait % temp : 0;//更新等待时间(须小于"处理一个的时长")：对"处理一个的时长"取模(除法的非零保护)
        /**************************************************************************/

        m_bRun = true;
        for (int i = 0; i < runNum; ++i){
            auto it = m_aObj.begin() + m_runPos;
            m_func(*it);
            if (++m_runPos == m_aObj.size()) m_runPos = 0; //到末尾了，回到队头
        }
        m_bRun = false;
    }
};

//如果后面项的时间比前面项的时间小，那么在前面项没有被pop_front前，后面项无法执行到，则会出问题
//正常使用不会出现此类情形，因为执行的同一函数，后注册的Obj一定更晚，exeTime更小
class cServiceList: public iService{
    listTimer   m_list;
    itListTimer m_runIt;
public:
	cServiceList(RefreshFun func) : iService(func) {
        m_list.push_back(TimerPair(0, NULL)); //【填充头结点，防止--m_runIt宕机】
        m_runIt = m_list.end();
    }
	bool Register(void* pObj, uint exeTime){
		m_list.push_back(TimerPair(exeTime,pObj)); //list结构，放到最后面，在Run时调了Reg也没关系
		return true;
	}
	void UnRegister(void* pObj){
        if (m_runIt != m_list.end() && m_runIt->second == pObj) {
            m_runIt = m_list.erase(m_runIt);//删自己，迭代器到下一个
            --m_runIt;                      //回到上一个，循环中++m_runIt再挪回来【begin时会跪】
        } else {
                                            //删别人，直接erase，循环中++m_runIt安全【vector需考虑在m_runIt前还是后，删前面要挪动...vectro用索引替换it吧】
            for (itListTimer it = m_list.begin(); it != m_list.end(); ++it)
            {
                if (it->second == pObj) { m_list.erase(it); break; }
            }
        }
    }
	void RunSevice(uint /*time_elapse*/, time_t timenow){
        if (m_runIt == m_list.end()) m_runIt = ++m_list.begin(); //【跳过头结点】
        m_bRun = true;
        while (m_runIt != m_list.end()) {
            if (m_runIt->first <= timenow) {
                void* runObj = m_runIt->second;
                uint nextTime = m_func(runObj);//里头可能把自己删掉，m_runIt指向改变
                if (m_runIt->second == runObj) m_runIt->first = timenow + nextTime;
                if (++m_runIt == m_list.end()) m_runIt = ++m_list.begin();
            } else
                break;
        }
        m_bRun = false;
	}
};
class cServiceVec : public iService {
    int  m_runPos = 0;
    std::vector<TimerPair> m_vec;
public:
    cServiceVec(RefreshFun func) : iService(func) {}

    bool Register(void* pObj, uint exeTime) {
        m_vec.push_back(TimerPair(exeTime, pObj));
        return true;
    }
    void UnRegister(void* pObj) {
        for (int i = 0; i < (int)m_vec.size(); ++i) {
            if (m_vec[i].second == pObj) {
                m_vec.erase(m_vec.begin() + i);
                if (i < m_runPos) --m_runPos;
                break;
            }
        }
    }
    void RunSevice(uint /*time_elapse*/, time_t timenow) {
        m_bRun = true;
        while (m_runPos != m_vec.size()) {
            TimerPair& it = m_vec[m_runPos];
            if (it.first <= timenow) {
                void* runObj = it.second;
                uint nextTime = m_func(runObj);//里头可能把自己删掉，m_runIt指向改变
                if (it.second == runObj) it.first = timenow + nextTime;
                if (++m_runPos == m_vec.size()) m_runPos = 0;
            }
            else
                break;
        }
        m_bRun = false;
    }
};
class cServiceMap : public iService{
    mapTimer m_map;
    mapTimer m_mapAdd;
    std::set<void*> m_setDel;
public:
    cServiceMap(RefreshFun func) : iService(func){}

	bool Register(void* pObj, uint exeTime){
        m_bRun ? m_mapAdd.insert(TimerPair(exeTime, pObj)) : m_map.insert(TimerPair(exeTime, pObj));
        return true;
	}
	void UnRegister(void* pObj){
		for (auto it = m_map.begin(); it != m_map.end(); ++it)
        {
            if (it->second == pObj) m_setDel.insert(pObj);
		}
	}
	void RunSevice(uint /*time_elapse*/, time_t timenow){
        ItSet itSet;
        for (auto it = m_map.begin(); it != m_map.end() && !m_setDel.empty();){
            itSet = m_setDel.find(it->second);
            if (itSet != m_setDel.end()){
                m_setDel.erase(itSet);
                it = m_map.erase(it);
            }
            else ++it;
        }
        m_setDel.clear();

		m_bRun = true;
        for (auto it = m_map.begin(); it != m_map.end();){
			if (it->first <= timenow)//mutilpymap所以要=号
			{
                if (m_setDel.find(it->second) != m_setDel.end()){
                    it = m_map.erase(it);
                    continue;
                }
				uint nextTime = m_func(it->second);
                if (nextTime > 0) m_mapAdd.insert(TimerPair(timenow + nextTime, it->second));
				it = m_map.erase(it);
			}else{
				//++it; //map以时间作key，自动按从小到大排序，本次的it->first > timenow那后面的会全大于，不必继续循环了
                break;
			}
		}
        m_map.insert(m_mapAdd.begin(), m_mapAdd.end());
        m_mapAdd.clear();
        m_bRun = false;
	}
};