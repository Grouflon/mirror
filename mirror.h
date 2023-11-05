#pragma once

//*****************************************************************************
// mirror - lightweight single header C++ reflection library
//*****************************************************************************

//-----------------------------------------------------------------------------
// Licence
//-----------------------------------------------------------------------------
/*
The MIT License (MIT)
Copyright © 2023 Rémi Bismuth

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the “Software”), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of 
the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.
The Software is provided “as is”, without warranty of any kind, express or implied, 
including but not limited to the warranties of merchantability, fitness for a 
particular purpose and noninfringement. In no event shall the authors or copyright 
holders be liable for any claim, damages or other liability, whether in an action 
of contract, tort or otherwise, arising from, out of or in connection with the 
software or the use or other dealings in the Software.
*/

//*****************************************************************************
// Header
//*****************************************************************************

//-----------------------------------------------------------------------------
// Header and includes
//-----------------------------------------------------------------------------

// Dll import/export markup
#ifndef MIRROR_API
#define MIRROR_API
#endif

#include <cstdint>
#include <vector>
#include <set>
#include <unordered_map>
#include <type_traits>
#include <typeinfo>
#include <assert.h>

//-----------------------------------------------------------------------------
// Mirror
//-----------------------------------------------------------------------------

namespace mirror {

	//-----------------------------------------------------------------------------
	// Forward Declarations & Types
	//-----------------------------------------------------------------------------
	typedef size_t TypeID;
	class TypeSet;
	class Type;
	class Class;
	class Enum;
	class Pointer;
	class StaticFunction;
	class FixedSizeArray;
	class ClassMember;
	class EnumValue;
	class VirtualTypeWrapper;
	struct MetaData;
	struct MetaDataSet;

	typedef void* (*AllocateFunction)(size_t _size, void* _userData);

	const TypeID UNDEFINED_TYPEID = 0;

	//-----------------------------------------------------------------------------
	// Enums
	//-----------------------------------------------------------------------------
	enum TypeInfo
	{
		TypeInfo_none = 0,

		TypeInfo_void,
		TypeInfo_bool,
		TypeInfo_char,
		TypeInfo_int8,
		TypeInfo_int16,
		TypeInfo_int32,
		TypeInfo_int64,
		TypeInfo_uint8,
		TypeInfo_uint16,
		TypeInfo_uint32,
		TypeInfo_uint64,
		TypeInfo_float,
		TypeInfo_double,
		TypeInfo_Enum,
		TypeInfo_Class,
		TypeInfo_Pointer,
		TypeInfo_FixedSizeArray,
		TypeInfo_StaticFunction,
		TypeInfo_Custom,

		TypeInfo_COUNT,
	};

	//-----------------------------------------------------------------------------
	// Helpers & Tools
	//-----------------------------------------------------------------------------
	#define _MIRROR_CAT(a, b) a##b
	#define MIRROR_CAT(a, b) _MIRROR_CAT(a, b)

	MIRROR_API uint32_t Hash32(const void* _data, size_t _size);
	MIRROR_API uint32_t HashCString(const char* _str);
	MIRROR_API const char* TypeInfoToString(TypeInfo _type);

	//-----------------------------------------------------------------------------
	// Global Functions
	//-----------------------------------------------------------------------------
	MIRROR_API void InitNewTypes();

	template <typename T> constexpr TypeID GetTypeID();
	template <typename T> constexpr TypeID GetTypeID(T&);

	template <typename T> Type* GetType();
	template <typename T> Type* GetType(T&);
	MIRROR_API Type* GetType(TypeID _id);

    template<typename T> Enum* GetEnum();
    template<typename T> Enum* GetEnum(T&);

	template <typename T> Class* GetClass();
	template <typename T> Class* GetClass(T&);

	template <typename DestType, typename SourceType> DestType Cast(SourceType _o);

	MIRROR_API Type* FindTypeByName(const char* _name);
	MIRROR_API Class* FindClassByName(const char* _name);
	MIRROR_API Type* FindTypeByID(TypeID _id);

	MIRROR_API Class* AsClass(TypeID _id);
	MIRROR_API Enum* AsEnum(TypeID _id);
	MIRROR_API Pointer* AsPointer(TypeID _id);
	MIRROR_API StaticFunction* AsStaticFunction(TypeID _id);
	MIRROR_API FixedSizeArray* AsFixedSizeArray(TypeID _id);

    MIRROR_API TypeSet& GetTypeSet();

	//-----------------------------------------------------------------------------
	// Meta Data
	//-----------------------------------------------------------------------------

	struct MIRROR_API MetaData
	{
		const char* getName() const;
		bool asBool() const;
		int asInt() const;
		float asFloat() const;
		const char* asString() const;

	// internal
		MetaData(const char* _name, const char* _data);
		MetaData(const MetaData& _other);
		~MetaData();

		char* m_name;
		char* m_data;
	};

	struct MIRROR_API MetaDataSet
	{
		const MetaData* findMetaData(const char* _key) const;

	// internal
		MetaDataSet(const char* _metaDataString);
		MetaDataSet(const MetaDataSet& _other);
		MetaDataSet& operator=(const MetaDataSet& _other) = default;
		std::unordered_map<uint32_t, MetaData> m_metaData;
	};

	//-----------------------------------------------------------------------------
	// TypeSet
	//-----------------------------------------------------------------------------

	class MIRROR_API TypeSet
	{
	public:
		~TypeSet();

		Type* findTypeByID(TypeID _typeID) const;
		Type* findTypeByName(const char* _name) const;

		void addType(Type* _type);
		void addTypeName(Type* _type, const char* _name);
		void removeType(Type* _type);

		const std::set<Type*>& getTypes() const;

		void initNewTypes();

	//private:
		std::set<Type*> m_types;
		std::unordered_map<TypeID, Type*> m_typesByID;
		std::unordered_map<TypeID, int> m_typesRegistrationCount;
		std::unordered_map<uint32_t, Type*> m_typesByName;
	};

	extern TypeSet* g_typeSetPtr;

	//-----------------------------------------------------------------------------
	// Type
	//-----------------------------------------------------------------------------

	class MIRROR_API Type
	{
	public:
		TypeInfo getTypeInfo() const;
		const char* getName() const;
		const char* getCustomTypeName() const;
		bool isCustomType(const char* _customTypeName) const;
		TypeID getTypeID() const;
		size_t getSize() const;

		const Class* asClass() const;
		const Enum* asEnum() const;
		const Pointer* asPointer() const;
		const StaticFunction* asStaticFunction() const;
		const FixedSizeArray* asFixedSizeArray() const;

		Class* asClass();
		Enum* asEnum();
		Pointer* asPointer();
		StaticFunction* asStaticFunction();
		FixedSizeArray* asFixedSizeArray();

