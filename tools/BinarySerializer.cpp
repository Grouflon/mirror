#include "BinarySerializer.h"

#include <cassert>

#include "../mirror.h"

namespace mirror
{

	BinarySerializer::BinarySerializer()
	{

	}

	BinarySerializer::~BinarySerializer()
	{
		assert(!m_isReading);
		assert(!m_isWriting);

		if (m_writeDataBuffer)
			_releaseDataBufferToPool(m_writeDataBuffer);

		for (FDataBuffer* dataBuffer : m_dataBufferPool)
		{
			delete dataBuffer;
		}
	}

	void BinarySerializer::beginWrite()
	{
		assert(!m_isReading);
		assert(!m_isWriting);

		m_isWriting = true;

		if (!m_writeDataBuffer)
		{
			m_writeDataBuffer = _getDataBufferFromPool();
		}
		else
		{
			m_writeDataBuffer->dataLength = 0;
			m_writeDataBuffer->cursor = 0;
		}
	}

	void BinarySerializer::endWrite()
	{
		assert(m_isWriting);
		m_isWriting = false;
	}

	void BinarySerializer::getWriteData(const void*& _outData, size_t& _outDataLength) const
	{
		if (m_writeDataBuffer)
		{
			_outData = m_writeDataBuffer->data;
			_outDataLength = m_writeDataBuffer->dataLength;
		}
		else
		{
			_outData = nullptr;
			_outDataLength = 0u;
		}
	}

