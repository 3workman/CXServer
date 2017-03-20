/***********************************************************************
* @ ͨ�������ж�ģ��
* @ brief
    1���ѻ���߻�
    2��һЩ����������޹ص�������ָ���������NULL

    3��arrayʽ��֯�ṹ���Ա�reward��mapʽ��֯�ṹ
* @ author zhoumf
* @ date 2016-11-21
************************************************************************/
#pragma once

#undef Declare
#undef Trigger_Enum
#define Declare(typ) typ,
#define Trigger_Enum\
    Declare(UpLevel)        /*�ȼ�����*/\
    Declare(DuringTime)     /*����ʱ���֮��*/\


class Player;
struct TriggerTable
{
    uint16      id;
    uint16      type;
    int32       val1; //1223145632��12��23��14:56:32
    int32       val2;
};
class Trigger {
    typedef std::map<int, const TriggerTable> TriggerMap;
    TriggerMap m_TriggerLst;

    Trigger();
public:
    typedef bool(Trigger::*TriggerFunc)(Player*, int32, int32);
    enum Type : uint {
        Trigger_Enum

        MAX_ENUM
    };
    static Trigger& Instance(){ static Trigger T; return T; }

    bool Check(Player* player, const int triggerId);
    bool Check(Player* player, const std::vector<int>& triggerIds);

    //�����жϺ���������Ͳ�����ָ���ˣ��߼��㸺��
public:
#undef Declare
#define Declare(typ) bool _Is_##typ(Player* player, int32 val1, int32 val2);
    Trigger_Enum
};
#define sTrigger Trigger::Instance()
