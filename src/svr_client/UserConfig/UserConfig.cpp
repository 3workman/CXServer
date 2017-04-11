#include "stdafx.h"
#include "UserConfig.h"
#include <atlstr.h>
#include "tool/mkdir.h"

#define DEFAULT_CONFIG { 1, 1, 0,\
                         1, 0,	/*DoneFactionTask...*/\
                       }
const int DEFAULT[] = DEFAULT_CONFIG;
const char* UserConfigName[] = {
    "CGuiXingHun::GetSelectColor",
    "CGuiXH_SoulHunt::GetSelectColor",
    "CGuiXH_SoulHunt::m_bMerge",
    "DoneFactionTask",
    "FirstUIJingLian",
};
STATIC_ASSERT_ARRAY_LENGTH(DEFAULT, UserCfg_MAX);// ȱ Ĭ������
STATIC_ASSERT_ARRAY_LENGTH(UserConfigName, UserCfg_MAX);// ȱ ������


int UserConfig::m_UserCfgData[UserCfg_MAX] = DEFAULT_CONFIG; // ��static ��Ա��ʼ��

static FILE* OpenFile(const char* mode) {
    char sfile[64];
    sprintf(sfile, "users\\UserConfig.txt");
    //sprintf(sfile, "users\\%s\\%d\\UserConfig.txt", m_pMainChar->GetAccount(), m_pMainChar->GetCharTrueID());
    Dir::CreatDir(sfile);
    return fopen(sfile, mode);
}
void UserConfig::OnLogin()
{
    FILE* fp = OpenFile("r");
	if (fp == NULL) {
		int defaultCfg[] = DEFAULT_CONFIG;
		memcpy(m_UserCfgData, defaultCfg, sizeof(m_UserCfgData));
		return;
	}
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* pText = new char[size + 1];
    ON_SCOPE_EXIT( [&]{ delete[] pText; });
	memset(pText, 0, size + 1);
	fread(pText, size, 1, fp);
	fclose(fp);
	fp = NULL;

	CString strText(pText);
    strText.Remove(' ');

	int iStart = 0;
	CString strItem;
	for (int i = 0; i < UserCfg_MAX; ++i)
	{
		strItem = strText.Tokenize(";", iStart); //���� ��iStartλ�õ�pszTokens�ַ�֮ǰ ������
		if (iStart == -1) break;
		int nQual = strItem.Find("=");
		if (nQual == -1) continue;

		m_UserCfgData[i] = atoi(strItem.Mid(nQual + 1)); //���� �ӵ�n���ַ������һ���ַ�֮�� ������
	}
}
void UserConfig::OnLogout()
{
	//���UserConfig��Ϣ
    CString strText;
	for (int i = 0; i < UserCfg_MAX; ++i)
	{
        strText.AppendFormat("%s = %d;\r\n", UserConfigName[i], m_UserCfgData[i]);
	}

	//��дUserConfig.txt
    if (FILE* fp = OpenFile("w")) {
        fprintf(fp, "%s", strText.GetString());
        fclose(fp);
    }
}