	void BinarySerializer::beginRead(const void* _data, size_t _dataLength)
	{
		assert(!m_isReading);
		assert(!m_isWriting);

		m_readDataBuffer = FDataBuffer(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(_data)), _dataLength);
		m_isReading = true;
	}

	void BinarySerializer::endRead()
	{
		assert(m_isReading);

		m_readDataBuffer = FDataBuffer();
		m_isReading = false;
	}

	void BinarySerializer::_serializeEntry(FDataBuffer* _dataBuffer, const char* _id, void* _object, const TypeDesc* _typeDesc)
	{
		assert(_dataBuffer);
		assert(_id);
		assert(_object);
		assert(_typeDesc);

		if (m_isWriting)
		{
			FDataBuffer* entryDataBuffer = _getDataBufferFromPool();
			_serialize(entryDataBuffer, _object, _typeDesc);

			_dataBuffer->write(_id, strlen(_id) + 1);
			_dataBuffer->write(entryDataBuffer->dataLength);
			_dataBuffer->write(entryDataBuffer->data, entryDataBuffer->dataLength);

			_releaseDataBufferToPool(entryDataBuffer);
		}
		else if (m_isReading)
		{
			_dataBuffer->cursor = 0u;
			while (_dataBuffer->cursor < _dataBuffer->dataLength)
			{
				char* id = (char*)(_dataBuffer->data + _dataBuffer->cursor);
				_dataBuffer->cursor += strlen(id) + 1;
				size_t payloadSize = 0u;
				_dataBuffer->read(payloadSize);

				if (strcmp(_id, id) != 0)
				{
					_dataBuffer->cursor += payloadSize;
					continue;
				}
				else
				{
					FDataBuffer entryDataBuffer(_dataBuffer->data + _dataBuffer->cursor, payloadSize);
					_serialize(&entryDataBuffer, _object, _typeDesc);
					break;
				}
			}
		}
	}

	void BinarySerializer::_serialize(FDataBuffer* _dataBuffer, void* _object, const TypeDesc* _typeDesc)
	{
		assert(_dataBuffer);
		assert(_object);
		assert(_typeDesc);

		switch (_typeDesc->getType())
		{
		case Type_bool:
			_serialize<bool>(_dataBuffer, _object);
			break;
		case Type_char:
			_serialize<char>(_dataBuffer, _object);
			break;
		case Type_uint8:
			_serialize<uint8_t>(_dataBuffer, _object);
			break;
		case Type_uint16:
			_serialize<uint16_t>(_dataBuffer, _object);
			break;
		case Type_uint32:
			_serialize<uint32_t>(_dataBuffer, _object);
			break;
		case Type_uint64:
			_serialize<uint64_t>(_dataBuffer, _object);
			break;
		case Type_int8:
			_serialize<int8_t>(_dataBuffer, _object);
			break;
		case Type_int16:
			_serialize<int16_t>(_dataBuffer, _object);
			break;
		case Type_int32:
			_serialize<int32_t>(_dataBuffer, _object);
			break;
		case Type_int64:
			_serialize<int64_t>(_dataBuffer, _object);
			break;
		case Type_float:
			_serialize<float>(_dataBuffer, _object);
			break;
		case Type_double:
			_serialize<double>(_dataBuffer, _object);
			break;
		case Type_std_string:
		{
			std::string* stringPtr = reinterpret_cast<std::string*>(_object);
			if (m_isWriting)
			{
				_dataBuffer->write(stringPtr->data(), stringPtr->length() + 1);
			}
			else if (m_isReading)
			{
				*stringPtr = reinterpret_cast<char *>(_dataBuffer->data + _dataBuffer->cursor);
			}
		}
		break;
		case Type_std_vector:
		{
			const StdVectorTypeDesc* vectorTypeDesc = static_cast<const StdVectorTypeDesc*>(_typeDesc);
			size_t vectorSize = 0u;
			if (m_isWriting)
			{
				vectorSize = vectorTypeDesc->instanceSize(_object);
				_dataBuffer->write(vectorSize);
			}
			else if (m_isReading)
			{
				_dataBuffer->read(vectorSize);
				vectorTypeDesc->instanceResize(_object, vectorSize);
			}
			for (size_t i = 0; i < vectorSize; ++i)
			{
				_serialize(_dataBuffer, vectorTypeDesc->instanceGetDataPointerAt(_object, i), vectorTypeDesc->getSubType());
			}
		}	
		break;
		case Type_Class:
		{
			const Class* clss = static_cast<const Class*>(_typeDesc);

			if (m_isWriting)
			{
				FDataBuffer* instanceDataBuffer = _getDataBufferFromPool();
				std::vector<mirror::ClassMember*> members;
				clss->getMembers(members);
				for (const ClassMember* member : members)
				{
					_serializeEntry(instanceDataBuffer, member->getName(), member->getInstanceMemberPointer(_object), member->getType());
				}
				_dataBuffer->write(instanceDataBuffer->dataLength);
				_dataBuffer->write(instanceDataBuffer->data, instanceDataBuffer->dataLength);
				_releaseDataBufferToPool(instanceDataBuffer);
			}
			else
			{
				size_t dataLength = 0u;
				_dataBuffer->read(dataLength);
				FDataBuffer instanceDataBuffer = FDataBuffer(_dataBuffer->data + _dataBuffer->cursor, dataLength);
				std::vector<mirror::ClassMember*> members;
				clss->getMembers(members);
				for (const ClassMember* member : members)
				{
					_serializeEntry(&instanceDataBuffer, member->getName(), member->getInstanceMemberPointer(_object), member->getType());
				}
				_dataBuffer->cursor += dataLength;
			}
		}
		break;
		default:
			break;
		}
	}

	mirror::BinarySerializer::FDataBuffer* BinarySerializer::_getDataBufferFromPool()
	{
		if (m_dataBufferPool.empty())
		{
			return new FDataBuffer();
		}
		else
		{
			FDataBuffer* dataBuffer = *(m_dataBufferPool.rbegin());
			m_dataBufferPool.pop_back();
			return dataBuffer;
		}
	}

	void BinarySerializer::_releaseDataBufferToPool(FDataBuffer* _dataBuffer)
	{
		assert(_dataBuffer);
		assert(_dataBuffer->isOwningData);

		_dataBuffer->cursor = 0;
		_dataBuffer->dataLength = 0;
		m_dataBufferPool.push_back(_dataBuffer);
	}

	BinarySerializer::FDataBuffer::FDataBuffer()
	{
		isOwningData = true;
	}

	BinarySerializer::FDataBuffer::FDataBuffer(uint8_t* _data, size_t _dataLength)
	{
		data = _data;
		dataLength = dataAllocatedSize = _dataLength;
		isOwningData = false;
	}

	BinarySerializer::FDataBuffer::~FDataBuffer()
	{
		if (isOwningData)
		{
			free(data);
		}
	}

	void BinarySerializer::FDataBuffer::write(const void* _data, size_t _size)
	{
		assert(isOwningData);

		if (cursor + _size > dataAllocatedSize)
			reserve(cursor + _size);

		memcpy(data + cursor, _data, _size);
		cursor += _size;
		if (dataLength < cursor)
			dataLength = cursor;
	}

	bool BinarySerializer::FDataBuffer::read(void* _data, size_t _size)
	{
		if (cursor + _size > dataLength)
			return false;

		memcpy(_data, data + cursor, _size);
		cursor += _size;
		return true;
	}

	void BinarySerializer::FDataBuffer::reserve(size_t _size)
	{
		assert(isOwningData);

		if (dataLength + _size > dataAllocatedSize)
		{
			dataAllocatedSize = dataLength + _size * 2;
			data = reinterpret_cast<uint8_t*>(realloc(data, dataAllocatedSize));
		}
	}
}
