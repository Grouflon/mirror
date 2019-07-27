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

		free(m_writeData);
	}

	void BinarySerializer::beginWrite()
	{
		assert(!m_isReading);
		assert(!m_isWriting);

		m_isWriting = true;
		m_writeDataLength = 0u;
	}

	void BinarySerializer::endWrite()
	{
		assert(m_isWriting);

		m_isWriting = false;
	}

	void BinarySerializer::getWriteData(const void*& _outData, size_t& _outDataLength) const
	{
		_outData = m_writeData;
		_outDataLength = m_writeDataLength;
	}

	void BinarySerializer::beginRead(const void* _data, size_t _dataLength)
	{
		assert(!m_isReading);
		assert(!m_isWriting);

		m_readData = reinterpret_cast<const uint8_t*>(_data);
		m_readDataLength = _dataLength;
		m_isReading = true;
		m_readCursor = 0;
	}

	void BinarySerializer::endRead()
	{
		assert(m_isReading);

		m_readData = nullptr;
		m_readDataLength = 0;
		m_isReading = false;
	}

	void BinarySerializer::serialize(void* _object, const TypeDesc* _typeDesc)
	{
		assert(_object);
		assert(_typeDesc);

		switch (_typeDesc->getType())
		{
		case Type_SimpleType_bool:
			serialize(reinterpret_cast<bool*>(_object));
			break;
		case Type_SimpleType_char:
			serialize(reinterpret_cast<char*>(_object));
			break;
		case Type_SimpleType_uint8:
			serialize(reinterpret_cast<uint8_t*>(_object));
			break;
		case Type_SimpleType_uint16:
			serialize(reinterpret_cast<uint16_t*>(_object));
			break;
		case Type_SimpleType_uint32:
			serialize(reinterpret_cast<uint32_t*>(_object));
			break;
		case Type_SimpleType_uint64:
			serialize(reinterpret_cast<uint64_t*>(_object));
			break;
		case Type_SimpleType_int8:
			serialize(reinterpret_cast<int8_t*>(_object));
			break;
		case Type_SimpleType_int16:
			serialize(reinterpret_cast<int16_t*>(_object));
			break;
		case Type_SimpleType_int32:
			serialize(reinterpret_cast<int32_t*>(_object));
			break;
		case Type_SimpleType_int64:
			serialize(reinterpret_cast<int64_t*>(_object));
			break;
		case Type_SimpleType_float:
			serialize(reinterpret_cast<float*>(_object));
			break;
		case Type_SimpleType_double:
			serialize(reinterpret_cast<double*>(_object));
			break;
		case Type_SimpleType_std_string:
			serialize(reinterpret_cast<std::string*>(_object));
			break;
		case Type_std_vector:
			serialize(_object, static_cast<const StdVectorTypeDescBase*>(_typeDesc));
			break;
		case Type_Class:
		{
			const Class* clss = static_cast<const Class*>(_typeDesc);
			for (const ClassMember& member : clss->getMembers())
			{
				serialize(member.getInstanceMemberPointer(_object), member.type);
			}
		}
		break;
		default:
			break;
		}
	}

	void BinarySerializer::serialize(bool* _object)
	{
		if (m_isWriting) _writeObject(_object);
		else if (m_isReading) _readObject(_object);
		else assert(false);
	}

	void BinarySerializer::serialize(char* _object)
	{
		if (m_isWriting) _writeObject(_object);
		else if (m_isReading) _readObject(_object);
		else assert(false);
	}

	void BinarySerializer::serialize(uint8_t* _object)
	{
		if (m_isWriting) _writeObject(_object);
		else if (m_isReading) _readObject(_object);
		else assert(false);
	}

	void BinarySerializer::serialize(uint16_t* _object)
	{
		if (m_isWriting) _writeObject(_object);
		else if (m_isReading) _readObject(_object);
		else assert(false);
	}

	void BinarySerializer::serialize(uint32_t* _object)
	{
		if (m_isWriting) _writeObject(_object);
		else if (m_isReading) _readObject(_object);
		else assert(false);
	}

	void BinarySerializer::serialize(uint64_t* _object)
	{
		if (m_isWriting) _writeObject(_object);
		else if (m_isReading) _readObject(_object);
		else assert(false);
	}

	void BinarySerializer::serialize(int8_t* _object)
	{
		if (m_isWriting) _writeObject(_object);
		else if (m_isReading) _readObject(_object);
		else assert(false);
	}

	void BinarySerializer::serialize(int16_t* _object)
	{
		if (m_isWriting) _writeObject(_object);
		else if (m_isReading) _readObject(_object);
		else assert(false);
	}

	void BinarySerializer::serialize(int32_t* _object)
	{
		if (m_isWriting) _writeObject(_object);
		else if (m_isReading) _readObject(_object);
		else assert(false);
	}

	void BinarySerializer::serialize(int64_t* _object)
	{
		if (m_isWriting) _writeObject(_object);
		else if (m_isReading) _readObject(_object);
		else assert(false);
	}

	void BinarySerializer::serialize(float* _object)
	{
		if (m_isWriting) _writeObject(_object);
		else if (m_isReading) _readObject(_object);
		else assert(false);
	}

	void BinarySerializer::serialize(double* _object)
	{
		if (m_isWriting) _writeObject(_object);
		else if (m_isReading) _readObject(_object);
		else assert(false);
	}

	void BinarySerializer::serialize(std::string* _object)
	{
		assert(_object);
		if (m_isWriting)
		{
			size_t length = _object->length() + 1;
			_reserve(length);
			memcpy(m_writeData + m_writeDataLength, _object->data(), length);
			m_writeDataLength += length;
		}
		else if (m_isReading)
		{
			*_object = reinterpret_cast<const char*>(m_readData + m_readCursor);
			m_readCursor += _object->length() + 1;
		}
		else assert(false);
	}

	void BinarySerializer::serialize(void* _object, const StdVectorTypeDescBase* _typeDesc)
	{
		assert(_object);
		size_t size;
		if (m_isWriting)
		{
			size = _typeDesc->instanceSize(_object);
			serialize(&size);
		}
		else if (m_isReading)
		{
			serialize(&size);
			_typeDesc->instanceResize(_object, size);
		}
		else assert(false);

		for (size_t i = 0; i < size; ++i)
		{
			serialize(_typeDesc->instanceGetDataPointerAt(_object, i), _typeDesc->getSubType());
		}
	}

	void BinarySerializer::_reserve(size_t _size)
	{
		if (m_writeDataLength + _size > m_writeDataSize)
		{
			m_writeDataSize = m_writeDataLength + _size * 2;
			m_writeData = reinterpret_cast<uint8_t*>(realloc(m_writeData, m_writeDataSize));
		}
	}

}
