#include "stdafx.h"
#include "DataSend.h"
#include "bytebuffer.h"

void DataSend::Set(uint index, int value){
    if (index < DATA_SEND_SIZE)
    {
        if (value != m_data[index] || value == 0) //清0一定记变动
        {
            m_bits |= (1 << index);
            m_data[index] = value;
        }
    }
}
void DataSend::ReadDif(ByteBuffer& bf, bool bReadAll){
    bf.clear();
    for (uint8 i = 0; i < DATA_SEND_SIZE; ++i)
    {
        if (bReadAll || (m_bits & (1 << i)))
        {
            bf << i << m_data[i];
        }
    }
}
void DataSend::WriteDif(ByteBuffer& bf){
    uint8 index; int32 value;
    const uint8 size = bf.size() / (sizeof(uint8) + sizeof(int32));
    for (uint8 i = 0; i < size; ++i)
    {
        bf >> index >> value;
        OnDataChange(index, m_data[index], value);
        m_data[index] = value;
    }
}

bool ArrayData::Change(uint n, int v) {
    if (n < DATA_SIZE)
    {
        if (m_data[n] == v) return false;
        if (m_data[n] = v)
            m_bits |= (1 << n);   // 将n为置1
        else
            m_bits &= ~(1 << n);  // 将n位置0
        return true;
    }
    return false;
}
bool ArrayData::Clear() {
    if (m_bits) {
        m_bits = 0;
        memset(m_data, 0, sizeof(m_data));
        return true;
    }
    return false;
}
int Msg_ArrayData::GetLength() const {
    int cnt = 0;
    for (int i = 0; i < ArrayData::DATA_SIZE; ++i)
    {
        if (bits & (1 << i)) ++cnt;
    }
    return offsetof(Msg_ArrayData, data) + sizeof(data[0])*cnt;
}
void Msg_ArrayData::FillMsg(DataTyp typ, const ArrayData& refArr) {
    for (int cnt = 0, i = 0; i < ArrayData::DATA_SIZE; ++i){
        if (refArr.m_data[i]){
            this->data[cnt++] = refArr.m_data[i];
        }
    }
    this->bits = refArr.m_bits;
    this->isReset = true;
    this->typ = typ;
}
void Msg_ArrayData::FillMsg(DataTyp typ, const ArrayData& refArr, uint n) {
    if (n < ArrayData::DATA_SIZE){
        this->data[0] = refArr.m_data[n];
        this->bits = (1 << n);
    }else{
        this->bits = 0;
    }
    this->isReset = false;
    this->typ = typ;
}
void Msg_ArrayData::FillArray(ArrayData& refArr) const {
    if (this->isReset){
        refArr.m_bits = this->bits;
        memset(refArr.m_data, 0, sizeof(refArr.m_data));
    }else{
        refArr.m_bits |= this->bits;
    }
    for (int cnt = 0, i = 0; i < ArrayData::DATA_SIZE; ++i){
        if (this->bits & (1 << i)){
            refArr.m_data[i] = this->data[cnt++];
        }
    }
}