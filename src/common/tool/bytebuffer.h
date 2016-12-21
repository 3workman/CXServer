#pragma once
#include <vector>
#include <list>
#include <map>
#include <set>
#include <hash_map>
#include <assert.h>

/*
	注意：【buffer输出，不要用位域变量接】
		1、底层取地址，memcpy时，极易写越界
		2、位域变量，用'&'取地址，不是真正的数据开头
		3、话说，位域变量进如何进行函数匹配的呢？ int64  a:2; b62;  ByteBuffer::read(a) 匹配进哪个？ read<int8>、read<int64>？
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
	//静态数据成员：只有一份拷贝，不属于任何一个对象
	//一般通过非内联函数来访问，因为编译器确保在调用任何一个非内联函数之前，初始化静态数据成员
	//当从【另一个】编译单元通过内联函数调用静态成员时，静态成员有可能尚未被初始化
	//C++对“定义于不同编译单元内的non-local static对象”的初始化相对次序无明确定义
	//C++保证，函数内的local static对象会在“该函数被调用期间”“首次遇上该对象之定义式”时被初始化
	const static size_t DEFAULT_SIZE = 0x1000;

	//vector的reserve增加了vector的capacity，但是它的size没有改变！而resize改变了vector的capacity同时也增加了它的size
	//reserve是容器预留空间，但在空间内不真正创建元素对象，所以在没有添加新的对象之前，不能引用容器内的元素
	//resize是改变容器的大小，且在创建对象，因此调用这个函数之后，就可以引用容器内的对象了
	//resize函数可以有两个参数，第一个参数是容器新的大小， 第二个参数是要加入容器中的新元素，如果这个参数被省略，那么就调用元素对象的默认构造函数
	//两者都会保留原容器内的数据
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
	//		value += c; // 生成临时string对象再拼接，/(ㄒoㄒ)/~~
	//	}
	//	/* 这个接口不安全异常情况：
	//		1、len超长
	//		2、buf中有多个'/0'
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

	// NOTICE：常量字符串不会隐式转换为string
	// NOTICE：若无const char* 重载，会匹配进template函数
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