#include "mirror.h"
#include "mirror_base.h"


#include <cstring>
#include <cassert>
#include <algorithm>

namespace mirror
{
	TypeSet g_typeSet;
	std::unordered_map<size_t, StaticFunction*> g_functionsByTypeHash;

	MetaData::MetaData(const char* _name, const char* _data)
		: m_name(_name)
		, m_data(_data)
	{

	}

	const char* MetaData::getName() const
	{
		return m_name.c_str();
	}

	bool MetaData::asBool() const
	{
		return false;
	}

	int MetaData::asInt() const
	{
		return 0;
	}

	float MetaData::asFloat() const
	{
		return 0.f;
	}

	const char* MetaData::asString() const
	{
		return m_data.c_str();
	}

#define OFFSET_BASIS	2166136261
#define FNV_PRIME		16777619

	uint32_t Hash32(const void* _data, size_t _size)
	{
		if (_size == 0)
		{
			return 0;
		}

		// FNV-1a algorithm
		// http://isthe.com/chongo/tech/comp/fnv/
		uint32_t hash = OFFSET_BASIS;
		const uint8_t* buf = static_cast<const uint8_t*>(_data);
		for (uint32_t i = 0u; i < _size; i++)
		{
			hash = hash * FNV_PRIME;
			hash = hash ^ *buf;
			++buf;
		}
		return hash;
	}

	uint32_t HashCString(const char* _str)
	{
		return Hash32(_str, strlen(_str));
	}

	void sanitizeMetaDataString(char* _buf)
	{
		assert(_buf);

		bool hasFirstChar = false;
		char* firstChar = _buf;
		char* lastChar = _buf;
		for (char* cur = _buf; *cur != 0; ++cur)
		{
			if (*cur != ' ')
			{
				if (!hasFirstChar)
				{
					firstChar = cur;
					lastChar = cur;
					hasFirstChar = true;
				}
				else
				{
					lastChar = cur + 1;
				}
				
			}
		}

		size_t len = lastChar - firstChar;
		strncpy(_buf, firstChar, len);
		_buf[len] = 0;
	}

	ClassMember::ClassMember(const char* _name, size_t _offset, TypeDesc* _type, const char* _metaDataString)
		: m_name(_name)
		, m_offset(_offset)
		, m_type(_type)
	{
		// Parse meta data
		assert(_metaDataString);
		size_t len = strlen(_metaDataString);

		const char* key = _metaDataString;
		size_t keyLen = 0;
		const char* value = nullptr;
		size_t valueLen = 0;
		int mode = 0;
		for (const char* cur = _metaDataString; cur <= _metaDataString + len; ++cur)
		{
			if (*cur == ',' || cur == _metaDataString + len)
			{
				if (value == nullptr)
				{
					keyLen = cur - key;
				}
				else
				{
					valueLen = cur - value;
				}

				if (keyLen == 0)
					continue;

				assert(keyLen < 256);
				char keyBuf[256];
				char valueBuf[256];

				strncpy(keyBuf, key, keyLen);
				keyBuf[keyLen] = 0;

				if (value)
				{
					strncpy(valueBuf, value, valueLen);
				}
				valueBuf[valueLen] = 0;

				sanitizeMetaDataString(keyBuf);
				sanitizeMetaDataString(valueBuf);

				MetaData metaData(keyBuf, valueBuf);
				m_metaData.insert(std::make_pair(HashCString(metaData.getName()), metaData));

				key = cur + 1;
			}
			else if (value == nullptr && *cur == '=')
			{
				keyLen = cur - key;
				value = cur + 1;
			}
		}
	}

	void* ClassMember::getInstanceMemberPointer(void* _classInstancePointer) const
	{
		return reinterpret_cast<uint8_t*>(_classInstancePointer) + m_offset;
	}

	mirror::MetaData* ClassMember::getMetaData(const char* _key) const
	{
		return nullptr;
	}

	Class::Class(const char* _name, size_t _typeHash)
		: TypeDesc(Type_Class, _name, _typeHash)
	{

	}

	Class::~Class()
	{
		for (ClassMember* member : m_members)
		{
			delete member;
		}
	}

	void Class::getMembers(std::vector<ClassMember*>& _outMemberList, bool _includeInheritedMembers) const
	{
		_outMemberList.insert(_outMemberList.end(), m_members.begin(), m_members.end());

		if (_includeInheritedMembers)
		{
			for (Class* parent : m_parents)
			{
				parent->getMembers(_outMemberList, true);
			}
		}
	}

