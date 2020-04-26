#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string.h>
#include <type_traits>
#include <typeinfo>

#include <mirror_types.h>

#define MIRROR_CLASS(_class)\
public:\
	static const mirror::Class* GetClass()\
	{\
		mirror::Class* mClass = mirror::g_classSet.findClassByTypeHash(typeid(_class).hash_code());\
		if (!mClass)\
		{\
			mClass = new mirror::Class(#_class, typeid(_class).hash_code());\
			mirror::g_classSet.addClass(mClass);\
			char fakePrototype[sizeof(_class)];\
			_class* prototypePtr = reinterpret_cast<_class*>(fakePrototype);\
			_MIRROR_CONTENT

#define _MIRROR_CONTENT(...)\
			__VA_ARGS__\
		}\
		return mClass;\
	}

#define MIRROR_MEMBER(_memberName, ...)\
	{\
		size_t offset = reinterpret_cast<size_t>(&(prototypePtr->_memberName)) - reinterpret_cast<size_t>(prototypePtr);\
		const mirror::TypeDesc* type = mirror::GetTypeDesc(prototypePtr->_memberName);\
		mClass->addMember(mirror::ClassMember(#_memberName, offset, type));\
	}

/*#define MIRROR_MEMBER_CSTRING()
#define MIRROR_MEMBER_CFIXEDARRAY()
#define MIRROR_MEMBER_CDYNAMICARRAY()*/

namespace mirror
{
	class Class;
	class TypeDesc;

	class TypeDesc
	{
	public:
		TypeDesc(Type _type) : m_type(_type) {}

		Type getType() const { return m_type; }

	private:
		Type m_type = Type_none;
	};

	class PointerTypeDesc : public TypeDesc
	{
	public:
		PointerTypeDesc(const TypeDesc* _subType);

		const TypeDesc* getSubType() const { return m_subType; }

	private:
		const TypeDesc* m_subType;
	};

	struct ClassMember
	{
		ClassMember(const char* _name, size_t _offset, const TypeDesc* _type);

		void* getInstanceMemberPointer(void* _classInstancePointer) const;

		const char* name;
		size_t offset;
		const TypeDesc* type;
	};


	class Class : public TypeDesc
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

		Class* findClassByName(const char* _className);
		Class* findClassByTypeHash(size_t _classTypeHash);

		void addClass(Class* _class);
		void removeClass(Class* _class);

	private:
		std::unordered_map<uint32_t, Class*> m_classesByNameHash;
		std::unordered_map<size_t, Class*> m_classesByTypeHash;
	};
	extern ClassSet	g_classSet;

	template <typename T, std::enable_if_t<!std::is_enum<T>{}>* = nullptr >
	const TypeDesc* GetTypeDesc(const T& _v)
	{
		return T::GetClass();
	}
	template <typename T, std::enable_if_t<std::is_enum<T>{}>* = nullptr >
	const TypeDesc* GetTypeDesc(const T& _v)
	{
		switch (sizeof(T))
		{
		case 1: return GetTypeDesc(int8_t());
		case 2: return GetTypeDesc(int16_t());
		case 4: return GetTypeDesc(int32_t());
		case 8: return GetTypeDesc(int64_t());
		}
		return nullptr;
	}
	template <typename T> const TypeDesc* GetTypeDesc(T* _v)
	{
		return new PointerTypeDesc(GetTypeDesc(T()));
	}

	const TypeDesc* GetTypeDesc(const bool&);
	const TypeDesc* GetTypeDesc(const char&);
	const TypeDesc* GetTypeDesc(const int8_t&);
	const TypeDesc* GetTypeDesc(const int16_t&);
	const TypeDesc* GetTypeDesc(const int32_t&);
	const TypeDesc* GetTypeDesc(const int64_t&);
	const TypeDesc* GetTypeDesc(const uint8_t&);
	const TypeDesc* GetTypeDesc(const uint16_t&);
	const TypeDesc* GetTypeDesc(const uint32_t&);
	const TypeDesc* GetTypeDesc(const uint64_t&);
	const TypeDesc* GetTypeDesc(const float&);
	const TypeDesc* GetTypeDesc(const double&);

	uint32_t Hash32(const void* _data, size_t _size);
	uint32_t HashCString(const char* _str);
}
