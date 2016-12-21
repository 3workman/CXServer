#pragma once
#include <vector>
#include <list>
#include <map>
#include <set>
#include <hash_map>
#include <assert.h>

/*
	ע�⣺��buffer�������Ҫ��λ������ӡ�
		1���ײ�ȡ��ַ��memcpyʱ������дԽ��
		2��λ���������'&'ȡ��ַ���������������ݿ�ͷ
		3����˵��λ���������ν��к���ƥ����أ� int64  a:2; b62;  ByteBuffer::read(a) ƥ����ĸ��� read<int8>��read<int64>��
*/

typedef signed __int64 int64;
typedef signed __int32 int32;
typedef signed __int16 int16;
typedef signed __int8  int8;
typedef unsigned __int64 uint64;
typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef unsigned __int8  uint8;

class ByteBuffer final {//tolua_export
    // read and write positions
    size_t _rpos, _wpos;
    std::vector<uint8> _storage;
public:
	//��̬���ݳ�Ա��ֻ��һ�ݿ������������κ�һ������
	//һ��ͨ�����������������ʣ���Ϊ������ȷ���ڵ����κ�һ������������֮ǰ����ʼ����̬���ݳ�Ա
	//���ӡ���һ�������뵥Ԫͨ�������������þ�̬��Աʱ����̬��Ա�п�����δ����ʼ��
	//C++�ԡ������ڲ�ͬ���뵥Ԫ�ڵ�non-local static���󡱵ĳ�ʼ����Դ�������ȷ����
	//C++��֤�������ڵ�local static������ڡ��ú����������ڼ䡱���״����ϸö���֮����ʽ��ʱ����ʼ��
	const static size_t DEFAULT_SIZE = 0x1000;

	//vector��reserve������vector��capacity����������sizeû�иı䣡��resize�ı���vector��capacityͬʱҲ����������size
	//reserve������Ԥ���ռ䣬���ڿռ��ڲ���������Ԫ�ض���������û������µĶ���֮ǰ���������������ڵ�Ԫ��
	//resize�Ǹı������Ĵ�С�����ڴ���������˵����������֮�󣬾Ϳ������������ڵĶ�����
	//resize����������������������һ�������������µĴ�С�� �ڶ���������Ҫ���������е���Ԫ�أ�������������ʡ�ԣ���ô�͵���Ԫ�ض����Ĭ�Ϲ��캯��
	//���߶��ᱣ��ԭ�����ڵ�����
	ByteBuffer() : _rpos(0), _wpos(0) {
		_storage.reserve(DEFAULT_SIZE);
	}
	ByteBuffer(size_t res) : _rpos(0), _wpos(0) {
		_storage.reserve(res);
	}
	ByteBuffer(const ByteBuffer &buf) : _rpos(buf._rpos), _wpos(buf._wpos), _storage(buf._storage) { }

	void clear() {
		_storage.clear();
		_rpos = _wpos = 0;
	}
    size_t size() const { return _storage.size(); }//tolua_export
    // one should never use resize probably
    void resize(size_t newsize) {//tolua_export
        _storage.resize(newsize);
        _wpos = _storage.size();
        _rpos = 0;
    }//tolua_export
    void reserve(size_t ressize) {//tolua_export
        if (ressize > size()) _storage.reserve(ressize);
    }//tolua_export

	//template <typename T> void insert(size_t pos, T value) {
	//  insert(pos, (uint8 *)&value, sizeof(value));
	//}
	template <typename T> void put(size_t pos, T value) {
		put(pos, (uint8 *)&value, sizeof(value));
	}
    void put(size_t pos, const uint8 *src, size_t cnt) {//tolua_export
        assert(pos + cnt <= size());
        memcpy(&_storage[pos], src, cnt);
    }//tolua_export
    //void insert(size_t pos, const uint8 *src, size_t cnt) {
    //  std::copy(src, src + cnt, inserter(_storage, _storage.begin() + pos));
    //}

	// stream like operators for storing data
	ByteBuffer &operator<<(bool value) {//tolua_export
		append<char>((char)value);
		return *this;
	}//tolua_export
	// unsigned
	ByteBuffer &operator<<(uint8 value) {//tolua_export
		append<uint8>(value);
		return *this;
	}//tolua_export
	ByteBuffer &operator<<(uint16 value) {//tolua_export
		append<uint16>(value);
		return *this;
	}//tolua_export
	ByteBuffer &operator<<(uint32 value) {//tolua_export
		append<uint32>(value);
		return *this;
	}//tolua_export
	ByteBuffer &operator<<(uint64 value) {//tolua_export
		append<uint64>(value);
		return *this;
	}//tolua_export
	// signed as in 2e complement
	ByteBuffer &operator<<(int8 value) {//tolua_export
		append<int8>(value);
		return *this;
	}//tolua_export
	ByteBuffer &operator<<(int16 value) {//tolua_export
		append<int16>(value);
		return *this;
	}//tolua_export
	ByteBuffer &operator<<(int32 value) {//tolua_export
		append<int32>(value);
		return *this;
	}//tolua_export
	ByteBuffer &operator<<(int64 value) {//tolua_export
		append<int64>(value);
		return *this;
	}//tolua_export
	ByteBuffer &operator<<(float value) {//tolua_export
		append<float>(value);
		return *this;
	}//tolua_export
	ByteBuffer &operator<<(double value) {//tolua_export
		append<double>(value);
		return *this;
	}//tolua_export
	ByteBuffer &operator<<(const std::string &value) {//tolua_export
		append(value);
		return *this;
	}//tolua_export
	ByteBuffer &operator<<(const char *str) {//tolua_export
		append(str);
		return *this;
	}//tolua_export
	ByteBuffer &operator<<(char *str) {//tolua_export
		append(str);
		return *this;
	}//tolua_export

