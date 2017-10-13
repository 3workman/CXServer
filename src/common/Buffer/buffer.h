/***********************************************************************
* @ 网络通信Buffer
* @ brief
	1、相比于"bytebuffer.h"，数据read完后会重置游标，复用内存块
	2、支持prepend操作(在buffer前插入数据)，可能造成大量内存移动
* @ author zhoumf
* @ date 2016-7-11
************************************************************************/
#pragma once
#include <vector>
#include <string>
#include <assert.h>


namespace net
{
	/// @code
	/// +-------------------+------------------+------------------+
	/// | prependable bytes |  readable bytes  |  writable bytes  |
	/// |                   |     (CONTENT)    |                  |
	/// +-------------------+------------------+------------------+
	/// |                   |                  |                  |
	/// 0      <=      readerIndex   <=   writerIndex    <=     size
	/// @endcode
	class Buffer
	{
		static const size_t kCheapPrepend = 8;
		static const size_t kInitialSize = 1024;
	public:
		explicit Buffer(size_t initialSize = kInitialSize)
			: _buffer(kCheapPrepend + initialSize) // resize
			, _rpos(kCheapPrepend)
			, _wpos(kCheapPrepend)
		{
			assert(readableBytes() == 0);
			assert(writableBytes() == initialSize);
			assert(prependBytes() == kCheapPrepend);
		}

        char* beginRead(){ return begin() + _rpos; }
        char* beginWrite(){ return begin() + _wpos; }
        size_t readableBytes() const { return _wpos - _rpos; } //buff中还有多少数据
        size_t writableBytes() const { return size() - _wpos; } //剩余空间
		size_t prependBytes() const { return _rpos; }
		size_t size() const { return _buffer.size(); }
		size_t capacity() const { return _buffer.capacity(); }
		void clear() { _rpos = _wpos = kCheapPrepend; }

//---------------------------写入---------------------------------------------------
		void append(const void* data, size_t len){
			ensureWritableBytes(len);
			const char* p = static_cast<const char*>(data);
			std::copy(p, p + len, beginWrite());
			_wpos += len;
		}
		void prepend(const void* data, size_t len){
			const char* p = static_cast<const char*>(data);
			if (len <= _rpos){
				_rpos -= len;
			}else{
				//Notice：移动内存块太耗了，外部调用前最好判断下：len <= prependBytes()
				ensureWritableBytes(len - _rpos);
				std::copy(beginRead(), beginWrite(), begin() + len);
				_wpos += (len - _rpos);
				_rpos = 0;
			}
			std::copy(p, p + len, beginRead());
		}
		template <typename T> void append(const T& x) {
			append(&x, sizeof(x));
		}
		template <typename T> bool prepend(const T& x) {
			return prepend(&x, sizeof(x));
		}
        
		// NOTICE：常量字符串不会隐式转换为string
		// NOTICE：若无const char* 重载，会匹配进template函数
		void append(const std::string& str) {
			append(str.c_str(), str.size() + 1);
		}
		void prepend(const std::string& str) {
			prepend(str.c_str(), str.size() + 1);
		}
		void append(const char* str) {
			append(str, strlen(str) + 1);
		}
		void prepend(const char* str) {
			prepend(str, strlen(str) + 1);
		}

//---------------------------读出，指针移动，旧数据不会保留----------------------------------------
        template <typename T> T read() { // 外部写buf.read<string>()就笑了——偏特化
			T result(0);
			if (readableBytes() < sizeof(T))
			{
				return result;
			}
			::memcpy(&result, beginRead(), sizeof(T));
			readerMove(sizeof(T));
			return result;
		}

		void readerMove(size_t len)
		{
			if (len < readableBytes())
				_rpos += len;
			else
				_rpos = _wpos = kCheapPrepend; // 全部读完，回退至起点
		}
		void writerMove(size_t len)
		{
			ensureWritableBytes(len);
			_wpos += len;
		}
        void ensureWritableBytes(size_t len) //确保剩余空间足够，一定能写入
        {
            if (writableBytes() < len) makeSpace(len);

            assert(writableBytes() >= len);
        }

//---------------------------重置大小，旧内容保留------------------------------------
        void shrink(size_t reserve)
		{
			// FIXME: use vector::shrink_to_fit() in C++ 11 if possible.
			Buffer other;
			other.ensureWritableBytes(readableBytes() + reserve);
			other.append(beginRead(), readableBytes());
			swap(other);
		}
        void swap(Buffer& rhs)
        {
            _buffer.swap(rhs._buffer);
            std::swap(_rpos, rhs._rpos);
            std::swap(_wpos, rhs._wpos);
        }
	private:
		char* begin(){ return &*_buffer.begin(); }

		void makeSpace(size_t len)
		{
			if (writableBytes() + prependBytes() < len + kCheapPrepend)
			{
				_buffer.resize(_wpos + len);
			}
			else
			{
				// move readable data to the front, make space inside buffer
				size_t oldReadable = readableBytes();
				std::copy(beginRead(), beginWrite(), begin() + kCheapPrepend);
				_rpos = kCheapPrepend;
				_wpos = _rpos + oldReadable;
			}
		}
	private:
		std::vector<char> _buffer;
		size_t _rpos;
		size_t _wpos;
	};
    
    template<> inline std::string Buffer::read<std::string>()
    {
        size_t len = 0;
        const char* begin = beginRead();
        const char* p = begin;
        while (*p) { ++p; ++len; }
        
        if (readableBytes() < len + 1)
        {
            return "";
        }
        readerMove(len + 1); // NOTICE：把'/0'结尾读走
        return std::string(begin, len);
    }
}

/************************************************************************/
// 示例
#ifdef _MY_Test
	void test_NetBuffer(){
		cout << "———————————— net::Buffer ————————————" << endl;
		net::Buffer buf(8);
		buf.append("aa");
		cout << "size:" << buf.size() << endl;
		buf.append(13);
		cout << "size:" << buf.size() << endl;
		buf.prepend("1234567890");
		cout << "size:" << buf.size() << endl;
		string s = buf.read<string>();
		cout << "prepend: " << s << ":" << s.size() << endl;
		//Notice：脑洞清奇的cout函数调用顺序，先调后面的 /(ㄒoㄒ)/~~
		// cout << buf.read<string>() << "--int:" << buf.read<int>() << endl;
		cout << "append 1: " << buf.read<string>() << endl;
		cout << "append 2: " << buf.read<int>() << endl;
	}
#endif
