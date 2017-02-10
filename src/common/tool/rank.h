/***********************************************************************
* @ ʵʱ���а�
* @ brief
    1���Ӵ�С����1��ʼ
    2�����黺��Top N���䶯ʱ�����ƶ�
    3����������뺬��int rank��int GetRankVal()

* @ author zhoumf
* @ date 2016-12-26
************************************************************************/
#pragma once

template <class T>
class Rank {
    const int       _amount; //�Ŷ�����
    std::vector<T*> _arr;

public:
    Rank(int amount) : _amount(amount)
    {
        _arr.resize(_amount + 1, NULL);
    }
    void OnValueChange(T& obj)
    {
        int newIdx = SearchInsertIdx(obj.GetRankVal());
        if (obj.rank > 0) //���ϰ�
        {
            MoveToIndex(newIdx, obj.rank);
        }
        else if (_arr[_amount] == NULL || obj.GetRankVal() > _arr[_amount]->GetRankVal()) //δ�ϰ񵫳������һ��
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
        //FIXME�����ֲ����µ���λ
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
            //dst����һ��
            memmove(&_arr[dst + 1], &_arr[dst], (src - dst) * sizeof(T*));
            _arr[dst] = tmp;

            for (int i = dst; i <= src; ++i)
            {
                if (_arr[i]) _arr[i]->rank = i;
            }
        }
        else if (src < dst)
        {
            //src+1ǰ��һ��
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
        if (_arr[_amount]) _arr[_amount]->rank = 0; //β��������

        memmove(&_arr[idx + 1], &_arr[idx], (_amount - idx) * sizeof(T*));
        _arr[idx] = &obj;

        for (int i = idx; i <= _amount; ++i)
        {
            if (_arr[i]) _arr[i]->rank = i;
        }
    }
};
