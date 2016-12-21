/***********************************************************************
* @ �·�һϵ�������еı䶯����
* @ brief
	1��
	2��
* @ author zhoumf
* @ date 2014-12-25
************************************************************************/
#pragma once

// ��̬ȫ�ֱ������������������� ��ֻ�ڶ���ñ�����Դ�ļ�����Ч�� ��ͬһԴ���������Դ�ļ��в���ʹ����
// ��̬��ȫ�ֺ�����ʾֻ�ܱ���ǰ�ļ���ʹ�ã��ⲿ��extern�ؼ�������֮��Ҳ���޷�ʹ�õ�
static const uint DATA_SEND_SIZE = 32;

class ByteBuffer;
struct DataSend {
	int m_bits; //��¼ĳλ�������Ƿ�䶯
	int m_data[DATA_SEND_SIZE];
public:
	DataSend() {
        static_assert((sizeof(m_bits) * 8 >= DATA_SEND_SIZE), "DataSend m_bits too short !!!");
        Clear();
    }
    void Set(uint index, int value);
    int  Get(uint index){ return index < DATA_SEND_SIZE ? m_data[index] : -1; }
	void SetZero(){ m_bits = 0xffffffff; memset(m_data, 0, sizeof(m_data)); }
	void Clear(){ m_bits = 0; memset(m_data, 0, sizeof(m_data)); }
	//void ClearData(){ memset(this, 0, sizeof(*this)); } //������ɵ����������麯����崻���

	bool isDirty() const{ return m_bits > 0; }
    void ReadDif(ByteBuffer& bf, bool bReadAll);
    void WriteDif(ByteBuffer& bf);

	// ��Ҫmemset(this, ...)��ɵ���������麯��崻�
	virtual void OnDataChange(uint nIndex, int oldValue, int newValue){}
};

class ArrayData {
    static const uint DATA_SIZE = 16;
    short m_bits; // �����Чֵ
    int m_data[DATA_SIZE];
public:
    ArrayData() {
        static_assert((sizeof(m_bits) * 8 >= DATA_SIZE), "ArrayData m_bits too short !!!");
        m_bits = 0;
        memset(m_data, 0, sizeof(m_data));
    }
    int operator[](uint n) const{ return n < DATA_SIZE ? m_data[n] : 0; }
    bool Change(uint n, int v);
    bool Clear();
    friend struct Msg_ArrayData;
};
struct Msg_ArrayData {
    enum DataTyp {
        MapData,
        CampData,
    };
    char  typ;
    bool  isReset;
    short bits;
    int data[ArrayData::DATA_SIZE];

    Msg_ArrayData() : typ(0), bits(0), isReset(false){
        static_assert((sizeof(bits) * 8 >= ArrayData::DATA_SIZE), "Msg_ArrayData m_bits too short !!!");
        memset(data, 0, sizeof(data));
    }

    int GetLength() const;
    void FillMsg(DataTyp typ, const ArrayData& refArr);
    void FillMsg(DataTyp typ, const ArrayData& refArr, uint n);
    void FillArray(ArrayData& refArr) const;
};