/***********************************************************************
* @ ����ִ�к������ã������̡߳�ѭ�����
* @ brief
	1��cServicePatch���೤ʱ������һ�֣��ᶯ̬ƽ�⣨�磺ʵ��ʮ�����ȫ�������⣩���һᲹ֡�����ٺ�Ļ���ִ�У�

	2��cServiceList/cServiceMap��������ִ�У�����֡��ÿ֡�����һ��
        *��ע��Ļص���������ֵ��CD��0-�жϣ��ܶ�ָ̬���������
        *��cServiceList���߼��Ⱥ�˳��ִ�У�cServiceMap���ǰ�ʱ��˳��
        *�����ǽ�ִ�й��Ķ���Ų����β������List��Run���ܱ�Map�ã���Ƶ����ɾ�Ļ���Map��������

    3��Service��Ϊ��ͬ�����ʱִ��ͬһ��������ʱ���Ǽ�ʱִ�и��ֲ�ͬ��ע�ắ����
        *���������˼·һ�µģ�����exeTime�������֡���ִ��
        *��һ��ĳ���ڵ�δ��ʱ���Ǻ���ľͶ�δ��ʱ���漴����ѭ��

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
    virtual bool Register(void* pObj, uint exeTime = 0) = 0; //�ĸ�ʱ��ִ��
    virtual void RunSevice(uint time_elapse, time_t timenow) = 0; //ѭ���ڵĻص�����(m_func)���ܵ���Register��UnRegister
protected:
    typedef std::multimap<time_t, void*>    mapTimer;
    typedef std::pair<time_t, void*>        TimerPair;
    typedef std::list<TimerPair>            listTimer;
    typedef listTimer::iterator             itListTimer;
    typedef std::set<void*>::iterator       ItSet;

	bool m_bRun;
	RefreshFun m_func;	//�ص�����
    iService(RefreshFun func) : m_bRun(false) { m_func = func; }
	virtual ~iService(){};
};

class cServicePatch: public iService{
    int  m_runPos = 0;
    uint m_timeWait = 0;
    const uint c_timeAll;  //����msȫ������һ��
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

        /******************************���ؾ����㷨********************************/
        m_timeWait += time_elapse;
        const int runNum = m_timeWait * m_aObj.size() / c_timeAll; //��λʱ����Ҫ����ĸ��������ܴ����б���obj������������������ٺܾã���׷֡
        if (runNum == 0) return;

        int temp = c_timeAll / m_aObj.size();//����һ����ʱ��
        m_timeWait = temp ? m_timeWait % temp : 0;//���µȴ�ʱ��(��С��"����һ����ʱ��")����"����һ����ʱ��"ȡģ(�����ķ��㱣��)
        /**************************************************************************/

        m_bRun = true;
        for (int i = 0; i < runNum; ++i){
            auto it = m_aObj.begin() + m_runPos;
            m_func(*it);
            if (++m_runPos == m_aObj.size()) m_runPos = 0; //��ĩβ�ˣ��ص���ͷ
        }
        m_bRun = false;
    }
};

//����������ʱ���ǰ�����ʱ��С����ô��ǰ����û�б�pop_frontǰ���������޷�ִ�е�����������
//����ʹ�ò�����ִ������Σ���Ϊִ�е�ͬһ��������ע���Objһ������exeTime��С
class cServiceList: public iService{
    listTimer   m_list;
    itListTimer m_runIt;
public:
	cServiceList(RefreshFun func) : iService(func) {
        m_list.push_back(TimerPair(0, NULL)); //�����ͷ��㣬��ֹ--m_runIt崻���
        m_runIt = m_list.end();
    }
	bool Register(void* pObj, uint exeTime){
		m_list.push_back(TimerPair(exeTime,pObj)); //list�ṹ���ŵ�����棬��Runʱ����RegҲû��ϵ
		return true;
	}
	void UnRegister(void* pObj){
        if (m_runIt != m_list.end() && m_runIt->second == pObj) {
            m_runIt = m_list.erase(m_runIt);//ɾ�Լ�������������һ��
            --m_runIt;                      //�ص���һ����ѭ����++m_runIt��Ų������beginʱ���
        } else {
                                            //ɾ���ˣ�ֱ��erase��ѭ����++m_runIt��ȫ��vector�迼����m_runItǰ���Ǻ�ɾǰ��ҪŲ��...vectro�������滻it�ɡ�
            for (itListTimer it = m_list.begin(); it != m_list.end(); ++it)
            {
                if (it->second == pObj) { m_list.erase(it); break; }
            }
        }
    }
	void RunSevice(uint /*time_elapse*/, time_t timenow){
        if (m_runIt == m_list.end()) m_runIt = ++m_list.begin(); //������ͷ��㡿
        m_bRun = true;
        while (m_runIt != m_list.end()) {
            if (m_runIt->first <= timenow) {
                void* runObj = m_runIt->second;
                uint nextTime = m_func(runObj);//��ͷ���ܰ��Լ�ɾ����m_runItָ��ı�
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
                uint nextTime = m_func(runObj);//��ͷ���ܰ��Լ�ɾ����m_runItָ��ı�
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
			if (it->first <= timenow)//mutilpymap����Ҫ=��
			{
                if (m_setDel.find(it->second) != m_setDel.end()){
                    it = m_map.erase(it);
                    continue;
                }
				uint nextTime = m_func(it->second);
                if (nextTime > 0) m_mapAdd.insert(TimerPair(timenow + nextTime, it->second));
				it = m_map.erase(it);
			}else{
				//++it; //map��ʱ����key���Զ�����С�������򣬱��ε�it->first > timenow�Ǻ���Ļ�ȫ���ڣ����ؼ���ѭ����
                break;
			}
		}
        m_map.insert(m_mapAdd.begin(), m_mapAdd.end());
        m_mapAdd.clear();
        m_bRun = false;
	}
};