	// stream like operators for reading data
	ByteBuffer &operator>>(bool &value) {//tolua_export
		value = read<char>() > 0 ? true : false;
		return *this;
	}//tolua_export
	//unsigned
	ByteBuffer &operator>>(uint8 &value) {//tolua_export
		value = read<uint8>();
		return *this;
	}//tolua_export
	ByteBuffer &operator>>(uint16 &value) {//tolua_export
		value = read<uint16>();
		return *this;
	}//tolua_export
	ByteBuffer &operator>>(uint32 &value) {//tolua_export
		value = read<uint32>();
		return *this;
	}//tolua_export
	ByteBuffer &operator>>(uint64 &value) {//tolua_export
		value = read<uint64>();
		return *this;
	}//tolua_export
	//signed as in 2e complement
	ByteBuffer &operator>>(int8 &value) {//tolua_export
		value = read<int8>();
		return *this;
	}//tolua_export
	ByteBuffer &operator>>(int16 &value) {//tolua_export
		value = read<int16>();
		return *this;
	}//tolua_export
	ByteBuffer &operator>>(int32 &value) {//tolua_export
		value = read<int32>();
		return *this;
	}//tolua_export
	ByteBuffer &operator>>(int64 &value) {//tolua_export
		value = read<int64>();
		return *this;
	}//tolua_export
	ByteBuffer &operator>>(float &value) {//tolua_export
		value = read<float>();
		return *this;
	}//tolua_export
	ByteBuffer &operator>>(double &value) {//tolua_export
		value = read<double>();
		return *this;
	}//tolua_export
	ByteBuffer &operator>>(std::string& value) {//tolua_export
		read(value);
		return *this;
	}//tolua_export

	uint8 operator[](size_t pos) {
		return show<uint8>(pos);
	}
    template <typename T> T show(size_t pos) const {
        //ASSERT(pos + sizeof(T) <= size());
        if (pos + sizeof(T) > size())
        {
            return (T)(0);
        }
        else {
            return *((T*)&_storage[pos]);
        }
    }

	size_t rpos() {//tolua_export
		return _rpos;
	}//tolua_export
	size_t rpos(size_t rpos) {//tolua_export
		_rpos = rpos;
		return _rpos;
	}//tolua_export
	size_t wpos() {//tolua_export
		return _wpos;
	}//tolua_export
	size_t wpos(size_t wpos) {//tolua_export
		_wpos = wpos;
		return _wpos;
	}//tolua_export

	template <typename T> T read() {
		T r = show<T>(_rpos);
		_rpos += sizeof(T);
		return r;
	}
	void read(uint8 *dest, size_t len) {//tolua_export
		if (_rpos + len <= size()) {
			memcpy(dest, &_storage[_rpos], len);
		} else {
			//throw error();
			memset(dest, 0, len);
		}
		_rpos += len;
	}//tolua_export
    void read(std::string& value) {
        uint16 len = read<uint16>();
        value.assign((const char*)contentsRpos(), len);
        _rpos += len;
    }
	//void read(std::string& value, size_t len) {
	//	value.clear();
	//	while (--len >= 0) {
	//		char c = read<char>();
	//		value += c; // ������ʱstring������ƴ�ӣ�/(��o��)/~~
	//	}
	//	/* ����ӿڲ���ȫ�쳣�����
	//		1��len����
	//		2��buf���ж��'/0'
	//	*/
	//}

	const uint8* contents() const { assert(_storage.size()); return _storage.size() ? &_storage[0] : NULL; }//tolua_export
	const uint8* contentsRpos() const { return &_storage[_rpos]; }
	const uint8* contentsWpos() const { return &_storage[_wpos]; }

	// appending to the end of buffer
	void append(const std::string& str) {//tolua_export
        uint16 len = str.size();
        append(len);
        append((uint8*)str.c_str(), len);
	}//tolua_export

