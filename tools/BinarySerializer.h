#pragma once

#include <cassert>
#include <cstdint>
#include <string>

namespace mirror
{
	class Class;

	class BinarySerializer
	{
	public:
		BinarySerializer();
		~BinarySerializer();

		void beginWrite();
		void endWrite();
		void getWriteData(const void*& _outData, size_t& _outDataLength) const;

		void beginRead(const void* _data, size_t _dataLength);
		void endRead();

		template <typename T> void serialize(T* _object)
		{
			serialize(_object, T::GetClass());
		}

		void serialize(void* _object, const Class* _class);
		void serialize(bool* _object);
		void serialize(char* _object);
		void serialize(uint8_t* _object);
		void serialize(uint16_t* _object);
		void serialize(uint32_t* _object);
		void serialize(uint64_t* _object);
		void serialize(int8_t* _object);
		void serialize(int16_t* _object);
		void serialize(int32_t* _object);
		void serialize(int64_t* _object);
		void serialize(float* _object);
		void serialize(double* _object);
		void serialize(std::string* _object);
	private:

		void _reserve(size_t _size);

		template <typename T> void _writeObject(T* _object)
		{
			assert(_object);
			size_t size = sizeof(*_object);
			_reserve(size);
			memcpy(m_writeData + m_writeDataLength, _object, size);
			m_writeDataLength += size;
		}

		template <typename T> void _readObject(T* _object)
		{
			assert(_object);
			size_t size = sizeof(*_object);
			assert(m_readCursor + size <= m_readDataLength);
			memcpy(_object, m_readData + m_readCursor, size);
			m_readCursor += size;
		}

		uint8_t* m_writeData = nullptr;
		size_t m_writeDataSize = 0u;
		size_t m_writeDataLength = 0u;

		const uint8_t* m_readData = nullptr;
		size_t m_readDataLength = 0u;
		size_t m_readCursor = 0u;

		bool m_isWriting = false;
		bool m_isReading = false;
	};
}
