/***********************************************************************
* @ 批量执行函数调用，【单线程】循环框架
* @ brief
	1、cServicePatch：多长时间跑完一轮，会动态平衡（如：实现十五分钟全服玩家入库），且会补帧（卡顿后的会多次执行）

	2、cServiceList/cServiceMap：间隔多久执行，不补帧，每帧最多跑一次
        *、注册的回调函数返回值即CD，0-中断，能动态指定间隔周期
        *、cServiceList按逻辑先后顺序执行，cServiceMap则是按时间顺序

    3、Service是为不同对象计时执行同一函数，定时器是计时执行各种不同的注册函数。
        *、它们设计思路一致的，构造exeTime有序表，逐帧检查执行
        *、一旦某个节点未超时，那后面的就都未超时，随即结束循环

* @ Notice
    1、ServicePatch、ServiceList、ServiceVec等有游标的，在RunSevice中须【先刷新游标再调函数】

* @ author zhoumf
* @ date 2014-12-16
************************************************************************/
#pragma once
#include <set>
#include <map>
#include <list>
#include <vector>

template <typename T>
inline bool service_obj_equal (const T& lhs, const T& rhs)
{
    return lhs == rhs;
}

inline bool service_obj_equal(const weak<GameObject>& lhs, const weak<GameObject>& rhs)
{
    return lhs.get() == rhs.get();
}


template <typename T>
class iService {
protected:
    typedef std::multimap<time_t, T>    MapTimer;
    typedef std::pair<time_t, T>        TimerPair;
    typedef std::list<TimerPair>        ListTimer;
    using   ItListTimer = typename ListTimer::iterator;
    using   ItSet = typename std::set<T>::iterator;

public:
    typedef void(*RefreshFun)(T, uint);
    
    virtual ~iService() {};
    virtual void UnRegister(T pObj) = 0;
    virtual bool Register(T pObj, time_t exeTime = 0) = 0; //哪个时刻执行
    virtual void RunSevice(uint time_elapse, time_t timenow) = 0; //循环内的回调函数(m_func)可能调到Register、UnRegister
    
    bool        m_bRun;
    RefreshFun  m_func;  //回调函数
    
    iService(RefreshFun func) : m_bRun(false) { m_func = func; }
};

template<typename T>
class ServicePatch : public iService<T> {
    using RefreshFun = typename iService<T>::RefreshFun;

    int  m_runPos = 0;
    uint m_timeWait = 0;
    const uint kTimeAll;  //多少ms全部运行一次
    std::vector<T> m_aObj;
public:
    ServicePatch(RefreshFun func, uint timeAll) : iService<T>(func), kTimeAll(timeAll) {}

    void UnRegister(T pObj){
        for (int i = 0; i < (int)m_aObj.size(); ++i) {
            if (m_aObj[i] == pObj) {
                m_aObj.erase(m_aObj.begin() + i);
                if (i < m_runPos) --m_runPos;
                else if (m_runPos >= (int)m_aObj.size()) m_runPos = 0;
                break;
            }
        }
    }
    bool Register(T pObj, time_t /*exeTime*/) {
        m_aObj.push_back(pObj);
        return true;
    }
    void RunSevice(uint time_elapse, time_t /*timenow*/)
    {
        if (m_aObj.size() <= 0) return;

        /******************************负载均衡算法********************************/
        m_timeWait += time_elapse;
        const int runNum = m_timeWait * (int)m_aObj.size() / kTimeAll; //单位时长里要处理的个数，可能大于列表中obj总数，比如服务器卡顿很久，得追帧
        if (runNum == 0) return;

        int temp = kTimeAll / (int)m_aObj.size();//处理一个的时长
        m_timeWait = temp ? m_timeWait % temp : 0;//更新等待时间(须小于"处理一个的时长")：对"处理一个的时长"取模(除法的非零保护)
        /**************************************************************************/

        this->m_bRun = true;
        for (int i = 0; i < runNum; ++i){
            auto it = m_aObj.begin() + m_runPos;
            if (++m_runPos >= (int)m_aObj.size()) m_runPos = 0; //到末尾了，回到队头
            this->m_func(*it, 0);
        }
        this->m_bRun = false;
    }
};

//如果后面项的时间比前面项的时间小，那么在前面项没有被pop_front前，后面项无法执行到，则会出问题
//正常使用不会出现此类情形，因为执行的同一函数，后注册的Obj一定更晚，exeTime更小
template <typename T>
class ServiceList : public iService<T> {
    using RefreshFun = typename iService<T>::RefreshFun;
    using ListTimer = typename iService<T>::ListTimer;
    using ItListTimer = typename iService<T>::ItListTimer;
    using TimerPair = typename iService<T>::TimerPair;
    
