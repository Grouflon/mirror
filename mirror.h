#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string.h>
#include <type_traits>
#include <typeinfo>

#define MIRROR(_class)\
public:\
	static const mirror::Class* GetClass()\
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
		const mirror::Type* type = mirror::GetType(prototypePtr->_memberName);\
		mClass->addMember(mirror::ClassMember(#_memberName, offset, type));\
	}

namespace mirror
{
	enum TypeID
	{
		TypeID_none,

		TypeID_bool,
		TypeID_char,

		TypeID_int8,
		TypeID_int16,
		TypeID_int32,
		TypeID_int64,

		TypeID_uint8,
		TypeID_uint16,
		TypeID_uint32,
		TypeID_uint64,

		TypeID_float,
		TypeID_double,

		TypeID_string,

		TypeID_fixedCArray,
		TypeID_dynamicCArray,

		TypeID_Class,

		TypeID_Pointer,
	};

	class Type
	{
	public:
		virtual TypeID getTypeID() const = 0;
	};

	class SimpleType : public Type
	{
	public:
		SimpleType(TypeID _typeID);

		virtual TypeID getTypeID() const override;

	private:
		TypeID m_typeID = TypeID_none;
	};

	class PointerType : public Type
	{
	public:
		PointerType(const Type* _subType);

		virtual TypeID getTypeID() const override { return TypeID_Pointer; }
		const Type* getSubType() const { return m_subType; }

	private:
		const Type* m_subType;
	};

	struct ClassMember
	{
		ClassMember(const char* _name, size_t _offset, const Type* _type);

		const char* name;
		size_t offset;
		const Type* type;
	};


	class Class : public Type
	{
	public:
		Class(const char* _name, size_t _typeHash);

		inline const std::vector<ClassMember>& getMembers() const { return m_members; }
		inline const char* getName() const { return m_name; }
		inline size_t getTypeHash() const { return m_typeHash; }

		void addMember(const ClassMember& _member);

		virtual TypeID getTypeID() const override { return TypeID_Class; }

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

	//template<typename T> const Type* GetType(const T*);
	template <typename T> const Type* GetType(T _v);
	const Type* GetType(const bool&);
	const Type* GetType(const char&);
	const Type* GetType(const int8_t&);
	const Type* GetType(const int16_t&);
	const Type* GetType(const int32_t&);
	const Type* GetType(const int64_t&);
	const Type* GetType(const uint8_t&);
	const Type* GetType(const uint16_t&);
	const Type* GetType(const uint32_t&);
	const Type* GetType(const uint64_t&);
	const Type* GetType(const float&);
	const Type* GetType(const double&);
	const Type* GetType(const std::string&);

	uint32_t Hash32(const void* _data, size_t _size);
	uint32_t HashCString(const char* _str);
}

// INL

/*template <typename T> const mirror::Type* mirror::GetType(const T* _v)
{
	return new PointerType(GetType(*_v));
}*/

template <typename T> const mirror::Type* mirror::GetType(T _v)
{
	return T::GetClass();
}
