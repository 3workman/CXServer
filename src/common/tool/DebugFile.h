/***********************************************************************
* @ ���ڽ��ͱ������˳����������䶯���д���ļ�
* @ brief
	1��FIXME��Ŀǰ��֧��λ������İ󶨡�

	2������ debugFile << "nCount" << nCount << "nXingHun" << pChar->nXingHun;
		�� ����"key"����"value"
		�� ��̬�ַ�������"key"�����ײ㲻���ƶ�ָ��
		�� ��������"value"��������һ����ָ���ƶ�һ��

	3��Output()�У�sFileName�ɰ���·��"�ǻ�\\ExchangeTrain"��sKeyList����ָ�������Щ����

	4��������Ϣ��Ӧ�����а󶨹��ĵı���������ִ����Ϻ�鿴��䶯���

* @ author zhoumf
* @ date 2016-6-30
***********************************************************************/
#pragma once
#include <string>
#include <vector>
#include <assert.h>

using namespace std;

class cDebugFile {
    enum ValueEnum {
        v_uint8,
        v_uint16,
        v_uint32,
        v_uint64,
        v_int8,
        v_int16,
        v_int32,
        v_int64,
        v_float,
        v_double,
        v_string,
    };
    struct stKeyInfo {
        ValueEnum type;
        const char*  pObj;
        char*  pOld;
        string key;

        stKeyInfo() : type(v_int32), pObj(NULL), pOld(NULL) {}

        stKeyInfo(const char* str) : type(v_int32), pObj(NULL), pOld(NULL), key(str) {}
    };
    size_t  m_keyPos, m_wpos, m_size;  //����д����λ��,�ܴ�С
    char*   m_Data = NULL;
    string  m_sFileName;
    std::vector<stKeyInfo> m_vecKey;
public:
    enum eOutPut{
        All,        //��¼��������
        Change,     //��¼�䶯��
        Unchange,   //��¼û���
        Decrease,   //���� <----
        Increase,   //���� ---->
    };

    cDebugFile(string sFileName) : 
        m_keyPos(0), m_wpos(0), m_size(1024), m_sFileName(sFileName)
	{
		m_Data = new char[m_size];
	}
    ~cDebugFile(){ Output(); delete[] m_Data; }

	void Output(string sKeyList = "", eOutPut eType = All);

    //template<typename T> cDebugFile& operator<<(const T& value){
    //    Append<T>(value);
    //    return *this;
    //}

    #define Operator_CheckCode(v) \
        if (m_vecKey.size() <= m_keyPos) {assert(0); return *this;}\
        m_vecKey[m_keyPos].type = v;

    cDebugFile& operator<<(const uint8& value) {
        Operator_CheckCode(v_uint8)
        Append(value);
        return *this;
    }
    cDebugFile& operator<<(const uint16& value) {
        Operator_CheckCode(v_uint16)
        Append(value);
        return *this;
    }
    cDebugFile& operator<<(const uint32& value) {
        Operator_CheckCode(v_uint32)
        Append(value);
        return *this;
    }
    cDebugFile& operator<<(const uint64& value) {
        Operator_CheckCode(v_uint64)
        Append(value);
        return *this;
    }
    cDebugFile& operator<<(const int8& value) {
        Operator_CheckCode(v_int8)
        Append(value);
        return *this;
    }
    cDebugFile& operator<<(const int16& value) {
        Operator_CheckCode(v_int16)
        Append(value);
        return *this;
    }
    cDebugFile& operator<<(const int32& value) {
        Operator_CheckCode(v_int32)
        Append(value);
        return *this;
    }
    cDebugFile& operator<<(const int64& value) {
        Operator_CheckCode(v_int64)
        Append(value);
        return *this;
    }
    cDebugFile& operator<<(const bool& value) {
        Operator_CheckCode(v_int8)
        Append(value);
        return *this;
    }
    cDebugFile& operator<<(const float& value) {
        Operator_CheckCode(v_float)
        Append(value);
        return *this;
    }
    cDebugFile& operator<<(const double& value) {
        Operator_CheckCode(v_double)
        Append(value);
        return *this;
    }
    cDebugFile& operator<<(char *str) {
        Operator_CheckCode(v_string)
        Append(str, strlen(str)+1);
        return *this;
    }
	cDebugFile& operator<<(const char *str) { //����Key
		stKeyInfo info(str);
		m_vecKey.push_back(info);
        return *this;
    }
private:
	bool OnResult(eOutPut eType, const int64& result, const string& key, ostringstream& file);
	void ParseName(std::vector<string>& refVec, string str);
	void WriteToFile(string sFileName, ostringstream& osFile);

    template <typename T> void Append(const T& value)
	{
        Append((char*)&value, sizeof(value));
    }
    void Append(const char *src, size_t cnt);
    void Double(){
        char* temp = new char[m_size *= 2];
        memcpy(temp, m_Data, m_size / 2 * sizeof(char));
        delete[] m_Data;
        m_Data = temp;
    }
};

#ifdef _DEBUG
	#define DebugFile_Create(name) cDebugFile fileQA(name)

    #define DebugFile_Output_1() fileQA.Output(name)
    #define DebugFile_Output_2(KeyList) fileQA.Output(name, KeyList)
    #define DebugFile_Output_3(KeyList, fileType) fileQA.Output(name, KeyList, fileType)

    #define DebugFile_Input(k, v) fileQA << k << v
#else
	#define DebugFile_Create(name)

    #define DebugFile_Output_1()
    #define DebugFile_Output_2(KeyList)
    #define DebugFile_Output_3(KeyList, fileType)

    #define DebugFile_Input(k, v)
#endif

/************************************************************************/
// ʾ��
#ifdef _MY_Test
	void test_DebugFile(){
		int a = 0;
		float f = 0.1f;
		double d = 0.02;
		cDebugFile file("����\\test");
		file << "a" << a << "f" << f << "d" << d;
		//DebugFile_Create("����\\test");
		//DebugFile_Input("aa", a);
		a++; f = 0.11f; d = 0.787;
	}
#endif