	mirror::ClassMember* Class::findMemberByName(const char* _name, bool _includeInheritedMembers) const
	{
		uint32_t nameHash = HashCString(_name);
		auto it = m_membersByName.find(nameHash);
		if (it != m_membersByName.end())
		{
			return it->second;
		}
		if (_includeInheritedMembers)
		{
			for (Class* parent : m_parents)
			{
				ClassMember* member = parent->findMemberByName(_name);
				if (member)
					return member;
			}
		}
		return nullptr;
	}

	bool Class::isChildOf(const Class* _class, bool _checkSelf) const
	{
		if (_class == this)
			return _checkSelf;

		for (Class* parent : m_parents)
		{
			return parent->isChildOf(_class, true);
		}
		return false;
	}

	void Class::addMember(ClassMember* _member)
	{
		assert(_member);
		assert(std::find(m_members.begin(), m_members.end(), _member) == m_members.end());
		uint32_t nameHash = HashCString(_member->getName());
		assert(m_membersByName.find(nameHash) == m_membersByName.end());

		_member->m_class = this;
		m_members.push_back(_member);
		m_membersByName.insert(std::make_pair(nameHash, _member));
	}

	void Class::addParent(Class* _parent)
	{
		assert(_parent);
		assert(std::find(m_parents.begin(), m_parents.end(), _parent) == m_parents.end());
		assert(std::find(_parent->m_children.begin(), _parent->m_children.end(), this) == _parent->m_children.end());

		m_parents.insert(_parent);
		_parent->m_children.insert(this);
	}

	TypeSet::~TypeSet()
	{
		for (TypeDesc* type : m_types)
		{
			delete type;
		}
		m_types.clear();
		m_typesByTypeHash.clear();
	}

	TypeDesc* TypeSet::findTypeByTypeHash(size_t _typeHash)
	{
		auto it = m_typesByTypeHash.find(_typeHash);
		return it != m_typesByTypeHash.end() ? it->second : nullptr;
	}

	void TypeSet::addType(TypeDesc* _type)
	{
		// Checks if a type with the same typehash does not already exists
		assert(_type);
		assert(m_typesByTypeHash.find(_type->getTypeHash()) == m_typesByTypeHash.end());
		assert(m_types.find(_type) == m_types.end());

		m_typesByTypeHash.insert(std::make_pair(_type->getTypeHash(), _type));
		m_types.emplace(_type);
	}

	void TypeSet::removeType(TypeDesc* _type)
	{
		assert(_type);

		// Checks if a type with the same typehash already exists
		auto it = m_typesByTypeHash.find(_type->getTypeHash());
		assert(it != m_typesByTypeHash.end());
		m_typesByTypeHash.erase(it);

		auto it2 = m_types.find(_type);
		assert(it2 != m_types.end());
		m_types.erase(it2);
	}

	const std::set<TypeDesc*>& TypeSet::GetTypes() const
	{
		return m_types;
	}

	PointerTypeDesc::PointerTypeDesc(size_t _typeHash, TypeDesc* _subType)
		: TypeDesc(Type_Pointer, "pointer", _typeHash)
		, m_subType(_subType)
	{
		
	}

	Enum::Enum(const char* _name, size_t _typeHash)
		: TypeDesc(Type_Enum, _name, _typeHash)
	{

	}

	bool Enum::getValueFromString(const char* _string, int& _outValue) const
	{
		if (_string == nullptr)
			return false;

		size_t hash = HashCString(_string);
		auto it = m_valuesByNameHash.find(hash);
		if (it != m_valuesByNameHash.end())
		{
			_outValue = it->second->getValue();
			return true;
		}
		return false;
	}

	bool Enum::getStringFromValue(int _value, const char*& _outString) const
	{
		for (auto it = m_values.begin(); it != m_values.end(); ++it)
		{
			EnumValue* value = *it;
			if (value->getValue() == _value)
			{
				_outString = value->getName();
				return true;
			}
		}
		return false;
	}

	const std::vector<EnumValue*>& Enum::getValues() const
	{
		return m_values;
	}

	void Enum::addValue(EnumValue* _value)
	{
		assert(_value != nullptr);
		assert(std::find(m_values.begin(), m_values.end(), _value) == m_values.end());
		size_t hash = HashCString(_value->getName());
		assert(m_valuesByNameHash.find(hash) == m_valuesByNameHash.end());

		m_values.push_back(_value);
		m_valuesByNameHash.insert(std::make_pair(hash, _value));
	}

	EnumValue::EnumValue(const char* _name, int _value)
		: m_name(_name)
		, m_value(_value)
	{

	}

	const char* EnumValue::getName() const
	{
		return m_name.c_str();
	}

	int EnumValue::getValue() const
	{
		return m_value;
	}

}

