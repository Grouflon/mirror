#include "mirror.h"
#include "mirror_base.h"


#include <cstring>
#include <cassert>
#include <algorithm>

namespace mirror
{

// ------------- TOOLS -----------------

#define ALLOCATE_AND_COPY_STRING(_dst, _src) { _dst = (char*)malloc(strlen(_src) + 1); strcpy(_dst, _src); }

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

	void SanitizeMetaDataString(char* _buf)
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

// ------------- !TOOLS -----------------

	TypeSet g_typeSet;
	TypeSet* GetTypeSet()
    {
        return &g_typeSet;
    }

	TypeDesc* FindTypeByName(const char* _name)
	{
		return ::mirror::g_typeSet.findTypeByName(_name);
	}

	MetaData::MetaData(const char* _name, const char* _data)
	{
		ALLOCATE_AND_COPY_STRING(m_name, _name);
        ALLOCATE_AND_COPY_STRING(m_data, _data);
	}

	MetaData::~MetaData()
    {
        free(m_name);
        free(m_data);
    }

	const char* MetaData::getName() const
	{
		return m_name;
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
		return m_data;
	}

	MetaDataSet::MetaDataSet(const char* _metaDataString)
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

				SanitizeMetaDataString(keyBuf);
				SanitizeMetaDataString(valueBuf);

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

	MetaDataSet::MetaDataSet(const MetaDataSet& _other)
	{
		m_metaData = _other.m_metaData;
	}

	const mirror::MetaData* MetaDataSet::findMetaData(const char* _key) const
	{
		if (_key == nullptr)
			return nullptr;

		uint32_t hash = HashCString(_key);
		auto it = m_metaData.find(hash);
		if (it == m_metaData.end())
			return nullptr;

		return &it->second;
	}

	TypeDesc::TypeDesc(Type _type, const char* _name, VirtualTypeWrapper* _virtualTypeWrapper)
		: m_type(_type)
		, m_virtualTypeWrapper(_virtualTypeWrapper)
	{
		ALLOCATE_AND_COPY_STRING(m_name, _name);
	}

	TypeDesc::~TypeDesc()
	{
		if (m_virtualTypeWrapper) delete m_virtualTypeWrapper;
		free(m_name);
	}

	bool TypeDesc::hasFactory() const
	{
		return m_virtualTypeWrapper->hasFactory();
	}

	void* TypeDesc::instantiate() const
	{
		return m_virtualTypeWrapper->instantiate();
	}

	void TypeDesc::setName(const char* _name)
	{
		free(m_name);
		ALLOCATE_AND_COPY_STRING(m_name, _name);
	}

	ClassMember::ClassMember(const char* _name, size_t _offset, TypeID _type, const char* _metaDataString)
		: m_offset(_offset)
		, m_type(_type)
		, m_metaDataSet(_metaDataString)
	{
		ALLOCATE_AND_COPY_STRING(m_name, _name);
	}

	ClassMember::~ClassMember()
	{
		free(m_name);
	}

	TypeDesc* ClassMember::getType() const
	{
		return GetTypeSet()->findTypeByID(m_type);
	}

	void* ClassMember::getInstanceMemberPointer(void* _classInstancePointer) const
	{
		return reinterpret_cast<uint8_t*>(_classInstancePointer) + m_offset;
	}

	Class::Class(const char* _name, VirtualTypeWrapper* _virtualTypeWrapper, const char* _metaDataString)
		: Class(_name, _virtualTypeWrapper, MetaDataSet(_metaDataString))
	{

	}

	Class::Class(const char* _name, VirtualTypeWrapper* _virtualTypeWrapper, const MetaDataSet& _metaDataSet)
		: TypeDesc(Type_Class, _name, _virtualTypeWrapper)
		, m_metaDataSet(_metaDataSet)
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

	mirror::Class* Class::unsafeVirtualGetClass(void* _object) const
	{
		return getVirtualTypeWrapper()->unsafeVirtualGetClass(_object);
	}

	TypeSet::~TypeSet()
	{
		for (TypeDesc* type : m_types)
		{
			delete type;
		}
		m_types.clear();
		m_typesByID.clear();
		m_typesByName.clear();
	}

	TypeDesc* TypeSet::findTypeByID(TypeID _typeID)
	{
		auto it = m_typesByID.find(_typeID);
		return it != m_typesByID.end() ? it->second : nullptr;
	}

	mirror::TypeDesc* TypeSet::findTypeByName(const char* _name)
	{
		uint32_t nameHash = HashCString(_name);
		auto it = m_typesByName.find(nameHash);
		return it != m_typesByName.end() ? it->second : nullptr;
	}

