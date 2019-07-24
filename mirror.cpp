#include "mirror.h"

#include <cstring>
#include <cassert>
#include <algorithm>

namespace mirror
{
	ClassSet	g_classSet;

	mirror::MemberType getMemberType(int8_t*) { return MemberType_int8; }
	mirror::MemberType getMemberType(int16_t*) { return MemberType_int16; }
	mirror::MemberType getMemberType(int32_t*) { return MemberType_int32; }
	mirror::MemberType getMemberType(int64_t*) { return MemberType_int64; }
	mirror::MemberType getMemberType(uint8_t*) { return MemberType_uint8; }
	mirror::MemberType getMemberType(uint16_t*) { return MemberType_uint16; }
	mirror::MemberType getMemberType(uint32_t*) { return MemberType_uint32; }
	mirror::MemberType getMemberType(uint64_t*) { return MemberType_uint64; }
	mirror::MemberType getMemberType(float*) { return MemberType_float; }
	mirror::MemberType getMemberType(double*) { return MemberType_double; }
	mirror::MemberType getMemberType(std::string*) { return MemberType_string; }

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

	ClassMember::ClassMember(const char* _name, size_t _offset, MemberType _type)
		: name(_name)
		, offset(_offset)
		, type(_type)
	{

	}

	Class::Class(const char* _name)
		: m_name(_name)
	{

	}

	void Class::addMember(const char* _name, size_t _address, MemberType _type)
	{
		// Checks if a member with the same name does not already exists
		assert(std::find_if(m_members.begin(), m_members.end(), [_name](const ClassMember& _m) { return strcmp(_name, _m.name) == 0; }) == m_members.end());

		m_members.push_back(ClassMember(_name, _address, _type));
	}

	ClassSet::~ClassSet()
	{
		for (auto& pair : m_classes)
		{
			delete pair.second;
		}
		m_classes.clear();
	}

	mirror::Class* ClassSet::getClass(const char* _className)
	{
		auto it = m_classes.find(HashCString(_className));
		return it != m_classes.end() ? it->second : nullptr;
	}

	void ClassSet::addClass(Class* _class)
	{
		// Checks if a class with the same name does not already exists
		assert(_class);
		uint32_t nameHash = HashCString(_class->getName());
		assert(m_classes.find(nameHash) == m_classes.end());

		m_classes.insert(std::make_pair(nameHash, _class));
	}

	void ClassSet::removeClass(Class* _class)
	{
		// Checks if a class with the same name not already exists
		assert(_class);
		uint32_t nameHash = HashCString(_class->getName());
		auto it = m_classes.find(nameHash);
		assert(it != m_classes.end());

		m_classes.erase(it);
	}
}