    ListTimer   m_list;
    ItListTimer m_runIt;
    const uint  m_timeCD;
public:
	ServiceList(RefreshFun func, uint msec) : iService<T>(func), m_timeCD(msec){
        m_runIt = m_list.begin();
    }
	bool Register(T pObj, time_t exeTime){
		m_list.push_back(TimerPair(exeTime,pObj)); //list结构，放到最后面，在Run时调了Reg也没关系
        if (m_runIt == m_list.end()) m_runIt = m_list.begin();
		return true;
	}
	void UnRegister(T pObj){
        if (m_runIt != m_list.end() && service_obj_equal(m_runIt->second, pObj)) {
            m_runIt = m_list.erase(m_runIt);//删自己，迭代器到下一个
        } else {
                                            //删别人，直接erase，循环中++m_runIt安全【vector需考虑在m_runIt前还是后，删前面要挪动...vectro用索引替换it吧】
            for (auto it = m_list.begin(); it != m_list.end(); ++it)
            {
                if (service_obj_equal(it->second, pObj)) { m_list.erase(it); break; }
            }
        }
    }
	void RunSevice(uint /*time_elapse*/, time_t timenow) {
        this->m_bRun = true;
        while (m_runIt != m_list.end()) {
            TimerPair& it = *m_runIt;//m_func后it可能失效
            if (it.first <= timenow) {
                if (++m_runIt == m_list.end()) m_runIt = m_list.begin();
                uint timeDiff = uint(timenow-it.first)+m_timeCD;
                it.first = timenow + m_timeCD;
                this->m_func(it.second, timeDiff);//里头可能把自己删掉，m_runIt指向改变，it可能失效
            } else break;
        }
        this->m_bRun = false;
	}
};

template <typename T>
class ServiceVec : public iService<T> {
    using RefreshFun = typename iService<T>::RefreshFun;
    using ListTimer = typename iService<T>::ListTimer;
    using ItListTimer = typename iService<T>::ItListTimer;
    using TimerPair = typename iService<T>::TimerPair;
    
    int  m_runPos = 0;
    const uint m_timeCD;
    std::vector<TimerPair> m_vec;
public:
    ServiceVec(RefreshFun func, uint msec) : iService<T>(func), m_timeCD(msec) {}

    bool Register(T pObj, time_t exeTime) {
        m_vec.push_back(TimerPair(exeTime, pObj));
        return true;
    }
    void UnRegister(T pObj) {
        for (int i = 0; i < (int)m_vec.size(); ++i) {
            if (m_vec[i].second == pObj) {
                m_vec.erase(m_vec.begin() + i);
                if (i < m_runPos) --m_runPos;
                else if (m_runPos >= (int)m_vec.size()) m_runPos = 0;
                break;
            }
        }
    }
    void RunSevice(uint /*time_elapse*/, time_t timenow) {
        this->m_bRun = true;
        while (m_runPos < (int)m_vec.size()) {
            TimerPair& it = m_vec[m_runPos];
            if (it.first <= timenow) {
                if (++m_runPos >= (int)m_vec.size()) m_runPos = 0;
                uint timeDiff = uint(timenow-it.first)+m_timeCD;
                it.first = timenow + m_timeCD;
                this->m_func(it.second, timeDiff);//里头可能把自己删掉，m_runIt指向改变，it可能失效
            }
            else break;
        }
        this->m_bRun = false;
    }
};

template <typename T>
class ServiceMap : public iService<T> {
    using RefreshFun = typename iService<T>::RefreshFun;
    using ListTimer = typename iService<T>::ListTimer;
    using ItListTimer = typename iService<T>::ItListTimer;
    using ItSet = typename iService<T>::ItSet;
    using TimerPair = typename iService<T>::TimerPair;
    using MapTimer = typename iService<T>::MapTimer;
    
    MapTimer m_map;
    MapTimer m_mapAdd;
    std::set<T> m_setDel;
    const uint  m_timeCD;
public:
    ServiceMap(RefreshFun func, uint msec) : iService<T>(func), m_timeCD(msec) {}

	bool Register(T pObj, time_t exeTime) {
        this->m_bRun ? m_mapAdd.insert(TimerPair(exeTime, pObj)) : m_map.insert(TimerPair(exeTime, pObj));
        return true;
	}
	void UnRegister(T pObj) {
		for (auto& it : m_map) {
            if (it.second == pObj) m_setDel.insert(pObj);
		}
	}
	void RunSevice(uint /*time_elapse*/, time_t timenow) {
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

		this->m_bRun = true;
        for (auto it = m_map.begin(); it != m_map.end(); ) {
			if (it->first <= timenow) { //mutilpymap所以要=号
                if (m_setDel.find(it->second) != m_setDel.end()){
                    it = m_map.erase(it);
                    continue;
                }
				this->m_func(it->second, uint(timenow-it->first)+m_timeCD);
                m_mapAdd.insert(TimerPair(timenow+m_timeCD, it->second));
				it = m_map.erase(it);
			} else {
				//++it; //map以时间作key，自动按从小到大排序，本次的it->first > timenow那后面的会全大于，不必继续循环了
                break;
			}
		}
        m_map.insert(m_mapAdd.begin(), m_mapAdd.end());
        m_mapAdd.clear();
        this->m_bRun = false;
	}
};
