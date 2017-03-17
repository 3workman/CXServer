/***********************************************************************
* @ 下发一系列数据中的变动部分
* @ brief
	1、
	2、
* @ author zhoumf
* @ date 2014-12-25
************************************************************************/
#pragma once

// 静态全局变量则限制了其作用域， 即只在定义该变量的源文件内有效， 在同一源程序的其它源文件中不能使用它
// 静态的全局函数表示只能被当前文件所使用，外部用extern关键字声明之后也是无法使用的
static const uint DATA_SEND_SIZE = 32;

class ByteBuffer;
struct DataSend {
	int m_bits; //记录某位的数据是否变动
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
	//void ClearData(){ memset(this, 0, sizeof(*this)); } //会把虚表干掉！！！调虚函数就宕机了

	bool isDirty() const{ return m_bits > 0; }
    void ReadDif(ByteBuffer& bf, bool bReadAll);
    void WriteDif(ByteBuffer& bf);

	// 不要memset(this, ...)会干掉虚表，调用虚函数宕机
	virtual void OnDataChange(uint nIndex, int oldValue, int newValue){}
};

class ArrayData {
    static const uint DATA_SIZE = 16;
    short m_bits; // 标记有效值
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