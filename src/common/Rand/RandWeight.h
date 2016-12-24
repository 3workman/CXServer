
template <typename T> typename std::vector<T*>::iterator RandWeight(std::vector<T*>& vec)
{
    int sumWeight = 0;
    for (std::vector<T*>::iterator it = vec.begin(); it != vec.end(); ++it)
    {
        sumWeight += (*it)->weight;
    }
    int rendNum = sRand.Rand(0, sumWeight);
    for (std::vector<T*>::iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (rendNum < (*it)->weight)
            return it;
        else
            rendNum -= (*it)->weight;
    }
    assert(0);
    return vec.end();
}