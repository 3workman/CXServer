#pragma once

class Rand {
    Rand();
    static Rand _assist_obj; //ûʲô�õľ�̬����ֻ��Ϊ�˴���ctor���srand()
    //static Rand& Instance(){ static Rand T; return T; }
    /*
        1����̬�����ɱ����������ʼ������main()֮ǰ
        2������ģʽ���״ε���ʱ��ʼ����������һ��Instance()�������÷��ع���
        3���о�����̬���󡱵ķ�ʽ����ЧЩ����������ִ��ӿ���
    */
public:
    static int rand();
    static int rand(int left, int right);
    static float randf();
    static float randf(float left, float right);
};