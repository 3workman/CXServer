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

typedef void ServiceObj;
typedef uint(*RefreshFun)(ServiceObj*);

class iService{
public:
    virtual bool UnRegister(ServiceObj* pObj) = 0;
    virtual bool Register(ServiceObj* pObj, uint exeTime = 0) = 0; //�ĸ�ʱ��ִ��
    virtual void RunSevice(uint time_elasped, uint timenow) = 0; //ѭ���ڵĻص�����(m_func)���ܵ���Register��UnRegister
protected:
    typedef std::multimap<uint, ServiceObj*> mapTimer;
    typedef mapTimer::iterator itMapTimer;
    typedef std::pair<uint, ServiceObj*> TimerPair;
    typedef std::list<TimerPair> listTimer;
    typedef listTimer::iterator itListTimer;
    typedef std::set<ServiceObj*>::iterator ItSet;
    typedef std::vector<ServiceObj*>::iterator ItVec;

	bool m_bRun;
	RefreshFun m_func;	//�ص�����
    iService(RefreshFun func) : m_bRun(false) { m_func = func; }
	virtual ~iService(){};
};

#ifdef _DEBUG
class cServicePatch : public iService{
private:
	class cServiceArray{
	public:
		cServiceArray():m_nCount(0),m_nTickPos(0)
		{
			m_nCapacity = 8000;
			m_aObj = new ServiceObj*[m_nCapacity];
		}

		~cServiceArray()
		{
			if (m_aObj)
			{
				delete[] m_aObj;
				m_aObj = NULL;
			}
			m_nCapacity = 0;
			m_nCount = 0;
			m_nTickPos = 0;
		}

		int GetCount()const{ return m_nCount; }

		void Clear(){ m_nCount = 0; }

		bool AddObj(ServiceObj *pObj)
		{
			if (m_nCount >= m_nCapacity
				&& !ExtendSpace())
			{
				return false;
			}
			m_aObj[m_nCount++] = pObj;
			return true;
		}

		bool DelObj(ServiceObj *pObj)
		{
			for (int i = 0; i < m_nCount; ++i)
			{
				if (pObj == m_aObj[i])
				{
					memcpy(m_aObj + i, m_aObj + i + 1, sizeof(ServiceObj*) * (m_nCount - i - 1));
					--m_nCount;
					if (m_nTickPos > i)
					{
						--m_nTickPos;
					}
					else if (m_nTickPos >= m_nCount)
					{
						m_nTickPos = 0;
					}
					return true;
				}
			}
			return false;
		}

		bool AddObjs(cServiceArray& rArray)
		{
			if (rArray.m_nCount <= 0) return false;

			if (m_nCount + rArray.m_nCount > m_nCapacity
				&& !ExtendSpace(rArray.m_nCount))
			{
				return false;
			}

			memcpy(m_aObj + m_nCount, rArray.m_aObj, rArray.m_nCount * sizeof(ServiceObj*));
			m_nCount += rArray.m_nCount;
			return true;
		}

		bool DelObjs(cServiceArray& rArray)
		{
			if (rArray.m_nCount <= 0) return false;

			if (rArray.m_nCount > 20)
			{
				static std::set<ServiceObj*> s_objset;
				s_objset.clear();
				for (int i = 0; i < rArray.m_nCount; ++i)
				{
					s_objset.insert(rArray.m_aObj[i]);
				}

				int nWPos = 0;
				int nCount = m_nCount;
				int nTickPosCount = 0;
				for (int i = 0; i < nCount; ++i)
				{
					ItSet it = s_objset.find(m_aObj[i]);
					if (it != s_objset.end())
					{
						if (m_nTickPos > i) ++nTickPosCount;

						s_objset.erase(it);
						--m_nCount;

						if (s_objset.empty())
						{
							if (i < nCount - 1)// ��ɾ�����ˣ�����һ���ƶ��ͺ�
							{
								memcpy(m_aObj + nWPos, m_aObj + i + 1, sizeof(ServiceObj*) * (nCount - i - 1));
							}
							break;
						}
					}else if (nWPos != i){
						m_aObj[nWPos++] = m_aObj[i]; //��ɾ��Ԫ��Ų������ǰ���α����
					}
                    else
                        ++nWPos;
				}

				m_nTickPos -= nTickPosCount;
				if (m_nTickPos >= m_nCount) m_nTickPos = 0;

				return true;
			}else{
				for (int i = 0; i < rArray.m_nCount; ++i)
				{
					DelObj(rArray.m_aObj[i]);
				}
				return true;
			}
		}

		ServiceObj* NextObj()
		{
			if (m_nCount > 0)
			{
				ServiceObj* pObj = m_aObj[m_nTickPos];
				++m_nTickPos;
				if (m_nTickPos >= m_nCount) m_nTickPos = 0;

				return pObj;
			}
			return NULL;
		}
	private:
		bool ExtendSpace(int nNeedCount = 0)
		{
			int nNewCapacity = m_nCapacity;
			nNewCapacity += nNeedCount < nNewCapacity ? nNewCapacity : nNeedCount;

			ServiceObj** paNewObjs = new ServiceObj*[nNewCapacity];
			if (!paNewObjs) return false;

			m_nCapacity = nNewCapacity;
			memcpy(paNewObjs, m_aObj, sizeof(ServiceObj*) * m_nCount);
			delete[] m_aObj;
			m_aObj = paNewObjs;
			return true;
		}
	private:
		int m_nCapacity;		// �ܳ���
		int m_nCount;			// ��ǰʹ�ó���
		int m_nTickPos;			// ָ��
		ServiceObj** m_aObj;	// ����
	};

