/**************************************************************************
* @ 本地文件存储的标记数据
* @ brief
	1、多用于客户端的相关标记，如新手指引、界面
	2、其它类可声明一个UserConfig::m_UserCfgData[i]的引用，便于使用
* @ author zhoumf
* @ date 2014-12-24
***************************************************************************/
#pragma once

enum UserConfigType // 只能后入
{
    UserCfg_CGuiXingHun_SelectColor         = 0,
    UserCfg_CGuiXH_SoulHunt_SelectColor     = 1,
    UserCfg_CGuiXH_SoulHunt_Merge           = 2,
    UserCfg_DoneFactionTask                 = 3,
    UserCfg_FirstUIJingLian                 = 4, //首次打开精炼分页

    UserCfg_MAX // 只能后入
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