	void TypeSet::addType(TypeDesc* _type)
	{
		assert(_type);
		uint32_t nameHash = HashCString(_type->getName());

		// Checks if a type with the same typeID or namehash does not already exists
		assert(m_typesByID.find(_type->getTypeID()) == m_typesByID.end());
		assert(m_typesByName.find(nameHash) == m_typesByName.end());
		assert(m_types.find(_type) == m_types.end());

		m_typesByID.insert(std::make_pair(_type->getTypeID(), _type));
		m_typesByName.insert(std::make_pair(nameHash, _type));
		m_types.emplace(_type);
	}

	void TypeSet::removeType(TypeDesc* _type)
	{
		assert(_type);

		// Checks if a type with the same typeID or namehash already exists
		{
			auto it = m_typesByID.find(_type->getTypeID());
			assert(it != m_typesByID.end());
			m_typesByID.erase(it);
		}
		
		{
			uint32_t nameHash = HashCString(_type->getName());
			auto it = m_typesByName.find(nameHash);
			assert(it != m_typesByName.end());
			m_typesByName.erase(it);
		}
		
		{
			auto it = m_types.find(_type);
			assert(it != m_types.end());
			m_types.erase(it);
		}
	}

	const std::set<TypeDesc*>& TypeSet::getTypes() const
	{
		return m_types;
	}

	PointerTypeDesc::PointerTypeDesc(TypeID _subType, VirtualTypeWrapper* _virtualTypeWrapper)
		: TypeDesc(Type_Pointer, "", _virtualTypeWrapper)
		, m_subType(_subType)
	{
		setName((std::string("pointer_") + GetTypeSet()->findTypeByID(_subType)->getName()).c_str());
	}

	TypeDesc* PointerTypeDesc::getSubType() const { return GetTypeSet()->findTypeByID(m_subType); }


	FixedSizeArrayTypeDesc::FixedSizeArrayTypeDesc(TypeID _subType, size_t _elementCount, VirtualTypeWrapper* _virtualTypeWrapper)
		: TypeDesc(Type_FixedSizeArray, "fixed_size_array", _virtualTypeWrapper)
		, m_subType(_subType)
		, m_elementCount(_elementCount)
	{

	}

	TypeDesc* FixedSizeArrayTypeDesc::getSubType() const
	{
		return GetTypeSet()->findTypeByID(m_subType);
	}

	Enum::Enum(const char* _name, VirtualTypeWrapper* _virtualTypeWrapper, TypeDesc* _subType)
		: TypeDesc(Type_Enum, _name, _virtualTypeWrapper)
		, m_subType(_subType ? _subType : TypeDescGetter<int>::Get())
	{

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

	EnumValue::EnumValue(const char* _name, int64_t _value)
		: m_value(_value)
	{
		ALLOCATE_AND_COPY_STRING(m_name, _name)
	}

	EnumValue::~EnumValue()
	{
		free(m_name);
	}

	const char* EnumValue::getName() const
	{
		return m_name;
	}

	int64_t EnumValue::getValue() const
	{
		return m_value;
	}

#define TYPEDESCINITIALIZER_DEFINE(_type, _hasFactory, _mirrorType) TypeDescInitializer<_type, _hasFactory> g_##_type##TypeInitializer(_mirrorType, #_type)

	TYPEDESCINITIALIZER_DEFINE(void, false, Type_void);
	TYPEDESCINITIALIZER_DEFINE(bool, true, Type_bool);
	TYPEDESCINITIALIZER_DEFINE(char, true, Type_char);
	TYPEDESCINITIALIZER_DEFINE(int8_t, true, Type_int8);
	TYPEDESCINITIALIZER_DEFINE(int16_t, true, Type_int16);
	TYPEDESCINITIALIZER_DEFINE(int32_t, true, Type_int32);
	TYPEDESCINITIALIZER_DEFINE(int64_t, true, Type_int64);
	TYPEDESCINITIALIZER_DEFINE(uint8_t, true, Type_uint8);
	TYPEDESCINITIALIZER_DEFINE(uint16_t, true, Type_uint16);
	TYPEDESCINITIALIZER_DEFINE(uint32_t, true, Type_uint32);
	TYPEDESCINITIALIZER_DEFINE(uint64_t, true, Type_uint64);
	TYPEDESCINITIALIZER_DEFINE(float, true, Type_float);
	TYPEDESCINITIALIZER_DEFINE(double, true, Type_double);

#undef TYPEDESCINITIALIZER_DEFINE

}