	// NOTICE�������ַ���������ʽת��Ϊstring
	// NOTICE������const char* ���أ���ƥ���template����
	void append(const char* str) {
        uint16 len = strlen(str);
        append(len);
        append((const uint8*)str, len);
	}
    void append(char* str) {
        uint16 len = strlen(str);
        append(len);
        append((const uint8*)str, len);
    }
	void append(const uint8 *src, size_t cnt) {//tolua_export
		if (!cnt) return;

		// noone should even need uint8buffer longer than 10mb
		// if you DO need, think about it
		// then think some more
		// then use something else
		// -- qz
		assert(size() < 10000000);

		if (_storage.size() < _wpos + cnt)
			_storage.resize(_wpos + cnt);
		memcpy(&_storage[_wpos], src, cnt);
		_wpos += cnt;
	}//tolua_export
	void append(const ByteBuffer& buffer) {//tolua_export
		if (buffer.size() > 0) append(buffer.contents(), buffer.size());
	}//tolua_export
    template <typename T> void append(T value) {
        append((uint8 *)&value, sizeof(value));
    }

	void hexlike()
	{
		uint8 val = 0;
		uint32 j = 1, k = 1;
		printf("STORAGE_SIZE: %u\n", (unsigned int)size());
		for (uint32 i = 0; i < size(); ++i)
		{
			val = show<uint8>(i);

			if (i == 8*j && i != 16*k)
			{
				(val <= 0x0F) ? printf("| 0%X ", val) : printf("| %X ", val);

				++j;
			}
			else if (i == 16*k)
			{
				rpos(rpos() - 16);	// move read pointer 16 places back
				printf(" | ");	  // write split char

				for (int x = 0; x < 16; ++x) printf("%c", show<uint8>(i - 16 + x));

				(val <= 0x0F) ? printf("\n0%X ", val) : printf("\n%X ", val);

				++k; ++j;
			}
			else
			{
				(val <= 0x0F) ? printf("0%X ", val) : printf("%X ", val);
			}
		}
		printf("\n");
	}

	void reverse() {//tolua_export
		std::reverse(_storage.begin(), _storage.end());
	}//tolua_export

	//compare
	bool operator== (ByteBuffer & bb2)
	{
		ByteBuffer const& bb1 = *this;
		if (bb1.size() != bb2.size())
		{
			return false;
		}
		if (memcmp(bb1.contents(), bb2.contents(), bb1.size()) != 0)
		{
			return false;
		}
		return true;

	}
};//tolua_export


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <typename T> ByteBuffer &operator<<(ByteBuffer &b, std::vector<T> v)
{
	b << (uint32)v.size();
	for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); ++i) {
		b << *i;
	}
	return b;
}
template <typename T> ByteBuffer &operator>>(ByteBuffer &b, std::vector<T> &v)
{
	uint32 vsize; T t;
	b >> vsize;
	v.clear();
	while (--vsize >= 0) {
		b >> t;
		v.push_back(t);
	}
	return b;
}

template <typename T> ByteBuffer &operator<<(ByteBuffer &b, std::list<T> v)
{
	b << (uint32)v.size();
	for (typename std::list<T>::iterator i = v.begin(); i != v.end(); ++i) {
		b << *i;
	}
	return b;
}
template <typename T> ByteBuffer &operator>>(ByteBuffer &b, std::list<T> &v)
{
	uint32 vsize; T t;
	b >> vsize;
	v.clear();
	while (--vsize >= 0) {
		b >> t;
		v.push_back(t);
	}
	return b;
}

template <typename T> ByteBuffer &operator<<(ByteBuffer &b, std::set<T> v)
{
	b << (uint32)v.size();
	for (typename std::set<T>::iterator i = v.begin(); i != v.end(); ++i) {
		b << *i;
	}
	return b;
}
template <typename T> ByteBuffer &operator>>(ByteBuffer &b, std::set<T> &v)
{
	uint32 vsize; T t;
	b >> vsize;
	v.clear();
	while (--vsize >= 0) {
		b >> t;
		v.insert(t);
	}
	return b;
}

template <typename K, typename V> ByteBuffer &operator<<(ByteBuffer &b, std::map<K, V> &m)
{
	b << (uint32)m.size();
	for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); ++i) {
		b << i->first << i->second;
	}
	return b;
}
template <typename K, typename V> ByteBuffer &operator>>(ByteBuffer &b, std::map<K, V> &m)
{
	uint32 msize; K k; V v;
	b >> msize;
	m.clear();
	while (--msize >= 0) {
		b >> k >> v;
		m.insert(make_pair(k, v));
	}
	return b;
}

template <typename K, typename V> ByteBuffer &operator<<(ByteBuffer &b, stdext::hash_map<K, V> &m)
{
	b << (uint32)m.size();
	for (typename stdext::hash_map<K, V>::iterator i = m.begin(); i != m.end(); ++i) {
		b << i->first << i->second;
	}
	return b;
}
template <typename K, typename V> ByteBuffer &operator>>(ByteBuffer &b, stdext::hash_map<K, V> &m)
{
	uint32 msize; K k; V v;
	b >> msize;
	m.clear();
	while (--msize >= 0) {
		b >> k >> v;
		m.insert(make_pair(k, v));
	}
	return b;
}