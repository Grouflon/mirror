#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <set>
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
			char fakePrototype[sizeof(_class)] = {};\
			_class* prototypePtr = reinterpret_cast<_class*>(fakePrototype);\
			_MIRROR_CONTENT

#define _MIRROR_CONTENT(...)\
			__VA_ARGS__\
		}\
		return mClass;\
	}

#define MIRROR_MEMBER(_memberName)\
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

		// NOTE(Remi|2020/04/26): virtual specifier is not needed, but is added to allow the debugger to show inherited types
		virtual Type getType() const { return m_type; }

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

		const std::vector<ClassMember>& getMembers() const { return m_members; }
		const char* getName() const { return m_name; }
		size_t getTypeHash() const { return m_typeHash; }

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

		const std::set<Class*>& GetClasses() const;

	private:
		std::set<Class*> m_classes;
		std::unordered_map<uint32_t, Class*> m_classesByNameHash;
		std::unordered_map<size_t, Class*> m_classesByTypeHash;
	};

	template <typename T, typename IsPointer = void, typename IsEnum = void>
	struct TypeDescGetter
	{
		static const TypeDesc* Get()
		{
			return T::GetClass();
		}
	};

	template <typename T>
	struct TypeDescGetter<T, std::enable_if_t<std::is_pointer<T>::value>>
	{
		static const TypeDesc* Get()
		{
			static PointerTypeDesc s_pointerTypeDesc(TypeDescGetter<std::remove_pointer<T>::type>::Get()); return &s_pointerTypeDesc;
		}
	};

	template <typename T>
	struct TypeDescGetter<T, void, std::enable_if_t<std::is_enum<T>::value>>
	{
		static const TypeDesc* Get()
		{
			switch (sizeof(T))
			{
			case 1: return TypeDescGetter<int8_t>::Get();
			case 2: return TypeDescGetter<int16_t>::Get();
			case 4: return TypeDescGetter<int32_t>::Get();
			case 8: return TypeDescGetter<int64_t>::Get();
			}
			return nullptr;
		}
	};

	template <> struct TypeDescGetter<void> { static const TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_void); return &s_typeDesc; } };
	template <> struct TypeDescGetter<bool> { static const TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_bool); return &s_typeDesc; } };
	template <> struct TypeDescGetter<char> { static const TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_char); return &s_typeDesc; } };
	template <> struct TypeDescGetter<int8_t> { static const TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_int8); return &s_typeDesc; } };
	template <> struct TypeDescGetter<int16_t> { static const TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_int16); return &s_typeDesc; } };
	template <> struct TypeDescGetter<int32_t> { static const TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_int32); return &s_typeDesc; } };
	template <> struct TypeDescGetter<int64_t> { static const TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_int64); return &s_typeDesc; } };
	template <> struct TypeDescGetter<uint8_t> { static const TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_uint8); return &s_typeDesc; } };
	template <> struct TypeDescGetter<uint16_t> { static const TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_uint16); return &s_typeDesc; } };
	template <> struct TypeDescGetter<uint32_t> { static const TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_uint32); return &s_typeDesc; } };
	template <> struct TypeDescGetter<uint64_t> { static const TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_uint64); return &s_typeDesc; } };
	template <> struct TypeDescGetter<float> { static const TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_float); return &s_typeDesc; } };
	template <> struct TypeDescGetter<double> { static const TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_double); return &s_typeDesc; } };

	template <typename T>
	const TypeDesc* GetTypeDesc(T) { return TypeDescGetter<T>::Get(); }

	uint32_t Hash32(const void* _data, size_t _size);
	uint32_t HashCString(const char* _str);

	extern ClassSet	g_classSet;
}
