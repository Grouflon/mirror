#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace mirror
{
	class Class;
	class TypeDesc;
	struct MetaDataSet; 
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

			template<>
			void write<std::string>(const std::string& _string)
			{
				size_t size = _string.size();
				write(size);
				write(_string.data(), size);
			}

			void write(const void* _data, size_t _size);

			template <typename T>
			bool read(T& _object)
			{
				static_assert(std::is_arithmetic<T>::value, "T is not of arithmetic type");

				size_t size = sizeof(_object);
				return read(&_object, size);
			}

			template<>
			bool read<std::string>(std::string& _string)
			{
				size_t size = 0;
				if (!read(size))
					return false;

				_string.resize(size);
				if (!read(const_cast<char*>(_string.data()), size))
					return false;

				return true;
			}

			bool read(void* _data, size_t _size);

			void reserve(size_t _size);
		};

		void _serializeEntry(FDataBuffer* _dataBuffer, const char* _id, void* _object, const TypeDesc* _typeDesc, const MetaDataSet* _metaDataSet = nullptr);
		void _serialize(FDataBuffer* _dataBuffer, void* _object, const TypeDesc* _typeDesc, const MetaDataSet* _metaDataSet = nullptr);
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


	static BinarySerializer s_staticSerializer;

	template <typename T>
	static bool SaveToFile(T& _data, const char* _fileName)
	{
		s_staticSerializer.beginWrite();
		s_staticSerializer.serialize("", _data);
		s_staticSerializer.endWrite();

		const void* data = nullptr;
		size_t dataSize = 0u;
		s_staticSerializer.getWriteData(data, dataSize);

		FILE* fp = fopen(_fileName, "w");
		if (!fp)
			return false;

		fwrite(data, dataSize, 1, fp);
		fclose(fp);

		return true;
	}

	template <typename T>
	static bool LoadFromFile(T& _data, const char* _fileName)
	{
		size_t dataSize = 0u;
		void* data = nullptr;

		FILE* fp = fopen(_fileName, "r");
		if (!fp)
			return false;

		fseek(fp, 0, SEEK_END);
		dataSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		data = malloc(dataSize);

		fread(data, dataSize, 1, fp);
		fclose(fp);

		s_staticSerializer.beginRead(data, dataSize);
		s_staticSerializer.serialize("", _data);
		s_staticSerializer.endRead();

		free(data);

		return true;
	}
}
