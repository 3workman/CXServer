#pragma once

class Random {
    Random();
public:
    static Random& Instance(){ static Random T; return T; }

    int Rand();
    int Rand(int left, int right);
    float Randf();
    float Randf(float left, float right);
};
#define sRand Random::Instance()
