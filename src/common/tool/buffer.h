/***********************************************************************
* @ ����ͨ��Buffer
* @ brief
	1�������"bytebuffer.h"������read���������α꣬�����ڴ��
	2��֧��prepend����(��bufferǰ��������)��������ɴ����ڴ��ƶ�
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
			: _buffer(kCheapPrepend + initialSize)
			, _rpos(kCheapPrepend)
			, _wpos(kCheapPrepend)
		{
			assert(readableBytes() == 0);
			assert(writableBytes() == initialSize);
			assert(prependBytes() == kCheapPrepend);
		}
        ~Buffer()
        {
			printf("---Buffer����---%s (%d)\n", read<string>().c_str(), size());// ���� AsyncLog
        }

        char* beginRead(){ return begin() + _rpos; }
        char* beginWrite(){ return begin() + _wpos; }
        size_t readableBytes() const { return _wpos - _rpos; } //buff�л��ж�������
        size_t writableBytes() const { return size() - _wpos; } //ʣ��ռ�
		size_t prependBytes() const { return _rpos; }
		size_t size() const { return _buffer.size(); }
		size_t capacity() const { return _buffer.capacity(); }
		void clear() { _rpos = _wpos = kCheapPrepend; }

//---------------------------д��---------------------------------------------------
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
				//Notice���ƶ��ڴ��̫���ˣ��ⲿ����ǰ����ж��£�len <= prependBytes()
				ensureWritableBytes(len - _rpos);
				std::copy(beginRead(), beginWrite(), begin() + len);
				_wpos += (len - _rpos);
				_rpos = 0;
			}
			std::copy(p, p + len, beginRead());
		}
		template <typename T> void append(T x) {
			append(&x, sizeof(x));
		}
		template <typename T> bool prepend(T x) {
			return prepend(&x, sizeof(x));
		}
		// NOTICE�������ַ���������ʽת��Ϊstring
		// NOTICE������const char* ���أ���ƥ���template����
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

//---------------------------������ָ���ƶ��������ݲ��ᱣ��----------------------------------------
        template <typename T> T read() { // �ⲿдbuf.read<string>()��Ц�ˡ���ƫ�ػ�
			T result(0);
			if (readableBytes() < sizeof(T))
			{
				return result;
			}
			::memcpy(&result, beginRead(), sizeof(T));
			readerMove(sizeof(T));
			return result;
		}
		template <> std::string read() { // ƫ�ػ������ֽӿڷ��ͳһ
			size_t len = 0;
			const char* begin = beginRead();
			const char* p = begin;
            while (*p) { ++p; ++len; }

            if (readableBytes() < len + 1)
            {
                return "";
            }
			readerMove(len + 1); // NOTICE����'/0'��β����
			return std::string(begin, len);
		}
		void readerMove(size_t len)
		{
			if (len < readableBytes())
				_rpos += len;
			else
				_rpos = _wpos = kCheapPrepend; // ȫ�����꣬���������
		}
		void writerMove(size_t len)
		{
			ensureWritableBytes(len);
			_wpos += len;
		}

//---------------------------���ô�С�������ݱ���------------------------------------
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

		void ensureWritableBytes(size_t len) //ȷ��ʣ��ռ��㹻��һ����д��
		{
			if (writableBytes() < len) makeSpace(len);

			assert(writableBytes() >= len);
		}
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
}

/************************************************************************/
// ʾ��
#ifdef _MY_Test
	void test_NetBuffer(){
		cout << "������������������������ net::Buffer ������������������������" << endl;
		net::Buffer buf(8);
		buf.append("aa");
		cout << "size:" << buf.size() << endl;
		buf.append(13);
		cout << "size:" << buf.size() << endl;
		buf.prepend("1234567890");
		cout << "size:" << buf.size() << endl;
		string s = buf.read<string>();
		cout << "prepend: " << s << ":" << s.size() << endl;
		//Notice���Զ������cout��������˳���ȵ������ /(��o��)/~~
		// cout << buf.read<string>() << "--int:" << buf.read<int>() << endl;
		cout << "append 1: " << buf.read<string>() << endl;
		cout << "append 2: " << buf.read<int>() << endl;
	}
#endif