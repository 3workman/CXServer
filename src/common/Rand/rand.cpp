#include "stdafx.h"
#include "rand.h"
#include <stdlib.h>
#include <time.h>

Rand Rand::_assist_obj;

Rand::Rand()
{
    srand((unsigned)time(NULL));
}
int Rand::rand()
{
    return ::rand();
}
int Rand::rand(int left, int right)
{
    assert(right >= left);
    if (left == right) return left;
    int	range = right - left;
    return left + (::rand() % range);
}
float Rand::randf()
{
    return float(::rand() / (double)RAND_MAX);
}
float Rand::randf(float left, float right)
{
    float diff = right - left;
    return left + randf() * diff;
}
