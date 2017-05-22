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
    virtual void RunSevice(uint time_elapse, uint timenow) = 0; //ѭ���ڵĻص�����(m_func)���ܵ���Register��UnRegister
protected:
    typedef std::multimap<uint, void*> mapTimer;
    typedef mapTimer::iterator itMapTimer;
    typedef std::pair<uint, void*> TimerPair;
    typedef std::list<TimerPair> listTimer;
    typedef listTimer::iterator itListTimer;
    typedef std::set<void*>::iterator ItSet;
    typedef std::vector<void*>::iterator ItVec;

	bool m_bRun;
	RefreshFun m_func;	//�ص�����
    iService(RefreshFun func) : m_bRun(false) { m_func = func; }
	virtual ~iService(){};
};

class cServicePatch: public iService{
	uint m_timeWait;
	int  m_iRunPos;  //��ǰRun����λ��(�α�)
	const uint c_timeAll;  //����msȫ������һ��
    std::vector<void*> m_aObj;
    std::vector<void*> m_vecAdd;
    std::set<void*> m_setDel;
public:
	cServicePatch(RefreshFun func, uint timeAll) : iService(func), c_timeAll(timeAll), m_iRunPos(0), m_timeWait(0){}

	void UnRegister(void* pObj){
        for (ItVec it = m_aObj.begin(); it != m_aObj.end(); ++it)
        {
            if (*it == pObj) m_setDel.insert(pObj);
		}
	}
	bool Register(void* pObj, uint /*exeTime*/){
		m_bRun ? m_vecAdd.push_back(pObj) : m_aObj.push_back(pObj);
		return true;
	}
    void RunSevice(uint time_elapse, uint /*timenow*/)
	{
        m_aObj.insert(m_aObj.end(), m_vecAdd.begin(), m_vecAdd.end());
		m_vecAdd.clear();

        ItVec itBegin = m_aObj.begin();
        for (ItVec it = itBegin; it != m_aObj.end() && !m_setDel.empty();){
            if (m_setDel.find(*it) != m_setDel.end())
            {
                if (it-itBegin < m_iRunPos) { --m_iRunPos; }
                it = m_aObj.erase(it);
            }
            else ++it;
        }
		m_setDel.clear();

        const int kItemCount = m_aObj.size();
        if (kItemCount <= 0) return;

		/******************************���ؾ����㷨********************************/
        m_timeWait += time_elapse;
        const int runNum = m_timeWait * kItemCount / c_timeAll; //��λʱ����Ҫ����ĸ��������ܴ����б���obj������������������ٺܾã���׷֡
        if (runNum == 0) return;
		
        int temp = c_timeAll / kItemCount;//����һ����ʱ��
		m_timeWait = temp ? m_timeWait % temp : 0;//���µȴ�ʱ��(��С��"����һ����ʱ��")����"����һ����ʱ��"ȡģ(�����ķ��㱣��)
		/**************************************************************************/

        m_bRun = true;
        for (int i = 0; i < runNum; ++i){
            ItVec it = m_aObj.begin() + m_iRunPos;
            if (++m_iRunPos == kItemCount) m_iRunPos = 0; //��ĩβ�ˣ��ص���ͷ
            if (m_setDel.find(*it) != m_setDel.end()){
                continue;
            }
            m_func(*it);
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
    }
	bool Register(void* pObj, uint exeTime){
		m_list.push_back(TimerPair(exeTime,pObj)); //list�ṹ���ŵ�����棬��Runʱ����RegҲû��ϵ
		return true;
	}
	void UnRegister(void* pObj){
        if (m_runIt->second == pObj) {
            m_runIt = m_list.erase(m_runIt);//ɾ�Լ�������������һ��
            --m_runIt;                      //�ص���һ����ѭ����++m_runIt��Ų������beginʱ���
        } else {
                                            //ɾ���ˣ�ֱ��erase��ѭ����++m_runIt��ȫ
            for (itListTimer it = m_list.begin(); it != m_list.end(); ++it)
            {
                if (it->second == pObj) { m_list.erase(it); break; }
            }
        }
    }
	void RunSevice(uint /*time_elapse*/, uint timenow){
		m_bRun = true;
        m_runIt = ++ m_list.begin(); //������ͷ��㡿
        while (m_runIt != m_list.end())
        {
            if (m_runIt->first <= timenow) {
                void* runObj = m_runIt->second;
                uint nextTime = m_func(runObj);//��ͷ���ܰ��Լ�ɾ����m_runItָ��ı�
                if (m_runIt->second == runObj) m_runIt->first = timenow + nextTime;
                ++m_runIt;
            } else
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
        //if (m_nLock) printf("cServiceMap:Register:LockError ,%d\n", m_eService);
        m_map.insert(TimerPair(exeTime, pObj));
		return true;
	}
	void UnRegister(void* pObj){
		for (itMapTimer it = m_map.begin(); it != m_map.end(); ++it)
        {
            if (it->second == pObj) m_setDel.insert(pObj);
		}
	}
	void RunSevice(uint /*time_elapse*/, uint timenow){
        ItSet itSet;
        for (itMapTimer it = m_map.begin(); it != m_map.end() && !m_setDel.empty(); ){
            itSet = m_setDel.find(it->second);
            if (itSet != m_setDel.end()){
                m_setDel.erase(itSet);
                it = m_map.erase(it);
            }
            else ++it;
        }
        m_setDel.clear();

		m_bRun = true;
		for (itMapTimer it = m_map.begin(); it != m_map.end(); ){
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