#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace mirror
{
	class Class;
	class TypeDesc;
	class StdVectorTypeDescBase;

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

		template <typename T> void serialize(const char* _id, T& _object)
		{
			assert(m_isReading || m_isWriting);

			FDataBuffer* dataBuffer = nullptr;
			if (m_isWriting) dataBuffer = m_writeDataBuffer;
			else if (m_isReading) dataBuffer = &m_readDataBuffer;

			_serializeEntry( dataBuffer, _id, &_object, GetTypeDesc(_object));
		}

	private:

		struct FDataBuffer
		{
			FDataBuffer();
			FDataBuffer(uint8_t* _data, size_t _dataLength);
			~FDataBuffer();

			uint8_t* data = nullptr;
			size_t cursor = 0u;
			size_t dataLength = 0u;
			size_t dataAllocatedSize = 0u;

			bool isOwningData = true;

			template <typename T>
			void write(const T& _object)
			{
				static_assert(std::is_arithmetic<T>::value, "T is not of arithmetic type");

				size_t size = sizeof(_object);
				write(&_object, size);
			}
			void write(const void* _data, size_t _size);

			template <typename T>
			bool read(T& _object)
			{
				static_assert(std::is_arithmetic<T>::value, "T is not of arithmetic type");

				size_t size = sizeof(_object);
				return read(&_object, size);
			}
			bool read(void* _data, size_t _size);

			void reserve(size_t _size);
		};

		void _serializeEntry(FDataBuffer* _dataBuffer, const char* _id, void* _object, const TypeDesc* _typeDesc);
		void _serialize(FDataBuffer* _dataBuffer, void* _object, const TypeDesc* _typeDesc);
		template <typename T>
		void _serialize(FDataBuffer* _dataBuffer, void* _object)
		{
			if (m_isWriting)
			{
				_dataBuffer->write(*reinterpret_cast<T*>(_object));
			}
			else if (m_isReading)
			{
				// TODO: error handling
				_dataBuffer->read(*reinterpret_cast<T*>(_object));
			}
		}

		FDataBuffer* _getDataBufferFromPool();
		void _releaseDataBufferToPool(FDataBuffer* _dataBuffer);

		std::vector<FDataBuffer*> m_dataBufferPool;

		FDataBuffer* m_writeDataBuffer = nullptr;
		FDataBuffer m_readDataBuffer;

		bool m_isWriting = false;
		bool m_isReading = false;
	};
}
