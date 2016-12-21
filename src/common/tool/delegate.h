/**************************************************************************
 * @ 事件委托
 * @ brief
	 1、委托可看作是对函数接口的抽象
	 2、一个委托对象可搭载多个方法，所有方法依次唤起
	 3、委托对象所搭载的方法不必来自同一个类，方法具备相同的参数表、返回值类型即可
	 4、【如何搭载类的非静态成员函数？】见MessageMap.h中的成员函数指针用法
 * @ author zhoumf
 * @ date 2016-3-6
 ************************************************************************/
#pragma once

class iDelegate{}; //只是为了满足c++类成员指针形式，唉~
template<typename TReturn, typename TParam>
class cDelegate{
public:
	typedef TReturn(iDelegate::*Ptr_Fun)(TParam); // 函数指针

	cDelegate() {}
	void Add(iDelegate* p, Ptr_Fun fun) { m_vecFun.push_back(stFun(p, fun)); }
	void Cut(iDelegate* p, Ptr_Fun fun)
	{
		auto it = m_vecFun.begin();
		for (; it != m_vecFun.end(); ++it){
			if (v.pObj == p && v.pFun == fun){
				m_vecFun.erase(it);
				break;
			}
		}
	}
	void operator()(TParam param)
	{
		for (stFun& v : m_vecFun) v(param); // 别忘了'&'
	}

private:
	struct stFun{
		iDelegate* pObj;   // 如果那个对象析构了，此指针不就野了！！！【who Add who Cut】
		Ptr_Fun pFun;
		stFun(iDelegate* p, Ptr_Fun fun) : pObj(p), pFun(fun) {}
		TReturn operator()(TParam param) { return (pObj->*pFun)(param); }
	};
	vector<stFun> m_vecFun;
};

/************************************************************************/
// 示例
#ifdef _MY_Test
	// 要搭载的方法
	class cDelegateObjA : public iDelegate{
	public:
		void Fun_1(int data)
		{
			cout << "事件委托（A）输出：" << data << endl;
		}
	};
	class cDelegateObjB : public iDelegate{
	public:
		void Fun_2(int data)
		{
			cout << "事件委托（B）输出：" << data << endl;
		}
	};
	void test_Delegate(){
		cout << "——————————————事件委托 Delegate——————————————" << endl;
		cDelegateObjA objA;
		cDelegateObjB objB;

		typedef cDelegate<void, int> Delegate;
		Delegate entrust;
		entrust.Add(&objA, static_cast<Delegate::Ptr_Fun>(&cDelegateObjA::Fun_1)); // 直接强转，参数为string类型，仍可编译通过，运行时宕机/(ㄒoㄒ)/~~
		entrust.Add(&objB, (Delegate::Ptr_Fun)&cDelegateObjB::Fun_2); // 有什么办法让它编译报错呢！？ static_cast
		entrust(10);
	}
#endif