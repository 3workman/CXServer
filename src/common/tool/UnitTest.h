#pragma once

namespace unittest
{
	/// <summary><![CDATA[
	/// A static class containing all unit test operations. In order to run test cases, you should do the following:
	/// 1) Write test cases in cpp files like this
	/// TEST_CASE(ServLink)
	/// {
	///		<Use TEST_ASSERT(condition) to test>
	///		<Use TEST_ERROR(expression) if you know "expression" will cause a fatal error by using the CHECK_ERROR macro.>
	///		<Use TEST_EXCEPTION(expression, exceptionType, assertFunction) if you know "expression" will throw an expression of "exceptionType", and then you can provide "assertFunction" to check the information provided in the exception.>
	/// }
	/// You should call [unittest::UnitTest::RunAllTests] in your main function to run all test cases.
	/// ]]></summary>
	class UnitTest{
	public:
		typedef void(*TestFunc)();

		UnitTest(const char* file, int line, TestFunc testFunc);

		static void RunAllTests(); // main()中调用
	};
}
#define TEST_CASE(Name)     /* cpp里写测试用例 */		\
		static void Func_##Name();						\
		static unittest::UnitTest						\
		Obj_##Name(__FILE__, __LINE__, Func_##Name);	\
		void Func_##Name()

#define TEST_CHECK_ERROR(CONDITION,DESCRIPTION) \
		do{                                     \
			if(!(CONDITION)){                   \
				throw Error(DESCRIPTION);       \
			}                                   \
		}while(0)

#define TEST_ASSERT(CONDITION) do{TEST_CHECK_ERROR(CONDITION,L"");}while(0)
#define TEST_EXCEPTION(STATEMENT,EXCEPTION,ASSERT_FUNCTION) try{STATEMENT; TEST_ASSERT(false);}catch(const EXCEPTION& e){ASSERT_FUNCTION(e);}