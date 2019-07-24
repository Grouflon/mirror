#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string.h>

namespace mirror
{
	enum MemberType
	{
		MemberType_int8,
		MemberType_int16,
		MemberType_int32,
		MemberType_int64,

		MemberType_uint8,
		MemberType_uint16,
		MemberType_uint32,
		MemberType_uint64,

		MemberType_float,
		MemberType_double,

		MemberType_string,

		MemberType_fixedCArray,
		MemberType_dynamicCArray,

	};
	template <typename T> MemberType getMemberType(T*);
	MemberType getMemberType(int8_t*);
	MemberType getMemberType(int16_t*);
	MemberType getMemberType(int32_t*);
	MemberType getMemberType(int64_t*);
	MemberType getMemberType(uint8_t*);
	MemberType getMemberType(uint16_t*);
	MemberType getMemberType(uint32_t*);
	MemberType getMemberType(uint64_t*);
	MemberType getMemberType(float*);
	MemberType getMemberType(double*);
	MemberType getMemberType(std::string*);

	struct ClassMember
	{
		ClassMember(const char* _name, size_t _offset, MemberType _type);

		const char* name;
		size_t offset;
		MemberType type;
	};


	class Class
	{
	public:
		Class(const char* _name);

		inline const std::vector<ClassMember>& getMembers() const { return m_members; }
		inline const char* getName() const { return m_name; }

		void addMember(const char* _name, size_t _address, MemberType _type);
		template <typename T> void addMember(const char* _name, size_t _address, T* _value);

	private:
		std::vector<ClassMember> m_members;
		const char* m_name;
	};


	class ClassSet
	{
	public:
		~ClassSet();

		Class* getClass(const char* _className);

		void addClass(Class* _class);
		void removeClass(Class* _class);

	private:
		std::unordered_map<uint32_t, Class*> m_classes;
	};
	extern ClassSet	g_classSet;


	uint32_t Hash32(const void* _data, size_t _size);
	uint32_t HashCString(const char* _str);
}

// INL
template <typename T> mirror::MemberType mirror::getMemberType(T*) { static_assert(false, "unsupported type"); return 0; }

template <typename T>
void mirror::Class::addMember(const char* _name, size_t _address, T* _value)
{
	addMember(_name, _address, getType(_value));
}
