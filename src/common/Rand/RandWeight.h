/***********************************************************************
* @ Ȩ�����
* @ brief
    1���ṹ��������weight�ֶ�
    2��std::random_shuffle(2)�Ǻö���

* @ author zhoumf
* @ date 2016-12-26
************************************************************************/
template <typename T> typename std::vector<T*>::const_iterator RandWeight(const std::vector<T*>& vec)
{
    int sumWeight = 0;
    for (std::vector<T*>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        sumWeight += (*it)->weight;
    }
    int rendNum = Rand::rand(0, sumWeight);
    for (std::vector<T*>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (rendNum < (*it)->weight)
            return it;
        else
            rendNum -= (*it)->weight;
    }
    assert(0);
    return vec.end();
}
int RandWeight(const IntPairVec& vec) //Ȩ����ֵ��<val, weight>
{
    int sumWeight = 0;
    for (IntPairVec::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        sumWeight += it->second;
    }
    int rendNum = Rand::rand(0, sumWeight);
    for (IntPairVec::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (rendNum < it->second)
            return it->first;
        else
            rendNum -= it->second;
    }
    assert(0);
    return -1;
}