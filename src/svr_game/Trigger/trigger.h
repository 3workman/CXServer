/***********************************************************************
* @ ͨ�������ж�ģ��
* @ brief
    1���ѻ���߻�
    2��һЩ����������޹ص�������ָ���������NULL

* @ author zhoumf
* @ date 2016-11-21
************************************************************************/
#pragma once

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
    enum {
        UpLevel,
        DuringTheTime,

        MAX_ENUM
    };
    static Trigger& Instance(){ static Trigger T; return T; }

    bool Check(Player* player, const int triggerId);
    bool Check(Player* player, const std::vector<int>& triggerIds);

    //�����жϺ���������Ͳ�����ָ���ˣ��߼��㸺��
    bool IsUpLevel(Player* player, int32 val1, int32 val2);
    bool IsDuringTime(Player* player, int32 val1, int32 val2);
};
#define sTrigger Trigger::Instance()