		// @TODO(2021/02/15|Remi): Allow the user to choose their allocator
		bool hasFactory() const;
		void* instantiate(AllocateFunction _allocateFunction = nullptr, void* _userData = nullptr) const;

	// internal
		void setName(const char* _name);
		void setCustomTypeName(const char* _name);

		virtual void shutdown();
		virtual void init();

		Type(TypeInfo _typeInfo);
		Type(TypeInfo _typeInfo, const char* _name);
		virtual ~Type();
		template <typename T> void createVirtualTypeWrapper();

		char* m_name = nullptr;
		char* m_customTypeName = nullptr;
		TypeInfo m_typeInfo = TypeInfo_none;
		VirtualTypeWrapper* m_virtualTypeWrapper = nullptr;
		bool m_initialized = false;
	};


	// @NOTE(remi): Need to rewrite this a bit after
	class MIRROR_API VirtualTypeWrapper
	{
	public:
		TypeID getTypeID() const { return m_typeID; }
		size_t getSize() const { return m_size; }

		virtual bool hasFactory() const { return false; }
		virtual void* instantiate(AllocateFunction _allocateFunction = nullptr, void* _userData = nullptr) const { return nullptr; }

		virtual Class* unsafeVirtualGetClass(void* _object) const { return nullptr; }

		virtual ~VirtualTypeWrapper() {}
		TypeID m_typeID = UNDEFINED_TYPEID;
		size_t m_size = 0;
	};

	//-----------------------------------------------------------------------------
	// Specialized Type Descs
	//-----------------------------------------------------------------------------

	// --- Class
	class MIRROR_API Class : public Type
	{
	public:
		size_t getMembersCount(bool _includeInheritedMembers = true) const;
		std::vector<ClassMember*> getMembers(bool _includeInheritedMembers = true) const;
		size_t getMembers(ClassMember** _outMemberList, size_t _memberListSize, bool _includeInheritedMembers = true) const;
		void getMembers(std::vector<ClassMember*>& _outMemberList, bool _includeInheritedMembers = true) const;
		ClassMember* findMemberByName(const char* _name, bool _includeInheritedMembers = true) const;

		Class* getParent() const;
		TypeID getParentID() const;
		const std::set<TypeID>& getParents() const;
		const std::set<TypeID>& getChildren() const;

		bool isChildOf(const Class* _class, bool _checkSelf = true) const;

		const MetaDataSet& getMetaDataSet() const;

		Class* unsafeVirtualGetClass(void* _object) const;

	// internal
		void addMember(ClassMember* _member);
		void addParent(TypeID _parent);
		Class(const char* _name, const char* _metaDataString);
		Class(const char* _name, const MetaDataSet& _metaDataSet);

		virtual void shutdown() override;
		virtual void init() override;

		virtual ~Class();

		std::set<TypeID> m_parents;
		std::set<TypeID> m_children;
		std::vector<ClassMember*> m_members;
		std::unordered_map<uint32_t, ClassMember*> m_membersByName;
		MetaDataSet m_metaDataSet;
	};

	class MIRROR_API ClassMember
	{
	public:
		const char* getName() const;
		Class* getOwnerClass() const;
		size_t getOffset() const;
		Type* getType() const;

		void* getInstanceMemberPointer(void* _classInstancePointer) const;
		const MetaDataSet& getMetaDataSet() const;

	// internal
		ClassMember(const char* _name, size_t _offset, TypeID _type, const char* _metaDataString);
		~ClassMember();

		Class* m_ownerClass = nullptr;
		char* m_name;
		size_t m_offset;
		TypeID m_typeInfo = UNDEFINED_TYPEID;
		MetaDataSet m_metaDataSet;
	};

	struct ClassInitializerBase {};
	template <typename T> struct ClassInitializer : public ClassInitializerBase {};

	// --- Enum
	class MIRROR_API Enum : public Type
	{
	public:

		template <typename T> bool getValueFromString(const char* _string, T& _outValue) const;
		template <typename T> bool getStringFromValue(T _value, const char*& _outString) const;

		const std::vector<EnumValue*>& getValues() const;
		Type* getSubType() const;

	// internal
		void addValue(EnumValue* _value);
		Enum(const char* _name, TypeID _subType = UNDEFINED_TYPEID);

		std::vector<EnumValue*> m_values;
		std::unordered_map<size_t, EnumValue*> m_valuesByNameHash;

		TypeID m_subType;
	};

	class MIRROR_API EnumValue
	{
	public:		
		const char* getName() const;
		int64_t getValue() const;

	// internal
		EnumValue(const char* _name, int64_t _value);
		~EnumValue();

		char* m_name;
		int64_t m_value;
	};

	// --- Pointer
	class MIRROR_API Pointer : public Type
	{
	public:
		Type* getSubType() const;

	// internal
		Pointer(TypeID _subType);
		virtual void init() override;

		TypeID m_subType = UNDEFINED_TYPEID;
	};

	// --- Fixed Size Array
	class MIRROR_API FixedSizeArray : public Type
	{
	public:
		Type* getSubType() const;
		size_t getElementCount() const { return m_elementCount; }
		void* getDataAt(void* _basePtr, size_t _index) const;

	// internal
		FixedSizeArray(TypeID _subType, size_t _elementCount);
		virtual void init() override;

		TypeID m_subType = UNDEFINED_TYPEID;
		size_t m_elementCount;
	};

} // namespace mirror

//*****************************************************************************
// REFLECTION MACROS
//*****************************************************************************

// Disable some warnings when expanding macros
#if defined(__clang__)
#define MIRROR_PUSH_DISABLE_WARNINGS \
_Pragma("clang diagnostic push") \
_Pragma("clang diagnostic ignored \"-Winconsistent-missing-override\"")
#define MIRROR_POP_DISABLE_WARNINGS \
_Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#define MIRROR_PUSH_DISABLE_WARNINGS \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Winconsistent-missing-override\"")
#define MIRROR_POP_DISABLE_WARNINGS \
_Pragma("GCC diagnostic pop")
#else
#define MIRROR_PUSH_DISABLE_WARNINGS
#define MIRROR_POP_DISABLE_WARNINGS
#endif

//-----------------------------------------------------------------------------
// Public Macros
//-----------------------------------------------------------------------------

