/***********************************************************************
* @ ��Դ����
* @ brief
    1��ͳһ���������ڹ���ɢ�ҵĿ�Ǯ�۽��...̫������

    2����������Դʱ��ע����;ʧ�ܵ����

    3�������������ݣ���ȫ������Щ������ע����ֵ...���Ϊ�գ�����0��Ȼ��Cut�ɹ����Ͳ���

* @ return ((!_isCheck) || retCheck) && ((!_isWrite) || retWrite);
    1��Ҫ��飬��������ret����
    2������飬��������true
    3��д�������һ�µ�

* @ author zhoumf
* @ date 2016-11-22
************************************************************************/
#pragma once

//Notice����ͬ���߱�һ��(item_proto)
#undef Declare
#undef Reward_Enum
#define Declare(typ) typ,
#define Reward_Enum\
    Declare(Gold)       \
    Declare(Diamond)    \
    Declare(Exp)        \
    Declare(HeroExp)    /*Ӣ�۾���*/\
    Declare(Item)       /*��Ʒ*/\


class Player;
class Reward {
    bool _isCheck;
    bool _isWrite;
public:
    typedef bool(Reward::*ResourceFunc)(Player&, int);
    enum Type : uint {
        Reward_Enum

        MAX_ENUM
    };
    static Reward& Instance(){ static Reward T; return T; }

    bool Change(Player& player, Type typ, int diff);
    bool Change(Player& player, const IntPairVec& resource);
    bool _Change(Player& player, Type typ, int diff);

	//��������
    //Notice�����ȼ����Ⱑ����Ҫ�뵱Ȼ������_������
    static inline int PackHeroExp(uint8 heroIdx, int exp){ return (exp << 8) + heroIdx; } // ��8λ:�ڼ���Hero����24λ:����ֵ
    static inline int PackItem(uint16 itemId, uint16 cnt){ return (cnt << 16) + itemId; } // ��16λ:��ƷID����16λ:����

    //������Դ�������
public:
#undef Declare
#define Declare(typ) bool _Set_##typ(Player& player, int diff);

    Reward_Enum
};
#define sResource Reward::Instance()
