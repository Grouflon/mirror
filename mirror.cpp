#include "mirror.h"

#include <cstring>
#include <cassert>
#include <algorithm>

namespace mirror
{
	ClassSet	g_classSet;

	const mirror::Type* GetType(const bool&) { static SimpleType s_type(TypeID_bool); return &s_type; }
	const mirror::Type* GetType(const char&) { static SimpleType s_type(TypeID_char); return &s_type; }
	const mirror::Type* GetType(const int8_t&) { static SimpleType s_type(TypeID_int8); return &s_type; }
	const mirror::Type* GetType(const int16_t&) { static SimpleType s_type(TypeID_int16); return &s_type; }
	const mirror::Type* GetType(const int32_t&) { static SimpleType s_type(TypeID_int32); return &s_type; }
	const mirror::Type* GetType(const int64_t&) { static SimpleType s_type(TypeID_int64); return &s_type; }
	const mirror::Type* GetType(const uint8_t&) { static SimpleType s_type(TypeID_uint8); return &s_type; }
	const mirror::Type* GetType(const uint16_t&) { static SimpleType s_type(TypeID_uint16); return &s_type; }
	const mirror::Type* GetType(const uint32_t&) { static SimpleType s_type(TypeID_uint32); return &s_type; }
	const mirror::Type* GetType(const uint64_t&) { static SimpleType s_type(TypeID_uint64); return &s_type; }
	const mirror::Type* GetType(const float&) { static SimpleType s_type(TypeID_float); return &s_type; }
	const mirror::Type* GetType(const double&) { static SimpleType s_type(TypeID_double); return &s_type; }

	const mirror::Type* GetType(const std::string&) { static SimpleType s_type(TypeID_string); return &s_type; }

#define OFFSET_BASIS	2166136261
#define FNV_PRIME		16777619

	uint32_t mirror::Hash32(const void* _data, size_t _size)
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

	uint32_t mirror::HashCString(const char* _str)
	{
		return Hash32(_str, strlen(_str));
	}

	ClassMember::ClassMember(const char* _name, size_t _offset, const Type* _type)
		: name(_name)
		, offset(_offset)
		, type(_type)
	{

	}

	Class::Class(const char* _name, size_t _typeHash)
		: m_typeHash(_typeHash)
		, m_name(_name)
	{

	}

	void Class::addMember(const ClassMember& _member)
	{
		// Checks if a member with the same name does not already exists
		assert(std::find_if(m_members.begin(), m_members.end(), [_member](const ClassMember& _m) { return strcmp(_member.name, _m.name) == 0; }) == m_members.end());

		m_members.push_back(_member);
	}

	ClassSet::~ClassSet()
	{
		for (auto& pair : m_classesByNameHash)
		{
			delete pair.second;
		}
		m_classesByNameHash.clear();
		m_classesByTypeHash.clear();
	}

	mirror::Class* ClassSet::getClassByName(const char* _className)
	{
		auto it = m_classesByNameHash.find(HashCString(_className));
		return it != m_classesByNameHash.end() ? it->second : nullptr;
	}

	Class* ClassSet::getClassByTypeHash(size_t _classTypeHash)
	{
		auto it = m_classesByTypeHash.find(_classTypeHash);
		return it != m_classesByTypeHash.end() ? it->second : nullptr;
	}

	void ClassSet::addClass(Class* _class)
	{
		// Checks if a class with the same name/typehash does not already exists
		assert(_class);
		uint32_t nameHash = HashCString(_class->getName());
		assert(m_classesByNameHash.find(nameHash) == m_classesByNameHash.end());
		assert(m_classesByTypeHash.find(_class->getTypeHash()) == m_classesByTypeHash.end());

		m_classesByNameHash.insert(std::make_pair(nameHash, _class));
		m_classesByTypeHash.insert(std::make_pair(_class->getTypeHash(), _class));
	}

	void ClassSet::removeClass(Class* _class)
	{
		assert(_class);

		// Checks if a class with the same name/typehash already exists
		auto it = m_classesByNameHash.find(HashCString(_class->getName()));
		assert(it != m_classesByNameHash.end());
		m_classesByNameHash.erase(it);

		auto it2 = m_classesByTypeHash.find(_class->getTypeHash());
		assert(it2 != m_classesByTypeHash.end());
		m_classesByTypeHash.erase(it2);
	}

	SimpleType::SimpleType(TypeID _typeID)
		: m_typeID(_typeID)
	{
	}

	mirror::TypeID SimpleType::getTypeID() const
	{
		return m_typeID;
	}

	PointerType::PointerType(const Type* _subType)
		: m_subType(_subType)
	{
		
	}

}
