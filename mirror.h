#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string.h>
#include <type_traits>
#include <typeinfo>

#define MIRROR(_class)\
public:\
	static mirror::Class* GetClass()\
	{\
		mirror::Class* mClass = mirror::g_classSet.getClassByTypeHash(typeid(_class).hash_code());\
		if (!mClass)\
		{\
			mClass = new mirror::Class(#_class, typeid(_class).hash_code());\
			mirror::g_classSet.addClass(mClass);\
			char fakePrototype[sizeof(_class)];\
			_class* prototypePtr = (_class*) fakePrototype;\
			_MIRROR_CONTENT

#define _MIRROR_CONTENT(...)\
			__VA_ARGS__\
		}\
		return mClass;\
	}

#define M_MEMBER(_memberName)\
	{\
		size_t offset = reinterpret_cast<size_t>(&(prototypePtr->_memberName)) - reinterpret_cast<size_t>(&prototypePtr);\
		mirror::MemberType type = mirror::GetMemberType(prototypePtr->_memberName);\
		if (type == mirror::MemberType_ClassInstance)\
		{\
			mClass->addMember(mirror::ClassMember(#_memberName, offset, type, mirror::GetClass(prototypePtr->_memberName)));\
		}\
		else\
		{\
			mClass->addMember(mirror::ClassMember(#_memberName, offset, type));\
		}\
	}

namespace mirror
{
	class Class;

	enum MemberType
	{
		MemberType_none,

		MemberType_bool,
		MemberType_char,

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

		MemberType_ClassInstance,
	};
	template <typename T> MemberType GetMemberType(T&);
	MemberType GetMemberType(bool&);
	MemberType GetMemberType(char&);
	MemberType GetMemberType(int8_t&);
	MemberType GetMemberType(int16_t&);
	MemberType GetMemberType(int32_t&);
	MemberType GetMemberType(int64_t&);
	MemberType GetMemberType(uint8_t&);
	MemberType GetMemberType(uint16_t&);
	MemberType GetMemberType(uint32_t&);
	MemberType GetMemberType(uint64_t&);
	MemberType GetMemberType(float&);
	MemberType GetMemberType(double&);
	MemberType GetMemberType(std::string&);

	template<typename T> Class* GetClass(T&);
	Class* GetClass(bool&);
	Class* GetClass(char&);
	Class* GetClass(int8_t&);
	Class* GetClass(int16_t&);
	Class* GetClass(int32_t&);
	Class* GetClass(int64_t&);
	Class* GetClass(uint8_t&);
	Class* GetClass(uint16_t&);
	Class* GetClass(uint32_t&);
	Class* GetClass(uint64_t&);
	Class* GetClass(float&);
	Class* GetClass(double&);
	Class* GetClass(std::string&);

	struct ClassMember
	{
		ClassMember(const char* _name, size_t _offset, MemberType _type);
		ClassMember(const char* _name, size_t _offset, MemberType _type, Class* _subClass);

		const char* name;
		size_t offset;
		MemberType type;
		Class* subClass = nullptr;
	};


	class Class
	{
	public:
		Class(const char* _name, size_t _typeHash);

		inline const std::vector<ClassMember>& getMembers() const { return m_members; }
		inline const char* getName() const { return m_name; }
		inline size_t getTypeHash() const { return m_typeHash; }

		void addMember(const ClassMember& _member);

	private:
		std::vector<ClassMember> m_members;
		size_t m_typeHash;
		const char* m_name;
	};


	class ClassSet
	{
	public:
		~ClassSet();

		Class* getClassByName(const char* _className);
		Class* getClassByTypeHash(size_t _classTypeHash);

		void addClass(Class* _class);
		void removeClass(Class* _class);

	private:
		std::unordered_map<uint32_t, Class*> m_classesByNameHash;
		std::unordered_map<size_t, Class*> m_classesByTypeHash;
	};
	extern ClassSet	g_classSet;


	uint32_t Hash32(const void* _data, size_t _size);
	uint32_t HashCString(const char* _str);
}

// INL
template <typename T> mirror::MemberType mirror::GetMemberType(T&)
{
	if (T::GetClass())
	{
		return MemberType_ClassInstance;
	}
	return MemberType_none;
}

template <typename T> mirror::Class* mirror::GetClass(T&)
{
	return T::GetClass();
}

