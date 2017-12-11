#pragma once

class Rand { // [left, right)
    Rand();
    static Rand _assist_obj; //没什么用的静态对象，只是为了触发ctor里的srand()
    //static Rand& Instance(){ static Rand T; return T; }
    /*
        1、静态对象，由编译器负责初始化，在main()之前
        2、单例模式，首次调用时初始化，还多了一次Instance()函数调用返回过程
        3、感觉“静态对象”的方式更高效些，尤其对这种纯接口类
    */
public:
    static int rand();
    static int rand(int left, int right);
    static float randf();
    static float randf(float left, float right);
};