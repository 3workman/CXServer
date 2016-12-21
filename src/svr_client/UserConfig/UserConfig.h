/**************************************************************************
* @ �����ļ��洢�ı������
* @ brief
	1�������ڿͻ��˵���ر�ǣ�������ָ��������
	2�������������һ��UserConfig::m_UserCfgData[i]�����ã�����ʹ��
* @ author zhoumf
* @ date 2014-12-24
***************************************************************************/
#pragma once

enum UserConfigType // ֻ�ܺ���
{
    UserCfg_CGuiXingHun_SelectColor         = 0,
    UserCfg_CGuiXH_SoulHunt_SelectColor     = 1,
    UserCfg_CGuiXH_SoulHunt_Merge           = 2,
    UserCfg_DoneFactionTask                 = 3,
    UserCfg_FirstUIJingLian                 = 4, //�״δ򿪾�����ҳ

    UserCfg_MAX // ֻ�ܺ���
}; 

class UserConfig{
public:
    static void OnLogin();
    static void OnLogout();

    static int GetData(UserConfigType eType){
        if (eType >= 0 && eType < UserCfg_MAX)
        {
            return m_UserCfgData[eType];
        }
        return -1;
    }
    static void SetData(UserConfigType eType, int nData){
        if (eType >= 0 && eType < UserCfg_MAX)
        {
            m_UserCfgData[eType] = nData;
        }
    }
    static int m_UserCfgData[UserCfg_MAX];
};