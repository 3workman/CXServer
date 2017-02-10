/***********************************************************************
* @ 实时排行榜
* @ brief
    1、从大到小排序，1起始
    2、数组缓存Top N，变动时上下移动
    3、排序对象须含有int rank，int GetRankVal()

* @ author zhoumf
* @ date 2016-12-26
************************************************************************/
#pragma once

template <class T>
class Rank {
    const int       _amount; //排多少人
    std::vector<T*> _arr;

public:
    Rank(int amount) : _amount(amount)
    {
        _arr.resize(_amount + 1, NULL);
    }
    void OnValueChange(T& obj)
    {
        int newIdx = SearchInsertIdx(obj.GetRankVal());
        if (obj.rank > 0) //已上榜
        {
            MoveToIndex(newIdx, obj.rank);
        }
        else if (_arr[_amount] == NULL || obj.GetRankVal() > _arr[_amount]->GetRankVal()) //未上榜但超过最后一名
        {
            InsertToIndex(newIdx, obj);
        }
    }
    void Clear()
    {
        memset(&_arr[0], 0, _arr.size() * sizeof(T*));
    }

private:
    int SearchInsertIdx(int dstVal)
    {
        //FIXME：二分查找新的排位
        for (int i = 1; i <= _amount; ++i)
        {
            if (_arr[i] == NULL || dstVal > _arr[i]->GetRankVal())
            {
                return i;
            }
        }
        return 0;
    }
    void MoveToIndex(int dst, int src)
    {
        T* tmp = _arr[src];
        if (src > dst)
        {
            //dst后移一步
            memmove(&_arr[dst + 1], &_arr[dst], (src - dst) * sizeof(T*));
            _arr[dst] = tmp;

            for (int i = dst; i <= src; ++i)
            {
                if (_arr[i]) _arr[i]->rank = i;
            }
        }
        else if (src < dst)
        {
            //src+1前移一步
            memmove(&_arr[src], &_arr[src + 1], (dst - src) * sizeof(T*));
            _arr[dst] = tmp;

            for (int i = src; i <= dst; ++i)
            {
                if (_arr[i]) _arr[i]->rank = i;
            }
        }
    }
    void InsertToIndex(int idx, T& obj)
    {
        if (_arr[_amount]) _arr[_amount]->rank = 0; //尾名被挤出

        memmove(&_arr[idx + 1], &_arr[idx], (_amount - idx) * sizeof(T*));
        _arr[idx] = &obj;

        for (int i = idx; i <= _amount; ++i)
        {
            if (_arr[i]) _arr[i]->rank = i;
        }
    }
};
