#include "stdafx.h"
#include "rand.h"
#include <stdlib.h>
#include <time.h>

Random::Random()
{
    srand((unsigned)time(NULL));
}
int Random::Rand()
{
    return rand();
}
int Random::Rand(int left, int right)
{
    assert(right > left);
    int	range = right - left;
    return left + (Rand() % range);
}
float Random::Randf()
{
    return float(rand() / (double)RAND_MAX);
}
float Random::Randf(float left, float right)
{
    assert(right > left);
    float diff = right - left;
    return left + Randf() * diff;
}
