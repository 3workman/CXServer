#include "stdafx.h"
#include "DebugFile.h"
#include <sstream>
#include "mkdir.h"

void cDebugFile::Append(const char *src, size_t cnt)
{
    if (!cnt) return;

    if (m_size < m_wpos + cnt) Double();

    assert(m_size < 102400);

    memcpy(&m_Data[m_wpos], src, cnt);

    stKeyInfo& info = m_vecKey[m_keyPos];
    info.size = cnt;
    info.pObj = src;
    info.pOld = &m_Data[m_wpos];

    ++m_keyPos;  m_wpos += cnt;
}

void cDebugFile::Output(string sKeyList /* =  */, eOutPut eOutType /* = All */)
{
    std::vector<string> vecKey;
	ParseName(vecKey, sKeyList);

	ostringstream osFile;
    int64 nResult(0); // >0增加  <0减少  =0不变
    for (size_t i = 0; i < m_keyPos; ++i)
    {
        stKeyInfo& info = m_vecKey[i];

        if (!vecKey.empty()
            && find(vecKey.begin(), vecKey.end(), info.key) == vecKey.end())
        {
            continue;
        }

        switch (info.type){
		case v_uint8:{
				nResult = int16(*(uint8*)info.pObj) - int16(*(uint8*)info.pOld); //可为-255
				if (OnResult(eOutType, nResult, info.key, osFile))
				{
					osFile << " = " << *(uint8*)info.pObj << "(" << *(uint8*)info.pOld << ")" << ";\r\n";
				}
			}break;
		case v_uint16:{
				nResult = int32(*(uint16*)info.pObj) - int32(*(uint16*)info.pOld);
				if (OnResult(eOutType, nResult, info.key, osFile))
				{
					osFile << " = " << *(uint16*)info.pObj << "(" << *(uint16*)info.pOld << ")" << ";\r\n";
				}
			}break;
		case v_uint32:{
				nResult = int64(*(uint32*)info.pObj) - int64(*(uint32*)info.pOld);
				if (OnResult(eOutType, nResult, info.key, osFile))
				{
					osFile << " = " << *(uint32*)info.pObj << "(" << *(uint32*)info.pOld << ")" << ";\r\n";
				}
			}break;
		case v_uint64:{
				nResult = (*(int64*)info.pObj) - (*(int64*)info.pOld);
				if (OnResult(eOutType, nResult, info.key, osFile))
				{
					osFile << " = " << *(int64*)info.pObj << "(" << *(int64*)info.pOld << ")" << ";\r\n";
				}
			}break;
		case v_int8:{
				nResult = int16(*(int8*)info.pObj) - int16(*(int8*)info.pOld);
				if (OnResult(eOutType, nResult, info.key, osFile))
				{
					osFile << " = " << *(int8*)info.pObj << "(" << *(int8*)info.pOld << ")" << ";\r\n";
				}
			}
			break;
		case v_int16:{
				nResult = int32(*(int16*)info.pObj) - int32(*(int16*)info.pOld);
				if (OnResult(eOutType, nResult, info.key, osFile))
				{
					osFile << " = " << *(int16*)info.pObj << "(" << *(int16*)info.pOld << ")" << ";\r\n";
				}
			}break;
		case v_int32:{
				nResult = int64(*(int32*)info.pObj) - int64(*(int32*)info.pOld);
				if (OnResult(eOutType, nResult, info.key, osFile))
				{
					osFile << " = " << *(int32*)info.pObj << "(" << *(int32*)info.pOld << ")" << ";\r\n";
				}
			}break;
		case v_int64:{
				nResult = (*(int64*)info.pObj) - (*(int64*)info.pOld);
				if (OnResult(eOutType, nResult, info.key, osFile))
				{
					osFile << " = " << *(int64*)info.pObj << "(" << *(int64*)info.pOld << ")" << ";\r\n";
				}
			}break;
		case v_float:{
				float dif = (*(float*)info.pObj) - (*(float*)info.pOld);
				if (OnResult(eOutType, dif == 0 ? 0 : (dif>0 ? 1 : -1), info.key, osFile))
				{
					osFile << " = " << *(float*)info.pObj << "(" << *(float*)info.pOld << ")" << ";\r\n";
				}
			}break;
		case v_double:{
				double dif = (*(double*)info.pObj) - (*(double*)info.pOld);
				if (OnResult(eOutType, dif == 0 ? 0 : (dif>0 ? 1 : -1), info.key, osFile))
				{
					osFile << " = " << *(double*)info.pObj << "(" << *(double*)info.pOld << ")" << ";\r\n";
				}
			}break;
		case v_string:{
				nResult = strcmp(info.pObj, info.pOld);
				if (OnResult(eOutType, nResult, info.key, osFile))
				{
					osFile << " = " << info.pObj << "(" << info.pOld << ")" << ";\r\n";
				}
			}break;
		default:{
				nResult = int64(*(int32*)info.pObj) - int64(*(int32*)info.pOld);
				if (OnResult(eOutType, nResult, info.key, osFile))
				{
					osFile << " = " << *(int32*)info.pObj << "(" << *(int32*)info.pOld << ")" << ";\r\n";
				}
			}break;
        }
    }
	osFile << "\r\n";

    WriteToFile(m_sFileName, osFile);
}
void cDebugFile::ParseName(std::vector<string>& refVec, string str)
{
	string strkey;
	std::vector<string> vecKey;
	int posList(0), indexList = str.find(",");
	while (-1 != indexList)
	{
		strkey = str.substr(posList, indexList);
		posList = indexList + 1;
		indexList = str.find(",", posList);
		if (!strkey.empty())
		{
			vecKey.push_back(strkey);
		}
	}
	strkey = str.substr(posList);
	if (!strkey.empty())
	{
		vecKey.push_back(strkey);
	}
}
void cDebugFile::WriteToFile(string sFileName, ostringstream& osFile)
{
    char sfile[64];
    sprintf(sfile, "DebugFile\\%s.txt", sFileName.c_str());
    Dir::CreatDir(sfile);
    FILE* fp = fopen(sfile, "a");
	if (fp == NULL) return;
	fprintf(fp, "%s", osFile.str().c_str()); // 傻逼字符串处理/(ㄒoㄒ)/~~
	fclose(fp);
	fp = NULL;
}

bool cDebugFile::OnResult(eOutPut eType, const int64& result, const string& key, ostringstream& file)
{
	switch (eType){
	case cDebugFile::All:
		{
			if (result > 0)
				file << "--->" << key;
			else if (result < 0)
				file << "<---" << key;
			else
				file << key;
		}break;
	case cDebugFile::Change:
		{
			if (result > 0)
				file << "--->" << key;
			else if (result < 0)
				file << "<---" << key;
			else
				return false;
		}break;
	case cDebugFile::Unchange:
		{
			if (result == 0)
				file << key;
			else
				return false;
		}break;
	case cDebugFile::Decrease:
		{
			if (result < 0)
				file << "<---" << key;
			else
				return false;
		}break;
	case cDebugFile::Increase:
		{
			if (result > 0)
				file << "--->" << key;
			else
				return false;
		}break;
	}
	return true;
}