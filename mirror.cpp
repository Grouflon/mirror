#include "mirror.h"

#include <cstring>
#include <cassert>
#include <algorithm>

namespace mirror
{
	ClassSet g_classSet;

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

	ClassMember::ClassMember(const char* _name, size_t _offset, const TypeDesc* _type)
		: name(_name)
		, offset(_offset)
		, type(_type)
	{
	}

	void* ClassMember::getInstanceMemberPointer(void* _classInstancePointer) const
	{
		return reinterpret_cast<uint8_t*>(_classInstancePointer) + offset;
	}

	Class::Class(const char* _name, size_t _typeHash)
		: TypeDesc(Type_Class)
		, m_typeHash(_typeHash)
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

	mirror::Class* ClassSet::findClassByName(const char* _className)
	{
		auto it = m_classesByNameHash.find(HashCString(_className));
		return it != m_classesByNameHash.end() ? it->second : nullptr;
	}

	Class* ClassSet::findClassByTypeHash(size_t _classTypeHash)
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
		assert(m_classes.find(_class) == m_classes.end());

		m_classesByNameHash.insert(std::make_pair(nameHash, _class));
		m_classesByTypeHash.insert(std::make_pair(_class->getTypeHash(), _class));
		m_classes.emplace(_class);
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

		auto it3 = m_classes.find(_class);
		assert(it3 != m_classes.end());
		m_classes.erase(it3);
	}

	const std::set<Class*>& ClassSet::GetClasses() const
	{
		return m_classes;
	}

	PointerTypeDesc::PointerTypeDesc(const TypeDesc* _subType)
		: TypeDesc(Type_Pointer)
		, m_subType(_subType)
	{
		
	}
}

