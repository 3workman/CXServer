/***********************************************************************
* @ ��Դ����
* @ brief
    1��ͳһ���������ڹ���ɢ�ҵĿ�Ǯ�۽��...̫������

    2����������Դʱ��ע����;ʧ�ܵ����

    3�������������ݣ���ȫ������Щ������ע����ֵ...���Ϊ�գ�����0��Ȼ��Cut�ɹ����Ͳ���

    4��ö�ٵ����ã�Ӧ������Ŀ�������ͨ������ɽ���ԴҲ����Item���У�ö����itemIdһ�£�g_handler���map�ṹ

    5��PackItem()�����Ľӿ����ɾ����̫���׳����Ҳ����������ģ�Ĭ�ϵ���Ʒ�ӿڡ�������ҪPack**�����;ͱ�Ž�����ҵ����Լ���

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
    static inline int PackItem(int itemId, char cnt){ return (itemId << 8) + cnt; } // ��8λ:��������24λ:��ƷID

    //������Դ�������
public:
#undef Declare
#define Declare(typ) bool _Set_##typ(Player& player, int diff);

    Reward_Enum
};
#define sResource Reward::Instance()
