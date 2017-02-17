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
    int             _last;
    std::vector<T*> _arr;

public:
    Rank(int amount) : _amount(amount), _last(0)
    {
        _arr.resize(_amount + 1, NULL);
    }
    bool OnValueChange(T& obj)
    {
        const int oldRank = obj.rank;

        int newIdx = SearchInsertIdx(obj.GetRankVal());
        if (obj.rank > 0) //已上榜
        {
            MoveToIndex(newIdx, obj.rank);
        }
        else if (newIdx > 0) //之前未上榜
        {
            InsertToIndex(newIdx, obj);
        }
        return oldRank == obj.rank;
    }
    void Clear()
    {
        memset(&_arr[0], 0, _arr.size() * sizeof(T*));
    }
    T*  GetRanker(int rank)
    {
        assert(rank > 0 && rank <= _amount);

        return _arr[rank];
    }
    int GetLastRank() { return _last; }

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
        T* tmp = _arr[src]; assert(dst > 0 && src > 0);
        if (src > dst) {
            memmove(&_arr[dst + 1], &_arr[dst], (src - dst) * sizeof(T*)); //dst后移一步
            _arr[dst] = tmp;

            for (int i = dst; i <= src; ++i)
            {
                if (_arr[i]) _arr[i]->rank = i;
            }
        } else if (src < dst) {
            memmove(&_arr[src], &_arr[src + 1], (dst - src) * sizeof(T*)); //src+1前移一步
            _arr[dst] = tmp;

            for (int i = src; i <= dst; ++i)
            {
                if (_arr[i]) _arr[i]->rank = i;
            }
        }
    }
    void InsertToIndex(int idx, T& obj)
    {
        assert(idx > 0);

        if (idx > _last) _last = idx;

        if (_arr[_amount]) _arr[_amount]->rank = 0; //尾名被挤出

        memmove(&_arr[idx + 1], &_arr[idx], (_amount - idx) * sizeof(T*));
        _arr[idx] = &obj;

        for (int i = idx; i <= _amount; ++i)
        {
            if (_arr[i]) _arr[i]->rank = i;
        }
    }
};
