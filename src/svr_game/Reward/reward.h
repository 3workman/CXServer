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

class Player;
class Reward {
    bool _isCheck;
    bool _isWrite;
public:
    typedef bool(Reward::*ResourceFunc)(Player&, int);
    enum Type : uint { //Notice���뱣��ͬcpp�к���ע��˳��һ��
        Gold,
        Diamond,
        Exp,
        HeroExp,    // ��8λ:�ڼ���Hero����24λ:����ֵ
        Item,       // ��16λ:��ƷID����16λ:����

        MAX_ENUM
    };
    static Reward& Instance(){ static Reward T; return T; }

    bool Change(Player& player, Type typ, int diff);
    bool Change(Player& player, const IntPairVec& resource);
    bool _Change(Player& player, Type typ, int diff);

	//��������
    static inline int PackHeroExp(uint8 heroIdx, int exp){ return (exp << 8) + heroIdx; } //Notice�����ȼ����Ⱑ����Ҫ�뵱Ȼ������_������
    static inline int PackItem(uint16 itemId, uint16 cnt){ return (cnt << 16) + itemId; }

    //������Դ�������
    bool _SetGold(Player& player, int diff);
    bool _SetDiamond(Player& player, int diff);
    bool _SetExp(Player& player, int diff);
    bool _SetHeroExp(Player& player, int diff);
    bool _SetItem(Player& player, int diff);
};
#define sResource Resource::Instance()