	uint m_timeWait;
	const uint c_timeAll;//����msȫ������һ��
	cServiceArray m_aObj;
	cServiceArray m_aObjAdd;
	cServiceArray m_aObjDel;
	std::set<ServiceObj*> m_setLockDel;
public:
	cServicePatch(RefreshFun func, uint timeAll) : iService(func), c_timeAll(timeAll), m_timeWait(0){}

	bool UnRegister(ServiceObj* pObj)
	{
		if (m_bRun) m_setLockDel.insert(pObj);

		return m_aObjDel.AddObj(pObj);
	}
	bool Register(ServiceObj* pObj, uint /*exeTime*/)
	{
		return m_bRun ? m_aObjAdd.AddObj(pObj) : m_aObj.AddObj(pObj);
	}
	void RunSevice(uint time_elasped, uint /*timenow*/)
	{
		if (m_aObj.AddObjs(m_aObjAdd)) m_aObjAdd.Clear();
		if (m_aObj.DelObjs(m_aObjDel)) m_aObjDel.Clear();

		const int kItemCount = m_aObj.GetCount();
        if (kItemCount <= 0) return;

		/******************************���ؾ����㷨********************************/
		m_timeWait += time_elasped;
		const int runNum = m_timeWait * kItemCount / c_timeAll; //��λʱ����Ҫ����ĸ��������ܴ����б���obj������������������ٺܾã���׷֡
        if (runNum == 0) return;

		int temp = c_timeAll / kItemCount;//����һ����ʱ��
		m_timeWait = temp ? m_timeWait % temp : 0;//���µȴ�ʱ��(��С��"����һ����ʱ��")����"����һ����ʱ��"ȡģ(�����ķ��㱣��)
		/**************************************************************************/

        m_bRun = true;
        for (int i = 0; i < runNum; ++i){
            if (ServiceObj* pObj = m_aObj.NextObj()){
                if (m_setLockDel.find(pObj) != m_setLockDel.end()){
                    continue;
                }
                m_func(pObj);
            }
		}
        m_setLockDel.clear();
        m_bRun = false;
	}
};
#else
class cServicePatch: public iService{
	uint m_timeWait;
	int   m_iRunPos;  //��ǰRun����λ��(�α�)
	const uint c_timeAll;  //����msȫ������һ��
	std::vector<ServiceObj*> m_aObj;
	std::vector<ServiceObj*> m_vecAdd;
    std::set<ServiceObj*> m_setDel;
public:
	cServicePatch(RefreshFun func, uint timeAll) : iService(func), c_timeAll(timeAll), m_iRunPos(0), m_timeWait(0){}

	bool UnRegister(ServiceObj* pObj){
        for (ItVec it = m_aObj.begin(); it != m_aObj.end(); ++it){
			if (*it == pObj){
                m_setDel.insert(pObj);
                return true;
			}
		}
		return false;
	}
	bool Register(ServiceObj* pObj, uint /*exeTime*/){
		m_bRun ? m_vecAdd.push_back(pObj) : m_aObj.push_back(pObj);
		return true;
	}
	void RunSevice(uint time_elasped, uint /*timenow*/)
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
		m_timeWait += time_elasped;
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
#endif // _DEBUG

//����������ʱ���ǰ�����ʱ��С����ô��ǰ����û�б�pop_frontǰ���������޷�ִ�е�����������
//����ʹ�ò�����ִ������Σ���Ϊִ�е�ͬһ��������ע���Objһ������exeTime��С
class cServiceList: public iService{
public:
	cServiceList(RefreshFun func) : iService(func){}
	listTimer m_list;
	std::set<ServiceObj*> m_setDel;

	bool Register(ServiceObj* pObj, uint exeTime){
		m_list.push_back(TimerPair(exeTime,pObj)); //list�ṹ���ŵ�����棬��Runʱ����RegҲû��ϵ
		return true;
	}
	bool UnRegister(ServiceObj* pObj){
		for (itListTimer it = m_list.begin(); it != m_list.end(); ++it){
			if (it->second == pObj) {
                m_setDel.insert(pObj);
				return true;
			}
		}		
		return false;
	}
	void RunSevice(uint /*time_elasped*/, uint timenow){
        ItSet itSet;
        for (itListTimer it = m_list.begin(); it != m_list.end() && !m_setDel.empty(); ){
            itSet = m_setDel.find(it->second);
            if (itSet != m_setDel.end()){
                m_setDel.erase(itSet);
                it = m_list.erase(it);
            }
            else ++it;
		}
        m_setDel.clear();

		m_bRun = true;
		while(!m_list.empty()) {
			TimerPair& pair = m_list.front();
			if (pair.first <= timenow)
			{
                if (m_setDel.find(pair.second) != m_setDel.end()){
                    m_list.pop_front();
                    continue;
                }
				uint nextTime = m_func(pair.second);
				if (nextTime > 0) Register(pair.second, timenow + nextTime);
				m_list.pop_front();
			}
			else
				break;
		};
        m_bRun = false;
	}
};
class cServiceMap : public iService{
public:
    cServiceMap(RefreshFun func) : iService(func){}
	mapTimer m_map;
    mapTimer m_mapAdd;
    std::set<ServiceObj*> m_setDel;

	bool Register(ServiceObj* pObj, uint exeTime){
        //if (m_nLock) printf("cServiceMap:Register:LockError ,%d\n", m_eService);
        m_map.insert(TimerPair(exeTime, pObj));
		return true;
	}
	bool UnRegister(ServiceObj* pObj){
		for (itMapTimer it = m_map.begin(); it != m_map.end(); ++it){
			if (it->second == pObj) {
                m_setDel.insert(pObj);
				return true;
			}
		}
		return false;
	}
	void RunSevice(uint /*time_elasped*/, uint timenow){
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