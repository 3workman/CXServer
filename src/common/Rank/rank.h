/***********************************************************************
* @ 实时排行榜
* @ brief
    1、从大到小排序，1起始
    2、数组缓存Top N，变动时上下移动
    3、排序对象须含有 uint rank，bool Less(const T&)
    4、支持多层级的数据比对，如：先积分、再等级、再vip...

* @ FIXME
    1、排行榜入库

* @ author zhoumf
* @ date 2016-12-26
************************************************************************/
#pragma once

template <class T>
class Rank {
    uint            _last;
    std::vector<T*> _arr;

public:
    Rank(int amount) : _last(0)
    {
        _arr.resize(amount+1, NULL);
    }
    bool OnValueChange(T& obj)
    {
        const uint oldRank = obj.rank;
        uint newIdx = SearchIndex(obj);
        if (obj.rank > 0) { //已上榜
            MoveToIndex(newIdx, obj.rank);
        } else if (newIdx > 0) { //之前未上榜
            InsertToIndex(newIdx, obj);
        }
        return oldRank == obj.rank;
    }
    void Clear()
    {
        memset(&_arr[0], 0, _arr.size() * sizeof(T*));
    }
    T*  GetRanker(uint rank)
    {
        assert(rank <= _last);
        return _arr[rank];
    }
    uint GetLastRank() { return _last; }

private:
    uint SearchIndex(T& obj)
    {
        //FIXME：二分查找新的排位
        for (int i = 1; i < _arr.size(); ++i)
        {
            if (_arr[i] == NULL || _arr[i]->Less(obj)) return i;
        }
        return 0;
    }
    void MoveToIndex(uint dst, uint src)
    {
        T* tmp = _arr[src]; assert(dst <= _last && src <= _last);
        if (src > dst) {
            memmove(&_arr[dst + 1], &_arr[dst], (src - dst) * sizeof(T*)); //dst后移一步
            _arr[dst] = tmp;

            for (uint i = dst; i <= src; ++i)
            {
                if (_arr[i]) _arr[i]->rank = i;
            }
        } else if (src < dst) {
            memmove(&_arr[src], &_arr[src + 1], (dst - src) * sizeof(T*)); //src+1前移一步
            _arr[dst] = tmp;

            for (uint i = src; i <= dst; ++i)
            {
                if (_arr[i]) _arr[i]->rank = i;
            }
        }
    }
    void InsertToIndex(uint idx, T& obj)
    {
        if (idx > _last) idx = ++_last;

        if (_arr[_last]) _arr[_last]->rank = 0; //尾名被挤出

        memmove(&_arr[idx + 1], &_arr[idx], (_last - idx) * sizeof(T*));
        _arr[idx] = &obj;

        for (uint i = idx; i <= _last; ++i)
        {
            if (_arr[i]) _arr[i]->rank = i;
        }
    }
};