#define MIRROR_CLASS(_class, ...) \
namespace mirror { \
template <> \
struct ClassInitializer<_class> : public ClassInitializerBase \
{ \
	ClassInitializer(); \
	~ClassInitializer() \
	{ \
		GetTypeSet().removeType(clss); \
		delete clss; \
		clss = nullptr; \
	} \
	Class* clss = nullptr;\
}; \
ClassInitializer<_class> MIRROR_CAT(initializer_,__LINE__); \
\
ClassInitializer<_class>::ClassInitializer() \
{ \
	const char* metaDataString = #__VA_ARGS__""; \
	mirror::MetaDataSet metaDataSet(metaDataString); \
	clss = new ::mirror::Class(#_class, metaDataSet); \
	clss->createVirtualTypeWrapper<_class>(); \
	char fakePrototype[sizeof(_class)] = {}; \
	_class* prototypePtr = reinterpret_cast<_class*>(fakePrototype); \
	(void)prototypePtr; \
	{ \
		__MIRROR_CLASS_CONTENT

#define __MIRROR_CLASS_CONTENT(...) \
		__VA_ARGS__ \
	} \
	GetTypeSet().addType(clss); \
} \
} \

#define MIRROR_GETCLASS_VIRTUAL() \
MIRROR_PUSH_DISABLE_WARNINGS \
public: \
	virtual ::mirror::Class* getClass() const { return ::mirror::GetClass(*this); } \
MIRROR_POP_DISABLE_WARNINGS \

#define MIRROR_GETCLASS() \
public: \
	::mirror::Class* getClass() const { return ::mirror::GetClass(*this); } \

#define MIRROR_FRIEND() \
	friend struct ::mirror::ClassInitializerBase; \

#define MIRROR_MEMBER(_memberName, ...) \
	{ \
		::mirror::GetType(prototypePtr->_memberName); \
		size_t offset = reinterpret_cast<size_t>(&(prototypePtr->_memberName)) - reinterpret_cast<size_t>(prototypePtr); \
		const char* memberName = #_memberName; \
		const char* metaDataString = #__VA_ARGS__""; \
		::mirror::ClassMember* classMember = new ::mirror::ClassMember(memberName, offset, ::mirror::GetTypeID(prototypePtr->_memberName), metaDataString); \
		clss->addMember(classMember); \
	} \

#define MIRROR_PARENT(_parentClass) \
	{ \
		clss->addParent(::mirror::GetTypeID<_parentClass>()); \
	} \

#define MIRROR_ENUM(_enumName) \
	namespace MIRROR_CAT(__Mirror, _enumName) { \
	struct Initializer \
	{\
		mirror::Enum* enm = nullptr; \
		~Initializer() \
		{ \
			::mirror::GetTypeSet().removeType(enm); \
			delete enm; \
		} \
		Initializer() \
		{ \
			using enumType = _enumName; \
			::mirror::Type* subType; \
			switch(sizeof(enumType)) { \
				case 1: subType = ::mirror::TypeGetter<int8_t>::Get(); break; \
				case 2: subType = ::mirror::TypeGetter<int16_t>::Get(); break; \
				case 4: subType = ::mirror::TypeGetter<int32_t>::Get(); break; \
				case 8: subType = ::mirror::TypeGetter<int64_t>::Get(); break; \
			} \
			enm = new ::mirror::Enum(#_enumName, subType->getTypeID()); \
			enm->createVirtualTypeWrapper<enumType>(); \
			__MIRROR_ENUM_CONTENT

#define __MIRROR_ENUM_CONTENT(...) \
			__VA_ARGS__ \
			mirror::GetTypeSet().addType(enm); \
		}\
	} initializer; \
	} // namespace __Mirror##_enumName

#define MIRROR_ENUM_VALUE(_enumValue) \
		enm->addValue(new ::mirror::EnumValue(#_enumValue, int64_t(_enumValue))); \

//*****************************************************************************
// Inline Implementations
//*****************************************************************************

#include <typeinfo>

namespace mirror {

	//-----------------------------------------------------------------------------
	// GetTypeID
	//-----------------------------------------------------------------------------
	template <typename T>
	constexpr TypeID GetTypeID()
	{
		return typeid(T).hash_code();
	}

	template <typename T>
	constexpr TypeID GetTypeID(T&)
	{
		return GetTypeID<T>();
	}

	//-----------------------------------------------------------------------------
	// Type
	//-----------------------------------------------------------------------------

	template <typename T>
	void Type::createVirtualTypeWrapper()
	{
		if (m_virtualTypeWrapper == nullptr)
		{
			m_virtualTypeWrapper = new TVirtualTypeWrapper<typename std::remove_const<T>::type>();
		}
	}

	//-----------------------------------------------------------------------------
	// Virtual Type Wrapper
	//-----------------------------------------------------------------------------
	template <typename T, typename IsShallow = void>
	class TVirtualTypeWrapper : public VirtualTypeWrapper
	{
	public:
		TVirtualTypeWrapper()
		{
			m_typeID = GetTypeID<T>();
			m_size = sizeof(T);
		}

		virtual bool hasFactory() const override { return true; }
		virtual void* instantiate(AllocateFunction _allocateFunction = nullptr, void* _userData = nullptr) const override
		{
			if (_allocateFunction == nullptr)
			{
				return new T();
			}
			else
			{
				T* memory = (T*)_allocateFunction(sizeof(T), _userData);
				new (memory) T();
				return memory;
			}
		}
	};

	template <typename T>
	class TVirtualTypeWrapper<T, std::enable_if_t<std::is_function<T>::value || std::is_void<T>::value>> : public VirtualTypeWrapper
	{
	public:
		TVirtualTypeWrapper()
		{
			m_typeID = GetTypeID<T>();
			m_size = 0;
		}

		virtual bool hasFactory() const override { return false; }
		virtual void* instantiate(AllocateFunction _allocateFunction = nullptr, void* _userData = nullptr) const override { return nullptr; }
	};

	//-----------------------------------------------------------------------------
	// Global Functions
	//-----------------------------------------------------------------------------

	// --- TypeGetter
	template <typename T>
	struct CustomTypeFactory
	{
		static Type* Create()
		{
			return nullptr;
		}
	};

	template <typename T, typename IsArray = void, typename IsPointer = void, typename IsEnum = void, typename IsFunction = void>
	struct TypeGetter
	{
		static Type* Get()
		{
			TypeID typeID = GetTypeID<T>();
			Type* typeDesc = GetTypeSet().findTypeByID(typeID);
			if (typeDesc == nullptr)
			{
				typeDesc = CustomTypeFactory<T>::Create();
				if (typeDesc != nullptr)
				{
					typeDesc->createVirtualTypeWrapper<T>();
					GetTypeSet().addType(typeDesc);
				}
			}
			return typeDesc;
		}
	};

	template <typename T>
    struct FixedSizeArrayInitializer
    {
        FixedSizeArrayInitializer()
        {
			using type = typename std::remove_extent<T>::type;
			typeDesc = new FixedSizeArray(GetTypeID<type>(), std::extent<T>::value);
			typeDesc->createVirtualTypeWrapper<T>();
			GetTypeSet().addType(typeDesc);
        }
        ~FixedSizeArrayInitializer()
        {
            GetTypeSet().removeType(typeDesc);
            delete typeDesc;
        }
        FixedSizeArray* typeDesc = nullptr;
    };

	template <typename T>
	struct TypeGetter<T, std::enable_if_t<std::is_array<T>::value>>
	{
		static Type* Get()
		{
			static FixedSizeArrayInitializer<T> s_FixedSizeArrayInitializer;
			return GetTypeSet().findTypeByID(GetTypeID<T>());
		}
	};

    template <typename T>
    struct PointerInitializer
    {
        PointerInitializer()
        {
            using type = typename std::remove_pointer<T>::type;
            typeDesc = new Pointer(TypeGetter<type>::Get()->getTypeID());
			typeDesc->createVirtualTypeWrapper<T>();
            GetTypeSet().addType(typeDesc);
        }
        ~PointerInitializer()
        {
            GetTypeSet().removeType(typeDesc);
            delete typeDesc;
        }
        Pointer* typeDesc = nullptr;
    };

	template <typename T>
	struct TypeGetter<T, void, std::enable_if_t<std::is_pointer<T>::value>>
	{
		static Type* Get()
		{
			static PointerInitializer<T> s_PointerInitializer;
			return GetTypeSet().findTypeByID(GetTypeID<T>());
		}
	};

	template <typename T>
	struct TypeGetter<T, void, void, std::enable_if_t<std::is_enum<T>::value>>
	{
		static Type* Get()
		{
			TypeID typeID = GetTypeID<T>();
			Type* type = GetTypeSet().findTypeByID(typeID);
			if (type != nullptr)
				return type;

			switch (sizeof(T))
			{
			case 1: return TypeGetter<int8_t>::Get();
			case 2: return TypeGetter<int16_t>::Get();
			case 4: return TypeGetter<int32_t>::Get();
			case 8: return TypeGetter<int64_t>::Get();
			}
			return nullptr;
		}
	};

	// Type Desc Accesors
	template<typename T>
    Enum* GetEnum()
    {
        Type* type = TypeGetter<T>::Get();
        if (type->getTypeInfo() != TypeInfo_Enum)
            return nullptr;

        return static_cast<Enum*>(type);
    }

    template<typename T>
    Enum* GetEnum(T&)
    {
        return GetEnum<T>();
    }

	template <typename T>
	Type* GetType()
	{
		return TypeGetter<T>::Get();
	}

	template <typename T>
	Type* GetType(T&)
	{
		return GetType<T>();
	}

	template <typename T>
	Class* GetClass()
	{ 
		Type* typeDesc = TypeGetter<T>::Get();
		if (typeDesc != nullptr && typeDesc->getTypeInfo() == TypeInfo_Class)
		{
			return static_cast<Class*>(typeDesc);
		}
		return nullptr;
	}

	template <typename T>
	Class* GetClass(T&)
	{ 
		return GetClass<T>();
	}

	// --- Cast
	template <typename DestType, typename SourceType, typename IsDestLastPointer = void, typename IsSourceLastPointer = void>
	struct CastClassesUnpiler
	{
		static_assert(std::is_pointer<DestType>::value, "Mismatching pointer count between cast source and cast destination (DestType is not a pointer).");
		static_assert(std::is_pointer<SourceType>::value, "Mismatching pointer count between cast source and cast destination (SourceType is not a pointer).");

		static bool Unpile(SourceType& _o, Class** _destClass, Class** _sourceClass)
		{
			using source_t = typename std::remove_pointer<SourceType>::type;
			using dest_t   = typename std::remove_pointer<DestType>::type;
			return CastClassesUnpiler<dest_t, source_t>::Unpile(*_o, _destClass, _sourceClass);
		}
	};

	template <typename DestType, typename SourceType>
	struct CastClassesUnpiler<DestType, SourceType, std::enable_if_t<!std::is_pointer<DestType>::value>, std::enable_if_t<!std::is_pointer<SourceType>::value>>
	{
		static bool Unpile(SourceType& _o, Class** _destClass, Class** _sourceClass)
		{
			*_destClass = GetClass<DestType>();
			*_sourceClass = _o.getClass();

			return true;
		}
	};

	template <typename DestType, typename SourceType>
	DestType Cast(SourceType _o)
	{
		Class* destClass = nullptr;
		Class* sourceClass = nullptr;
		if (CastClassesUnpiler<DestType, SourceType>::Unpile(_o, &destClass, &sourceClass))
		{
			if (destClass != nullptr && sourceClass != nullptr)
			{
				if (destClass->isChildOf(sourceClass) // upcast
				|| sourceClass->isChildOf(destClass)) // downcast
				{
					return reinterpret_cast<DestType>(_o);
				}
			}
		}
		return nullptr;
	}

	// --- Enum
	template <typename T>
	bool Enum::getValueFromString(const char* _string, T& _outValue) const
	{
		if (_string == nullptr)
			return false;

		size_t hash = HashCString(_string);
		auto it = m_valuesByNameHash.find(hash);
		if (it != m_valuesByNameHash.end())
		{
			_outValue = static_cast<T>(it->second->getValue());
			return true;
		}
		return false;
	}

	template <typename T>
	bool Enum::getStringFromValue(T _value, const char*& _outString) const
	{
		int64_t value = static_cast<int64_t>(_value);
		for (auto it = m_values.begin(); it != m_values.end(); ++it)
		{
			EnumValue* enumValue = *it;
			if (enumValue->getValue() == value)
			{
				_outString = enumValue->getName();
				return true;
			}
		}
		return false;
	}

	//-----------------------------------------------------------------------------
	// Types Initialization
	//-----------------------------------------------------------------------------

	template <typename T>
	struct TypeInitializer
	{
		TypeInitializer(TypeInfo _typeInfo, const char* _name)
		{
			typeDesc = new Type(_typeInfo, _name);
			typeDesc->createVirtualTypeWrapper<T>();
			GetTypeSet().addType(typeDesc);
		}
		~TypeInitializer()
		{
			GetTypeSet().removeType(typeDesc);
			delete typeDesc;
		}
		Type* typeDesc = nullptr;
	};

	#define TYPEDESCINITIALIZER_DECLARE(_type) extern TypeInitializer<_type> g_##_type##TypeInitializer

	TYPEDESCINITIALIZER_DECLARE(void);
	TYPEDESCINITIALIZER_DECLARE(bool);
	TYPEDESCINITIALIZER_DECLARE(char);
	TYPEDESCINITIALIZER_DECLARE(int8_t);
	TYPEDESCINITIALIZER_DECLARE(int16_t);
	TYPEDESCINITIALIZER_DECLARE(int32_t);
	TYPEDESCINITIALIZER_DECLARE(int64_t);
	TYPEDESCINITIALIZER_DECLARE(uint8_t);
	TYPEDESCINITIALIZER_DECLARE(uint16_t);
	TYPEDESCINITIALIZER_DECLARE(uint32_t);
	TYPEDESCINITIALIZER_DECLARE(uint64_t);
	TYPEDESCINITIALIZER_DECLARE(float);
	TYPEDESCINITIALIZER_DECLARE(double);

	#undef TYPEDESCINITIALIZER_DECLARE

	// === Static Function (WIP) ===
	// @TODO: refactor this so that we can have a full return type + argument type list at construction time, so that we can generate a unique name
    // @TODO: add a special initializer for function type desc so that we can manage its responsibility correctly
    class MIRROR_API StaticFunction : public Type
    {
    public:
        StaticFunction()
                : Type(TypeInfo_StaticFunction, "StaticFunction")
        {
        }

        template <typename T>
        void setReturnType()
        {
            m_returnType = TypeGetter<T>::Get()->getTypeID();
        }

        template <typename T>
        void addArgument() {
            Type* typeDesc = TypeGetter<T>::Get();
            m_argumentTypes.push_back(typeDesc->getTypeID());
        }

    private:
        TypeID m_returnType = UNDEFINED_TYPEID;
        std::vector<TypeID> m_argumentTypes;
    };

    /*
     * Some templates to do reflection for functions
     */
	template<typename NotAFunction>
	struct FunctionTraits {}; /// #1

	template<typename R, typename... Args>
	struct FunctionTraits<R(*)(Args...)> {
		using result = R;
		using args = std::tuple<Args...>;
	};

	template<typename F>
	using FunctionArguments_T = typename FunctionTraits<F>::args;

	template <std::size_t N, typename F>
	using FunctionArgument_T = typename std::tuple_element<N, FunctionArguments_T<F>>::type;

	template <typename F, typename T, T I, T Top>
	struct FunctionArgumentsUnpiler
	{
		static void Unpile(class StaticFunction* _function)
		{
			using ArgumentType = FunctionArgument_T<I, F>;
			_function->addArgument<ArgumentType>();
			FunctionArgumentsUnpiler<F, T, I + 1u, Top>::Unpile(_function);
		}
	};

	template <typename F, typename T, T I>
	struct FunctionArgumentsUnpiler<F, T, I, I>
	{
		static void Unpile(class StaticFunction* _function) {}
	};

	template<typename F>
	FunctionArguments_T<F> MakeFunctionArgumentsTuple(F _function)
	{
		return FunctionArguments_T<F>{};
	}

	template<typename Function, typename Tuple, size_t ... I>
	auto CallFunction(Function f, Tuple t, std::index_sequence<I ...>)
	{
		return f(std::get<I>(t) ...);
	}

	template<typename Function, typename Tuple>
	auto CallFunction(Function f, Tuple t)
	{
		static constexpr auto size = std::tuple_size<Tuple>::value;
		return CallFunction(f, t, std::make_index_sequence<size>{});
	}

	template <size_t I, size_t Top>
	struct FunctionArgumentsFiller
	{
		template <typename C, typename ...ArgumentsTypes>
		static void Fill(C* _classInstance, const char** _memberNames, size_t _memberCount, std::tuple<ArgumentsTypes...>& _arguments)
		{
			using Argument_T = typename std::tuple_element<I, std::tuple<ArgumentsTypes...>>::type;

			assert(_classInstance != nullptr);
			Class* c = _classInstance->GetClass();
			ClassMember* member = c->findMemberByName(_memberNames[I]);
			assert(member != nullptr);
			assert(member->getType() == TypeGetter<Argument_T>::Get());

			std::get<I>(_arguments) = *reinterpret_cast<Argument_T*>(member->getInstanceMemberPointer(_classInstance));

			FunctionArgumentsFiller<I + 1u, Top>::Fill(_classInstance, _memberNames, _memberCount, _arguments);
		}
	};

	template <size_t I>
	struct FunctionArgumentsFiller<I, I>
	{
		template <typename C, typename ...ArgumentsTypes>
		static void Fill(C* _classInstance, const char** _memberNames, size_t _memberCount, std::tuple<ArgumentsTypes...>& _arguments)
		{
		}
	};

	template<typename F, typename C>
	typename FunctionTraits<F>::result CallFunctionWithClassMembersAsArguments(F _functionPointer, C* _classInstance, const char** _memberNames, size_t _memberCount)
	{
		auto arguments = MakeFunctionArgumentsTuple(_functionPointer);

		constexpr size_t argumentsCount = std::tuple_size<typename FunctionTraits<F>::args>::value;
		assert(argumentsCount == _memberCount);
		FunctionArgumentsFiller<0, argumentsCount>::Fill(_classInstance, _memberNames, _memberCount, arguments);

		return CallFunction(_functionPointer, arguments);
	}

	template <typename T>
	struct TypeGetter<T, void, void, void, std::enable_if_t<std::is_function<T>::value>>
	{
		static Type* Get()
		{
			Type* typeDesc = GetTypeSet().findTypeByID(GetTypeID<T>());
			if (typeDesc == nullptr)
			{
				using function_pointer_t = typename std::add_pointer<T>::type;

				StaticFunction* staticFunction = new class StaticFunction();
				staticFunction->createVirtualTypeWrapper<T>();
				GetTypeSet().addType(staticFunction);

				// Return type
				using ReturnType = typename FunctionTraits<function_pointer_t>::result;
				StaticFunction->setReturnType<ReturnType>();

				// Arguments
				constexpr size_t argumentsCount = std::tuple_size<FunctionArguments_T<function_pointer_t>>::value;
				FunctionArgumentsUnpiler<function_pointer_t, std::size_t, 0, argumentsCount>::Unpile(StaticFunction);

				typeDesc = StaticFunction;
			}
			return typeDesc;
		}
	};

	template <typename F>
	StaticFunction* GetStaticFunctionType(F* _function)
	{
		return static_cast<StaticFunction*>(TypeGetter<F>::Get());
	}

} // namespace mirror

// Include to add custom types in. Just '#define MIRROR_EXTENSION_FILE "myfilename.h"' from your build system
#ifdef MIRROR_EXTENSION_FILE
#include MIRROR_EXTENSION_FILE
#endif

//*****************************************************************************
// Implementation
//*****************************************************************************

#ifdef MIRROR_IMPLEMENTATION

#include <cstring>
#include <string>

namespace mirror {

	//-----------------------------------------------------------------------------
	// Helpers & Tools
	//-----------------------------------------------------------------------------

	#define ALLOCATE_AND_COPY_STRING(_dst, _src) { _dst = (char*)malloc(strlen(_src) + 1); strcpy(_dst, _src); }

	uint32_t Hash32(const void* _data, size_t _size)
	{
		const uint32_t OFFSET_BASIS = 2166136261;
		const uint32_t FNV_PRIME = 16777619;

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

	const char* TypeInfoToString(TypeInfo _type)
	{
		switch(_type)
		{
			case TypeInfo_none: return "none";
			case TypeInfo_void: return "void";
			case TypeInfo_bool: return "bool";
			case TypeInfo_char: return "char";
			case TypeInfo_int8: return "int8";
			case TypeInfo_int16: return "int16";
			case TypeInfo_int32: return "int32";
			case TypeInfo_int64: return "int64";
			case TypeInfo_uint8: return "uint8";
			case TypeInfo_uint16: return "uint16";
			case TypeInfo_uint32: return "uint32";
			case TypeInfo_uint64: return "uint64";
			case TypeInfo_float: return "float";
			case TypeInfo_double: return "double";
			case TypeInfo_Enum: return "Enum";
			case TypeInfo_Class: return "Class";
			case TypeInfo_Pointer: return "Pointer";
			case TypeInfo_FixedSizeArray: return "FixedSizeArray";
			case TypeInfo_StaticFunction: return "StaticFunction";
			case TypeInfo_Custom: return "Custom";
			case TypeInfo_COUNT: return "COUNT";
			default: assert(false);
		}
		return "";
	}

	//-----------------------------------------------------------------------------
	// Global Functions
	//-----------------------------------------------------------------------------
	void InitNewTypes()
	{
		GetTypeSet().initNewTypes();
	}

	Type* FindTypeByName(const char* _name)
	{
		return GetTypeSet().findTypeByName(_name);
	}

	Class* FindClassByName(const char* _name)
	{
		Type* type = FindTypeByName(_name);
		return type != nullptr ? type->asClass() : nullptr;
	}

	Type* FindTypeByID(TypeID _id)
	{
		return GetTypeSet().findTypeByID(_id);
	}

	Type* GetType(TypeID _id)
	{
		return GetTypeSet().findTypeByID(_id);
	}

	Class* AsClass(TypeID _id)
	{
		Type* type = GetType(_id);
		return type != nullptr ? type->asClass() : nullptr;
	}

	Enum* AsEnum(TypeID _id)
	{
		Type* type = GetType(_id);
		return type != nullptr ? type->asEnum() : nullptr;
	}

	Pointer* AsPointer(TypeID _id)
	{
		Type* type = GetType(_id);
		return type != nullptr ? type->asPointer() : nullptr;
	}

	StaticFunction* AsStaticFunction(TypeID _id)
	{
		Type* type = GetType(_id);
		return type != nullptr ? type->asStaticFunction() : nullptr;
	}

	FixedSizeArray* AsFixedSizeArray(TypeID _id)
	{
		Type* type = GetType(_id);
		return type != nullptr ? type->asFixedSizeArray() : nullptr;
	}

	TypeSet& GetTypeSet()
    {
		if (g_typeSetPtr == nullptr)
		{
			g_typeSetPtr = new TypeSet(); // This will leak for now, but it's not a big deal
		}
        return *g_typeSetPtr;
    }

	//-----------------------------------------------------------------------------
	// Meta Data
	//-----------------------------------------------------------------------------

	const char* MetaData::getName() const
	{
		return m_name;
	}

	bool MetaData::asBool() const
	{
		return strcmp("true", m_data) == 0;
	}

	int MetaData::asInt() const
	{
		return atoi(m_data);
	}

	float MetaData::asFloat() const
	{
		return atof(m_data);
	}

	const char* MetaData::asString() const
	{
		return m_data;
	}

	MetaData::MetaData(const char* _name, const char* _data)
	{
		ALLOCATE_AND_COPY_STRING(m_name, _name);
        ALLOCATE_AND_COPY_STRING(m_data, _data);
	}

	MetaData::MetaData(const MetaData& _other)
		: MetaData(_other.m_name, _other.m_data)
	{
	}

	MetaData::~MetaData()
    {
        free(m_name);
        free(m_data);
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

    MetaDataSet::MetaDataSet(const char* _metaDataString)
	{
		auto sanitizeMetaDataString = [](char* _buf)
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
						lastChar = cur + 1;
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
		};

		// Parse meta data
		assert(_metaDataString != nullptr);
		size_t len = strlen(_metaDataString);

		const char* key = _metaDataString;
		size_t keyLen = 0;
		const char* value = nullptr;
		size_t valueLen = 0;
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
				value = nullptr;
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

	//-----------------------------------------------------------------------------
	// TypeSet
	//-----------------------------------------------------------------------------

	TypeSet::~TypeSet()
	{
		for (Type* type : m_types)
		{
			delete type;
		}
		m_types.clear();
		m_typesByID.clear();
		m_typesByName.clear();
	}

	Type* TypeSet::findTypeByID(TypeID _typeID) const
	{
		auto it = m_typesByID.find(_typeID);
		return it != m_typesByID.end() ? it->second : nullptr;
	}

	mirror::Type* TypeSet::findTypeByName(const char* _name) const
	{
		uint32_t nameHash = HashCString(_name);
		auto it = m_typesByName.find(nameHash);
		return it != m_typesByName.end() ? it->second : nullptr;
	}

	void TypeSet::addType(Type* _type)
	{
		assert(_type);

		// Using dlls can result in multiple registration the same type. We accept the first type and screen all others
		auto result = m_typesRegistrationCount.insert(std::make_pair(_type->getTypeID(), 0));
		++result.first->second;
		if (result.first->second > 1)
		{
			return;
		}

		// Checks if a type with the same typeID does not already exists
		assert(m_typesByID.find(_type->getTypeID()) == m_typesByID.end());
		assert(m_types.find(_type) == m_types.end());

		m_typesByID.insert(std::make_pair(_type->getTypeID(), _type));
		m_types.emplace(_type);
	}

	void TypeSet::addTypeName(Type* _type, const char* _name)
	{
		assert(_type);
		assert(_name);

		uint32_t nameHash = HashCString(_name);
		assert(m_typesByName.find(nameHash) == m_typesByName.end());
		m_typesByName.insert(std::make_pair(nameHash, _type));
	}

	void TypeSet::removeType(Type* _type)
	{
		assert(_type);

		// Using dlls can result in multiple registration the same type. We accept the first type and screen all others
		auto registrationCountIt = m_typesRegistrationCount.find(_type->getTypeID());		
		assert(registrationCountIt != m_typesRegistrationCount.end());
		--registrationCountIt->second;
		if (registrationCountIt->second > 0)
		{
			return;
		}

		// Checks if a type with the same typeID already exists
		{
			auto it = m_typesByID.find(_type->getTypeID());
			assert(it != m_typesByID.end());
			m_typesByID.erase(it);
		}

		if (_type->m_initialized)
		{
			uint32_t nameHash = HashCString(_type->getName());
			auto it = m_typesByName.find(nameHash);
			if (it != m_typesByName.end())
			{
				m_typesByName.erase(it);
			}
			_type->shutdown();
			_type->m_initialized = false;
		}
		
		{
			// because of the dll multiple loading thing, last removed type won't necessary be the same pointer, that's why we compare ids (although, if the dll has been unloaded, the call to getType will probably crash)
			auto it = std::find_if(m_types.begin(), m_types.end(),
				[_type](Type* _t) { return _t->getTypeID() == _type->getTypeID(); }
			);
			assert(it != m_types.end());
			m_types.erase(it);
		}
	}

	const std::set<Type*>& TypeSet::getTypes() const
	{
		return m_types;
	}

	void TypeSet::initNewTypes()
	{
		for (Type* type : m_types)
		{
			if (!type->m_initialized)
			{
				type->init();

				uint32_t nameHash = HashCString(type->getName());
				assert(m_typesByName.find(nameHash) == m_typesByName.end());
				m_typesByName.insert(std::make_pair(nameHash, type));

				type->m_initialized = true;
			}
		}
	}

	TypeSet* g_typeSetPtr = nullptr;


	//-----------------------------------------------------------------------------
	// Type
	//-----------------------------------------------------------------------------

	TypeInfo Type::getTypeInfo() const
	{
		return m_typeInfo;
	}

	const char* Type::getName() const
	{
		return m_name;
	}

	const char* Type::getCustomTypeName() const
	{
		return m_customTypeName;
	}

	bool Type::isCustomType(const char* _customTypeName) const
	{
		return m_typeInfo == TypeInfo_Custom && strcmp(_customTypeName, m_customTypeName) == 0;
	}

	TypeID Type::getTypeID() const
	{
		return m_virtualTypeWrapper->getTypeID();
	}

	size_t Type::getSize() const
	{
		return m_virtualTypeWrapper->getSize();
	}

	const Class* Type::asClass() const
	{
		if (getTypeInfo() == TypeInfo_Class)
		{
			return static_cast<const Class*>(this);
		}
		return nullptr;
	}

	const Enum* Type::asEnum() const
	{
		if (getTypeInfo() == TypeInfo_Enum)
		{
			return static_cast<const Enum*>(this);
		}
		return nullptr;
	}

	const Pointer* Type::asPointer() const
	{
		if (getTypeInfo() == TypeInfo_Pointer)
		{
			return static_cast<const Pointer*>(this);
		}
		return nullptr;
	}

	const StaticFunction* Type::asStaticFunction() const
	{
		if (getTypeInfo() == TypeInfo_StaticFunction)
		{
			return static_cast<const StaticFunction*>(this);
		}
		return nullptr;
	}

	const FixedSizeArray* Type::asFixedSizeArray() const
	{
		if (getTypeInfo() == TypeInfo_FixedSizeArray)
		{
			return static_cast<const FixedSizeArray*>(this);
		}
		return nullptr;
	}

	Class* Type::asClass()
	{
		if (getTypeInfo() == TypeInfo_Class)
		{
			return static_cast<Class*>(this);
		}
		return nullptr;
	}

	Enum* Type::asEnum()
	{
		if (getTypeInfo() == TypeInfo_Enum)
		{
			return static_cast<Enum*>(this);
		}
		return nullptr;
	}

	Pointer* Type::asPointer()
	{
		if (getTypeInfo() == TypeInfo_Pointer)
		{
			return static_cast<Pointer*>(this);
		}
		return nullptr;
	}

	StaticFunction* Type::asStaticFunction()
	{
		if (getTypeInfo() == TypeInfo_StaticFunction)
		{
			return static_cast<StaticFunction*>(this);
		}
		return nullptr;
	}

	FixedSizeArray* Type::asFixedSizeArray()
	{
		if (getTypeInfo() == TypeInfo_FixedSizeArray)
		{
			return static_cast<FixedSizeArray*>(this);
		}
		return nullptr;
	}

	Type::Type(TypeInfo _typeInfo, const char* _name)
		: m_typeInfo(_typeInfo)
	{
		ALLOCATE_AND_COPY_STRING(m_name, _name);
	}

	bool Type::hasFactory() const
	{
		return m_virtualTypeWrapper->hasFactory();
	}

	void* Type::instantiate(AllocateFunction _allocateFunction, void* _userData) const
	{
		return m_virtualTypeWrapper->instantiate(_allocateFunction, _userData);
	}

	void Type::shutdown()
	{

	}

	void Type::init()
	{

	}

	Type::~Type()
	{
		if (m_virtualTypeWrapper) delete m_virtualTypeWrapper;
		free(m_name);
	}

	void Type::setName(const char* _name)
	{
		free(m_name);
		ALLOCATE_AND_COPY_STRING(m_name, _name);
	}

	void Type::setCustomTypeName(const char* _name)
	{
		free(m_customTypeName);
		ALLOCATE_AND_COPY_STRING(m_customTypeName, _name);
	}

	//-----------------------------------------------------------------------------
	// Specialized Type Descs
	//-----------------------------------------------------------------------------

	// --- Class
	size_t Class::getMembersCount(bool _includeInheritedMembers) const
	{
		size_t count = m_members.size();
		if (_includeInheritedMembers)
		{
			for (TypeID parentID : m_parents)
			{
				Class* parent = AsClass(parentID);
				assert(parent != nullptr);
				count += parent->getMembersCount(true);
			}
		}
		return count;
	}

	std::vector<ClassMember*> Class::getMembers(bool _includeInheritedMembers) const
	{
		std::vector<ClassMember*> members;
		getMembers(members, _includeInheritedMembers);
		return members;
	}

	size_t Class::getMembers(ClassMember** _outMemberList, size_t _memberListSize, bool _includeInheritedMembers) const
	{
		size_t writtenCount = 0;

		for (ClassMember* member : m_members)
		{
			if (writtenCount >= _memberListSize)
				break;

			_outMemberList[writtenCount] = member;
			++writtenCount;
		}

		if (_includeInheritedMembers)
		{
			for (TypeID parentID : m_parents)
			{
				Class* parent = AsClass(parentID);
				assert(parent != nullptr);
				writtenCount += parent->getMembers(_outMemberList + writtenCount, _memberListSize - writtenCount, true);
			}
		}

		return writtenCount;
	}

	void Class::getMembers(std::vector<ClassMember*>& _outMemberList, bool _includeInheritedMembers) const
	{
		_outMemberList.insert(_outMemberList.end(), m_members.begin(), m_members.end());

		if (_includeInheritedMembers)
		{
			for (TypeID parentID : m_parents)
			{
				Class* parent = AsClass(parentID);
				assert(parent != nullptr);
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
			for (TypeID parentID : m_parents)
			{
				Class* parent = AsClass(parentID);
				assert(parent != nullptr);
				ClassMember* member = parent->findMemberByName(_name);
				if (member)
					return member;
			}
		}
		return nullptr;
	}

	Class* Class::getParent() const
	{
		Type* parentType = FindTypeByID(getParentID());
		return parentType != nullptr ? parentType->asClass() : nullptr;
	}

	TypeID Class::getParentID() const
	{
		return m_parents.size() > 0 ? *m_parents.begin() : UNDEFINED_TYPEID;
	}

	const std::set<TypeID>& Class::getParents() const
	{
		return m_parents;
	}

	const std::set<TypeID>& Class::getChildren() const
	{
		return m_children;
	}

	bool Class::isChildOf(const Class* _class, bool _checkSelf) const
	{
		if (_class == this)
			return _checkSelf;

		for (TypeID parentID : m_parents)
		{
			Class* parent = AsClass(parentID);
			assert(parent != nullptr);
			return parent->isChildOf(_class, true);
		}
		return false;
	}

	const MetaDataSet& Class::getMetaDataSet() const
	{
		return m_metaDataSet;
	}

	mirror::Class* Class::unsafeVirtualGetClass(void* _object) const
	{
		return m_virtualTypeWrapper->unsafeVirtualGetClass(_object);
	}

	void Class::addMember(ClassMember* _member)
	{
		assert(_member);
		assert(std::find(m_members.begin(), m_members.end(), _member) == m_members.end());
		uint32_t nameHash = HashCString(_member->getName());
		assert(m_membersByName.find(nameHash) == m_membersByName.end());

		_member->m_ownerClass = this;
		m_members.push_back(_member);
		m_membersByName.insert(std::make_pair(nameHash, _member));
	}

	void Class::addParent(TypeID _parent)
	{
		assert(_parent != UNDEFINED_TYPEID);
		assert(std::find(m_parents.begin(), m_parents.end(), _parent) == m_parents.end());

		m_parents.insert(_parent);
	}

	void Class::shutdown()
	{
		m_children.clear();
	}

	void Class::init()
	{
		for (TypeID parentID : m_parents)
		{
			Class* parent = AsClass(parentID);
			assert(parent != nullptr);
			parent->m_children.insert(getTypeID());
		}
	}

	Class::Class(const char* _name, const char* _metaDataString)
		: Class(_name, MetaDataSet(_metaDataString))
	{

	}

	Class::Class(const char* _name, const MetaDataSet& _metaDataSet)
		: Type(TypeInfo_Class, _name)
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

	const char* ClassMember::getName() const
	{
		return m_name;
	}

	Class* ClassMember::getOwnerClass() const
	{
		return m_ownerClass;
	}

	size_t ClassMember::getOffset() const
	{
		return m_offset;
	}

	Type* ClassMember::getType() const
	{
		return GetTypeSet().findTypeByID(m_typeInfo);
	}

	void* ClassMember::getInstanceMemberPointer(void* _classInstancePointer) const
	{
		return reinterpret_cast<uint8_t*>(_classInstancePointer) + m_offset;
	}

	const MetaDataSet& ClassMember::getMetaDataSet() const
	{
		return m_metaDataSet;
	}

	ClassMember::ClassMember(const char* _name, size_t _offset, TypeID _type, const char* _metaDataString)
		: m_offset(_offset)
		, m_typeInfo(_type)
		, m_metaDataSet(_metaDataString)
	{
		ALLOCATE_AND_COPY_STRING(m_name, _name);
	}

	ClassMember::~ClassMember()
	{
		free(m_name);
	}

	// --- Enum
	const std::vector<EnumValue*>& Enum::getValues() const
	{
		return m_values;
	}

	Type* Enum::getSubType() const
	{
		return GetType(m_subType);
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

	Enum::Enum(const char* _name, TypeID _subType)
		: Type(TypeInfo_Enum, _name)
		, m_subType(_subType)
	{

	}

	const char* EnumValue::getName() const
	{
		return m_name;
	}

	int64_t EnumValue::getValue() const
	{
		return m_value;
	}

	EnumValue::EnumValue(const char* _name, int64_t _value)
		: m_value(_value)
	{
		// Remove all namespaces from the name
		int colonPosition = strlen(_name) - 1;
		for (; colonPosition >= 0; --colonPosition)
		{
			if (_name[colonPosition] == ':')
				break;
		}
		ALLOCATE_AND_COPY_STRING(m_name, _name + (colonPosition + 1))
	}

	EnumValue::~EnumValue()
	{
		free(m_name);
	}

	// --- Pointer
	Type* Pointer::getSubType() const
	{
		return GetTypeSet().findTypeByID(m_subType);
	}

	Pointer::Pointer(TypeID _subType)
		: Type(TypeInfo_Pointer, "")
		, m_subType(_subType)
	{
	}

	void Pointer::init()
	{
		Type::init();
		setName((std::string("pointer_") + std::string(FindTypeByID(m_subType)->getName())).c_str());
	}

	// --- Fixed Size Array
	Type* FixedSizeArray::getSubType() const
	{
		return GetTypeSet().findTypeByID(m_subType);
	}

	void* FixedSizeArray::getDataAt(void* _basePtr, size_t _index) const
	{
		assert(_index < m_elementCount);
		return ((char*)_basePtr) + _index * getSubType()->getSize();
	}

	FixedSizeArray::FixedSizeArray(TypeID _subType, size_t _elementCount)
		: Type(TypeInfo_FixedSizeArray, "")
		, m_subType(_subType)
		, m_elementCount(_elementCount)
	{
		
	}

	void FixedSizeArray::init()
	{
		Type::init();

		const char* format = "array%d_%s";
		Type* subType = GetTypeSet().findTypeByID(m_subType);
		size_t nameSize = snprintf(nullptr, 0, format, m_elementCount, subType->getName());
		std::string name;
		name.resize(nameSize);
		snprintf(const_cast<char*>(name.data()), nameSize + 1, format, m_elementCount, subType->getName());
		setName(name.c_str());
	}

	//-----------------------------------------------------------------------------
	// Types Implementation
	//-----------------------------------------------------------------------------

	#define __MIRROR_TYPEDESCINITIALIZER_DEFINE(_type, _mirrorType) ::mirror::TypeInitializer<_type> g_##_type##TypeInitializer(_mirrorType, #_type)

	__MIRROR_TYPEDESCINITIALIZER_DEFINE(void, TypeInfo_void);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(bool, TypeInfo_bool);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(char, TypeInfo_char);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(int8_t, TypeInfo_int8);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(int16_t, TypeInfo_int16);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(int32_t, TypeInfo_int32);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(int64_t, TypeInfo_int64);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(uint8_t, TypeInfo_uint8);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(uint16_t, TypeInfo_uint16);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(uint32_t, TypeInfo_uint32);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(uint64_t, TypeInfo_uint64);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(float, TypeInfo_float);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(double, TypeInfo_double);

	#undef __MIRROR_TYPEDESCINITIALIZER_DEFINE

} // namespace mirror

#endif