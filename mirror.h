#pragma once


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
#include <assert.h>

//-----------------------------------------------------------------------------
// Mirror
//-----------------------------------------------------------------------------

namespace mirror {

	//-----------------------------------------------------------------------------
	// Forward Declarations
	//-----------------------------------------------------------------------------
	typedef size_t TypeID;
	class TypeSet;
	class TypeDesc;
	class Class;
	class Enum;
	class PointerTypeDesc;
	class StaticFunctionTypeDesc;
	class FixedSizeArrayTypeDesc;
	class ClassMember;
	class EnumValue;
	class VirtualTypeWrapper;
	struct MetaData;
	struct MetaDataSet;

	const TypeID UNDEFINED_TYPEID = 0;

	//-----------------------------------------------------------------------------
	// Enums
	//-----------------------------------------------------------------------------
	enum Type
	{
		Type_none = 0,

		Type_void,

		Type_bool,
		Type_char,
		Type_int8,
		Type_int16,
		Type_int32,
		Type_int64,
		Type_uint8,
		Type_uint16,
		Type_uint32,
		Type_uint64,
		Type_float,
		Type_double,

		Type_Enum,

		Type_Class,

		Type_Pointer,
		Type_FixedSizeArray,

		Type_StaticFunction,

		Type_COUNT,
	};

	//-----------------------------------------------------------------------------
	// Helpers & Tools
	//-----------------------------------------------------------------------------
	MIRROR_API uint32_t Hash32(const void* _data, size_t _size);
	MIRROR_API uint32_t HashCString(const char* _str);

	//-----------------------------------------------------------------------------
	// Global Functions
	//-----------------------------------------------------------------------------
	template <typename T> constexpr TypeID GetTypeID();
	template <typename T> constexpr TypeID GetTypeID(T&);

	template <typename T> TypeDesc* GetTypeDesc();
	template <typename T> TypeDesc* GetTypeDesc(T&);

    template<typename T> Enum* GetEnum();
    template<typename T> Enum* GetEnum(T&);

	template <typename T> Class* GetClass();
	template <typename T> Class* GetClass(T&);

	template <typename DestType, typename SourceType> DestType Cast(SourceType _o);

	MIRROR_API TypeDesc* FindTypeByName(const char* _name);

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
		std::unordered_map<uint32_t, MetaData> m_metaData;
	};

	//-----------------------------------------------------------------------------
	// TypeSet
	//-----------------------------------------------------------------------------

	class MIRROR_API TypeSet
	{
	public:
		~TypeSet();

		TypeDesc* findTypeByID(TypeID _typeID);
		TypeDesc* findTypeByName(const char* _name);

		void addType(TypeDesc* _type);
		void removeType(TypeDesc* _type);

		const std::set<TypeDesc*>& getTypes() const;

	private:
		std::set<TypeDesc*> m_types;
		std::unordered_map<TypeID, TypeDesc*> m_typesByID;
		std::unordered_map<uint32_t, TypeDesc*> m_typesByName;
	};

	extern TypeSet* g_typeSetPtr;

	//-----------------------------------------------------------------------------
	// TypeDesc
	//-----------------------------------------------------------------------------

	class MIRROR_API TypeDesc
	{
	public:
		Type getType() const;
		const char* getName() const;
		TypeID getTypeID() const;
		size_t getSize() const;

		// @TODO(2021/02/15|Remi): Allow the user to choose their allocator
		bool hasFactory() const;
		void* instantiate() const;

	// internal
		void setName(const char* _name);
		TypeDesc(Type _type, VirtualTypeWrapper* _virtualTypeWrapper);
		TypeDesc(Type _type, const char* _name, VirtualTypeWrapper* _virtualTypeWrapper);
		// NOTE(Remi|2020/04/26): virtual specifier is not needed, but is added to allow the debugger to show inherited types
		virtual ~TypeDesc();

		char* m_name;
		Type m_type = Type_none;
		VirtualTypeWrapper* m_virtualTypeWrapper = nullptr;
	};


	// @NOTE(remi): Need to rewrite this a bit after
	class MIRROR_API VirtualTypeWrapper
	{
	public:
		TypeID getTypeID() const { return m_typeID; }
		size_t getSize() const { return m_size; }

		virtual bool hasFactory() const { return false; }
		virtual void* instantiate() const { return nullptr; }

		virtual Class* unsafeVirtualGetClass(void* _object) const { return nullptr; }

		virtual ~VirtualTypeWrapper() {}
		TypeID m_typeID = UNDEFINED_TYPEID;
		size_t m_size = 0;
	};

	//-----------------------------------------------------------------------------
	// Specialized Type Descs
	//-----------------------------------------------------------------------------

	// --- Class
	class MIRROR_API Class : public TypeDesc
	{
	public:
		void getMembers(std::vector<ClassMember*>& _outMemberList, bool _includeInheritedMembers = true) const;
		ClassMember* findMemberByName(const char* _name, bool _includeInheritedMembers = true) const;

		const std::set<Class*>& getParents() const;
		const std::set<Class*>& getChildren() const;

		bool isChildOf(const Class* _class, bool _checkSelf = true) const;

		const MetaDataSet& getMetaDataSet() const;

		Class* unsafeVirtualGetClass(void* _object) const;

	// internal
		void addMember(ClassMember* _member);
		void addParent(Class* _parent);
		Class(const char* _name, VirtualTypeWrapper* _virtualTypeWrapper, const char* _metaDataString);
		Class(const char* _name, VirtualTypeWrapper* _virtualTypeWrapper, const MetaDataSet& _metaDataSet);
		virtual ~Class();

		// @TODO(remi): replace those pointers by TypeIDs
		std::set<Class*> m_parents;
		std::set<Class*> m_children;
		std::vector<ClassMember*> m_members;
		std::unordered_map<uint32_t, ClassMember*> m_membersByName;
		MetaDataSet m_metaDataSet;
	};

	class MIRROR_API ClassMember
	{
	public:
		const char* getName() const;
		Class* getClass() const;
		size_t getOffset() const;
		TypeDesc* getType() const;

		void* getInstanceMemberPointer(void* _classInstancePointer) const;
		const MetaDataSet& GetMetaDataSet() const;

	// internal
		ClassMember(const char* _name, size_t _offset, TypeID _type, const char* _metaDataString);
		~ClassMember();

		Class* m_class = nullptr;
		char* m_name;
		size_t m_offset;
		TypeID m_type = UNDEFINED_TYPEID;
		MetaDataSet m_metaDataSet;
	};

	// --- Enum
	class MIRROR_API Enum : public TypeDesc
	{
	public:

		template <typename T> bool getValueFromString(const char* _string, T& _outValue) const;
		template <typename T> bool getStringFromValue(T _value, const char*& _outString) const;

		const std::vector<EnumValue*>& getValues() const;
		TypeDesc* getSubType() const;

	// internal
		void addValue(EnumValue* _value);
		Enum(const char* _name, VirtualTypeWrapper* _virtualTypeWrapper, TypeDesc* _subType = nullptr);

		std::vector<EnumValue*> m_values;
		std::unordered_map<size_t, EnumValue*> m_valuesByNameHash;

		TypeDesc* m_subType;
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
	class MIRROR_API PointerTypeDesc : public TypeDesc
	{
	public:
		TypeDesc* getSubType() const;

	// internal
		PointerTypeDesc(TypeID _subType, VirtualTypeWrapper* _virtualTypeWrapper);

		TypeID m_subType = UNDEFINED_TYPEID;
	};

	// --- Fixed Size Array
	class MIRROR_API FixedSizeArrayTypeDesc : public TypeDesc
	{
	public:

		TypeDesc* getSubType() const;
		size_t getElementCount() const { return m_elementCount; }

	// internal
		FixedSizeArrayTypeDesc(TypeID _subType, size_t _elementCount, VirtualTypeWrapper* _virtualTypeWrapper);

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

#define MIRROR_CLASS(_class, ...)\
MIRROR_PUSH_DISABLE_WARNINGS \
public:\
	virtual ::mirror::Class* getClass() const { return _class::GetClass(); }\
	__MIRROR_CLASS_CONSTRUCTION(_class, __VA_ARGS__)


#define MIRROR_CLASS_NOVIRTUAL(_class, ...)\
MIRROR_PUSH_DISABLE_WARNINGS \
public:\
	::mirror::Class* getClass() const { return _class::GetClass(); }\
	__MIRROR_CLASS_CONSTRUCTION(_class, __VA_ARGS__)

#define __MIRROR_CLASS_CONSTRUCTION(_class, ...)\
	static ::mirror::Class* GetClass() { return ::mirror::GetClass<_class>(); }\
	\
	static ::mirror::ClassInitializer<_class, false> __MirrorInitializer;\
	static ::mirror::Class* __MirrorCreateClass()\
	{\
		using classType = _class;\
		\
		const char* metaDataString = #__VA_ARGS__"";\
		mirror::MetaDataSet metaDataSet(metaDataString);\
		mirror::VirtualTypeWrapper* virtualTypeWrapper = new mirror::TVirtualTypeWrapper<classType, false, true>();\
		::mirror::Class* clss = new ::mirror::Class(#_class, virtualTypeWrapper, metaDataSet);\
		char fakePrototype[sizeof(_class)] = {};\
		_class* prototypePtr = reinterpret_cast<_class*>(fakePrototype);\
		(void)prototypePtr;\
		{\
		__MIRROR_CLASS_CONTENT

#define __MIRROR_CLASS_CONTENT(...)\
			__VA_ARGS__\
		}\
		return clss;\
	}\
	MIRROR_POP_DISABLE_WARNINGS

#define MIRROR_MEMBER(_memberName)\
	{\
		size_t offset = reinterpret_cast<size_t>(&(prototypePtr->_memberName)) - reinterpret_cast<size_t>(prototypePtr);\
		::mirror::TypeDesc* type = ::mirror::GetTypeDesc(prototypePtr->_memberName);\
		const char* memberName = #_memberName;\
		__MIRROR_MEMBER_CONTENT

#define __MIRROR_MEMBER_CONTENT(...)\
		const char* metaDataString = #__VA_ARGS__"";\
		::mirror::ClassMember* classMember = new ::mirror::ClassMember(memberName, offset, type->getTypeID(), metaDataString);\
		clss->addMember(classMember);\
	}

#define MIRROR_PARENT(_parentClass)\
	{\
		clss->addParent(::mirror::GetClass<_parentClass>());\
	}

#define MIRROR_CLASS_DEFINITION(_class)\
	::mirror::ClassInitializer<_class, false> _class::__MirrorInitializer;

#define MIRROR_ENUM(_enumName)\
template <> struct ::mirror::TypeDescGetter<_enumName> {	static ::mirror::TypeDesc* Get() { \
	using enumType = _enumName; \
	static ::mirror::Enum* s_enum = nullptr; \
	if (s_enum == nullptr) \
	{ \
		s_enum = new ::mirror::Enum(#_enumName, new TVirtualTypeWrapper<enumType>()); \
		__MIRROR_ENUM_CONTENT

#define MIRROR_ENUM_VALUE(_enumValue)\
		s_enum->addValue(new ::mirror::EnumValue(#_enumValue, _enumValue));\
		_MIRROR_ENUM_VALUE_CONTENT

#define MIRROR_ENUM_CLASS(_enumName)\
template <> struct ::mirror::TypeDescGetter<_enumName> {	static ::mirror::TypeDesc* Get() { \
	using enumType = _enumName; \
	static ::mirror::Enum* s_enum = nullptr; \
	if (s_enum == nullptr) \
	{ \
		TypeDesc* subType; \
		switch(sizeof(enumType)) { \
			case 1: subType = ::mirror::TypeDescGetter<int8_t>::Get(); break; \
			case 2: subType = ::mirror::TypeDescGetter<int16_t>::Get(); break; \
			case 4: subType = ::mirror::TypeDescGetter<int32_t>::Get(); break; \
			case 8: subType = ::mirror::TypeDescGetter<int64_t>::Get(); break; \
		} \
		s_enum = new ::mirror::Enum(#_enumName, new TVirtualTypeWrapper<enumType>(), subType); \
		__MIRROR_ENUM_CONTENT

#define MIRROR_ENUM_CLASS_VALUE(_enumValue)\
		s_enum->addValue(new ::mirror::EnumValue(#_enumValue, int64_t(enumType::_enumValue)));\
		_MIRROR_ENUM_VALUE_CONTENT

#define __MIRROR_ENUM_CONTENT(...)\
		__VA_ARGS__\
		::mirror::GetTypeSet()->addType(s_enum);\
	}\
	return s_enum;\
}};

#define _MIRROR_ENUM_VALUE_CONTENT(...)



//*****************************************************************************
// Inline Implementations
//*****************************************************************************

#include <typeinfo>

namespace mirror {

	//-----------------------------------------------------------------------------
	// Global Functions
	//-----------------------------------------------------------------------------

	// --- TypeDescGetter
	template <typename T, typename IsArray = void, typename IsPointer = void, typename IsEnum = void, typename IsFunction = void>
	struct TypeDescGetter
	{
		static TypeDesc* Get()
		{
			TypeID typeID = GetTypeID<T>();
			return GetTypeSet().findTypeByID(typeID);
		}
	};

	template <typename T>
	struct TypeDescGetter<T, std::enable_if_t<std::is_array<T>::value>>
	{
		static TypeDesc* Get()
		{
			using type = typename std::remove_extent<T>::type;
			static FixedSizeArrayTypeDesc s_fixedSizeArrayTypeDesc(TypeDescGetter<type>::Get(), std::extent<T>::value , new TVirtualTypeWrapper<T>());
			return &s_fixedSizeArrayTypeDesc;
		}
	};

    template <typename T>
    struct PointerTypeDescInitializer
    {
        PointerTypeDescInitializer()
        {
            using type = typename std::remove_pointer<T>::type;
            typeDesc = new PointerTypeDesc(TypeDescGetter<type>::Get()->getTypeID(), new TVirtualTypeWrapper<T>());
            GetTypeSet().addType(typeDesc);
        }
        ~PointerTypeDescInitializer()
        {
            GetTypeSet().removeType(typeDesc);
            delete typeDesc;
        }
        PointerTypeDesc* typeDesc = nullptr;
    };

	template <typename T>
	struct TypeDescGetter<T, void, std::enable_if_t<std::is_pointer<T>::value>>
	{
		static TypeDesc* Get()
		{
			static PointerTypeDescInitializer<T> s_pointerTypeDescInitializer;
			/*TypeDesc* typeDesc = GetTypeSet().findTypeByID(GetTypeID<T>());
			if (!typeDesc)
			{
				using type = typename std::remove_pointer<T>::type;
				PointerTypeDesc* pointerTypeDesc = new PointerTypeDesc(TypeDescGetter<type>::Get()->getTypeID(), new TVirtualTypeWrapper<T>());
				GetTypeSet().addType(pointerTypeDesc);
				typeDesc = pointerTypeDesc;
			}*/
			//return s_pointerTypeDescInitializer->typeDesc;
			return GetTypeSet().findTypeByID(GetTypeID<T>());
		}
	};

	template <typename T>
	struct TypeDescGetter<T, void, void, std::enable_if_t<std::is_enum<T>::value>>
	{
		static TypeDesc* Get()
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

	// Type Desc Accesors
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

	template<typename T>
    Enum* GetEnum()
    {
        TypeDesc* type = TypeDescGetter<T>::Get();
        if (type->getType() != Type_Enum)
            return nullptr;

        return static_cast<Enum*>(type);
    }

    template<typename T>
    Enum* GetEnum(T&)
    {
        return GetEnum<T>();
    }

	template <typename T>
	TypeDesc* GetTypeDesc()
	{
		return TypeDescGetter<T>::Get();
	}

	template <typename T>
	TypeDesc* GetTypeDesc(T&)
	{
		return GetTypeDesc<T>();
	}

	template <typename T>
	Class* GetClass()
	{ 
		TypeDesc* typeDesc = TypeDescGetter<T>::Get();
		if (typeDesc != nullptr && typeDesc->getType() == Type_Class)
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

		static bool Unpile(Class** _destClass, Class** _sourceClass)
		{
			using source_t = typename std::remove_pointer<SourceType>::type;
			using dest_t   = typename std::remove_pointer<DestType>::type;
			return CastClassesUnpiler<dest_t, source_t>::Unpile(_destClass, _sourceClass);
		}
	};

	template <typename DestType, typename SourceType>
	struct CastClassesUnpiler<DestType, SourceType, std::enable_if_t<!std::is_pointer<DestType>::value>, std::enable_if_t<!std::is_pointer<SourceType>::value>>
	{
		static bool Unpile(Class** _destClass, Class** _sourceClass)
		{
			*_destClass = DestType::GetClass();
			*_sourceClass = SourceType::GetClass();

			return true;
		}
	};

	template <typename DestType, typename SourceType>
	DestType Cast(SourceType _o)
	{
		Class* destClass = nullptr;
		Class* sourceClass = nullptr;
		if (CastClassesUnpiler<DestType, SourceType>::Unpile(&destClass, &sourceClass))
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
	// Virtual Type Wrapper
	//-----------------------------------------------------------------------------
	template <typename T, typename IsShallow = void>
	class TVirtualTypeWrapperBase : public VirtualTypeWrapper
	{
	public:
		TVirtualTypeWrapperBase()
		{
			m_typeID = GetTypeID<T>();
			m_size = sizeof(T);
		}
	};

	template <typename T>
	class TVirtualTypeWrapperBase<T, std::enable_if_t<std::is_function<T>::value || std::is_void<T>::value>> : public VirtualTypeWrapper
	{
	public:
		TVirtualTypeWrapperBase()
		{
			m_typeID = GetTypeID<T>();
			m_size = 0;
		}
	};

	template <>
	class TVirtualTypeWrapperBase<void> : public VirtualTypeWrapper
	{
	public:
		TVirtualTypeWrapperBase()
		{
			m_typeID = GetTypeID<void>();
			m_size = 0;
		}
	};

	template <typename T, bool Factory = false>
	class TVirtualTypeWrapperFactory : public TVirtualTypeWrapperBase<T>
	{
	};

	template <typename T>
	class TVirtualTypeWrapperFactory<T, true> : public TVirtualTypeWrapperBase<T>
	{
	public:
		virtual bool hasFactory() const override { return true; }
		virtual void* instantiate() const override { return new T(); }
	};

	template <typename T, bool Factory = false, bool IsClass = false>
	class TVirtualTypeWrapper : public TVirtualTypeWrapperFactory<T, Factory>
	{
	};

	template <typename T, bool Factory>
	class TVirtualTypeWrapper<T, Factory, true> : public TVirtualTypeWrapperFactory<T, Factory>
	{
		virtual Class* unsafeVirtualGetClass(void* _object) const { return reinterpret_cast<T*>(_object)->getClass(); }
	};

	//-----------------------------------------------------------------------------
	// Types Initialization
	//-----------------------------------------------------------------------------

	template <typename T, bool HasFactory = true>
	struct TypeDescInitializer
	{
		TypeDescInitializer(Type _type, const char* _name)
		{
			typeDesc = new TypeDesc(_type, _name, new TVirtualTypeWrapper<T, HasFactory>());
			GetTypeSet().addType(typeDesc);
		}
		~TypeDescInitializer()
		{
			GetTypeSet().removeType(typeDesc);
			delete typeDesc;
		}
		TypeDesc* typeDesc = nullptr;
	};

	template <typename T, bool HasFactory = true>
	struct ClassInitializer
	{
		ClassInitializer()
		{
			clss = T::__MirrorCreateClass();
			GetTypeSet().addType(clss);
		}
		~ClassInitializer()
		{
			GetTypeSet().removeType(clss);
			delete clss;
		}
		Class* clss = nullptr;
	};

	#define TYPEDESCINITIALIZER_DECLARE(_type, _hasFactory) extern TypeDescInitializer<_type, _hasFactory> g_##_type##TypeInitializer

	TYPEDESCINITIALIZER_DECLARE(void, false);
	TYPEDESCINITIALIZER_DECLARE(bool, true);
	TYPEDESCINITIALIZER_DECLARE(char, true);
	TYPEDESCINITIALIZER_DECLARE(int8_t, true);
	TYPEDESCINITIALIZER_DECLARE(int16_t, true);
	TYPEDESCINITIALIZER_DECLARE(int32_t, true);
	TYPEDESCINITIALIZER_DECLARE(int64_t, true);
	TYPEDESCINITIALIZER_DECLARE(uint8_t, true);
	TYPEDESCINITIALIZER_DECLARE(uint16_t, true);
	TYPEDESCINITIALIZER_DECLARE(uint32_t, true);
	TYPEDESCINITIALIZER_DECLARE(uint64_t, true);
	TYPEDESCINITIALIZER_DECLARE(float, true);
	TYPEDESCINITIALIZER_DECLARE(double, true);

	#undef TYPEDESCINITIALIZER_DECLARE

	// === Static Function (WIP) ===
	// @TODO: refactor this so that we can have a full return type + argument type list at construction time, so that we can generate a unique name
    // @TODO: add a special initializer for function type desc so that we can manage its responsibility correctly
    class MIRROR_API StaticFunctionTypeDesc : public TypeDesc
    {
    public:
        StaticFunctionTypeDesc(VirtualTypeWrapper* _virtualTypeWrapper)
                : TypeDesc(Type_StaticFunction, "StaticFunction", _virtualTypeWrapper)
        {
        }

        template <typename T>
        void setReturnType()
        {
            m_returnType = TypeDescGetter<T>::Get()->getTypeID();
        }

        template <typename T>
        void addArgument() {
            TypeDesc* typeDesc = TypeDescGetter<T>::Get();
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
		static void Unpile(class StaticFunctionTypeDesc* _function)
		{
			using ArgumentType = FunctionArgument_T<I, F>;
			_function->addArgument<ArgumentType>();
			FunctionArgumentsUnpiler<F, T, I + 1u, Top>::Unpile(_function);
		}
	};

	template <typename F, typename T, T I>
	struct FunctionArgumentsUnpiler<F, T, I, I>
	{
		static void Unpile(class StaticFunctionTypeDesc* _function) {}
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
			assert(member->getType() == TypeDescGetter<Argument_T>::Get());

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
	struct TypeDescGetter<T, void, void, void, std::enable_if_t<std::is_function<T>::value>>
	{
		static TypeDesc* Get()
		{
			TypeDesc* typeDesc = GetTypeSet().findTypeByID(GetTypeID<T>());
			if (typeDesc == nullptr)
			{
				using function_pointer_t = typename std::add_pointer<T>::type;

				StaticFunctionTypeDesc* staticFunctionTypeDesc = new StaticFunctionTypeDesc(new TVirtualTypeWrapper<T, false>());
				GetTypeSet().addType(staticFunctionTypeDesc);

				// Return type
				using ReturnType = typename FunctionTraits<function_pointer_t>::result;
				staticFunctionTypeDesc->setReturnType<ReturnType>();

				// Arguments
				constexpr size_t argumentsCount = std::tuple_size<FunctionArguments_T<function_pointer_t>>::value;
				FunctionArgumentsUnpiler<function_pointer_t, std::size_t, 0, argumentsCount>::Unpile(staticFunctionTypeDesc);

				typeDesc = staticFunctionTypeDesc;
			}
			return typeDesc;
		}
	};

	template <typename F>
	StaticFunctionTypeDesc* GetStaticFunctionType(F* _function)
	{
		return static_cast<StaticFunctionTypeDesc*>(TypeDescGetter<F>::Get());
	}

} // namespace mirror

//*****************************************************************************
// Implementation
//*****************************************************************************

#ifdef MIRROR_IMPLEMENTATION

#include <cstring>

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

	//-----------------------------------------------------------------------------
	// Global Functions
	//-----------------------------------------------------------------------------
	TypeDesc* FindTypeByName(const char* _name)
	{
		return GetTypeSet().findTypeByName(_name);
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

	TypeSet* g_typeSetPtr = nullptr;


	//-----------------------------------------------------------------------------
	// TypeDesc
	//-----------------------------------------------------------------------------

	Type TypeDesc::getType() const
	{
		return m_type;
	}

	const char* TypeDesc::getName() const
	{
		return m_name;
	}

	TypeID TypeDesc::getTypeID() const
	{
		return m_virtualTypeWrapper->getTypeID();
	}

	size_t TypeDesc::getSize() const
	{
		return m_virtualTypeWrapper->getSize();
	}

	TypeDesc::TypeDesc(Type _type, const char* _name, VirtualTypeWrapper* _virtualTypeWrapper)
		: m_type(_type)
		, m_virtualTypeWrapper(_virtualTypeWrapper)
	{
		ALLOCATE_AND_COPY_STRING(m_name, _name);
	}

	bool TypeDesc::hasFactory() const
	{
		return m_virtualTypeWrapper->hasFactory();
	}

	void* TypeDesc::instantiate() const
	{
		return m_virtualTypeWrapper->instantiate();
	}

	TypeDesc::~TypeDesc()
	{
		if (m_virtualTypeWrapper) delete m_virtualTypeWrapper;
		free(m_name);
	}

	void TypeDesc::setName(const char* _name)
	{
		free(m_name);
		ALLOCATE_AND_COPY_STRING(m_name, _name);
	}

	//-----------------------------------------------------------------------------
	// Specialized Type Descs
	//-----------------------------------------------------------------------------

	// --- Class
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

	const std::set<Class*>& Class::getParents() const
	{
		return m_parents;
	}

	const std::set<Class*>& Class::getChildren() const
	{
		return m_children;
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

	const char* ClassMember::getName() const
	{
		return m_name;
	}

	Class* ClassMember::getClass() const
	{
		return m_class;
	}

	size_t ClassMember::getOffset() const
	{
		return m_offset;
	}

	TypeDesc* ClassMember::getType() const
	{
		return GetTypeSet().findTypeByID(m_type);
	}

	void* ClassMember::getInstanceMemberPointer(void* _classInstancePointer) const
	{
		return reinterpret_cast<uint8_t*>(_classInstancePointer) + m_offset;
	}

	const MetaDataSet& ClassMember::GetMetaDataSet() const
	{
		return m_metaDataSet;
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

	// --- Enum
	const std::vector<EnumValue*>& Enum::getValues() const
	{
		return m_values;
	}

	TypeDesc* Enum::getSubType() const
	{
		return m_subType;
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

	Enum::Enum(const char* _name, VirtualTypeWrapper* _virtualTypeWrapper, TypeDesc* _subType)
		: TypeDesc(Type_Enum, _name, _virtualTypeWrapper)
		, m_subType(_subType ? _subType : TypeDescGetter<int>::Get())
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
		ALLOCATE_AND_COPY_STRING(m_name, _name)
	}

	EnumValue::~EnumValue()
	{
		free(m_name);
	}

	// --- Pointer
	TypeDesc* PointerTypeDesc::getSubType() const
	{
		return GetTypeSet().findTypeByID(m_subType);
	}

	PointerTypeDesc::PointerTypeDesc(TypeID _subType, VirtualTypeWrapper* _virtualTypeWrapper)
		: TypeDesc(Type_Pointer, "", _virtualTypeWrapper)
		, m_subType(_subType)
	{
		setName((std::string("pointer_") + GetTypeSet().findTypeByID(_subType)->getName()).c_str());
	}

	// --- Fixed Size Array
	TypeDesc* FixedSizeArrayTypeDesc::getSubType() const
	{
		return GetTypeSet().findTypeByID(m_subType);
	}

	FixedSizeArrayTypeDesc::FixedSizeArrayTypeDesc(TypeID _subType, size_t _elementCount, VirtualTypeWrapper* _virtualTypeWrapper)
		: TypeDesc(Type_FixedSizeArray, "fixed_size_array", _virtualTypeWrapper)
		, m_subType(_subType)
		, m_elementCount(_elementCount)
	{

	}

	//-----------------------------------------------------------------------------
	// Types Implementation
	//-----------------------------------------------------------------------------

	#define __MIRROR_TYPEDESCINITIALIZER_DEFINE(_type, _hasFactory, _mirrorType) ::mirror::TypeDescInitializer<_type, _hasFactory> g_##_type##TypeInitializer(_mirrorType, #_type)

	__MIRROR_TYPEDESCINITIALIZER_DEFINE(void, false, Type_void);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(bool, true, Type_bool);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(char, true, Type_char);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(int8_t, true, Type_int8);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(int16_t, true, Type_int16);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(int32_t, true, Type_int32);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(int64_t, true, Type_int64);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(uint8_t, true, Type_uint8);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(uint16_t, true, Type_uint16);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(uint32_t, true, Type_uint32);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(uint64_t, true, Type_uint64);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(float, true, Type_float);
	__MIRROR_TYPEDESCINITIALIZER_DEFINE(double, true, Type_double);

	#undef __MIRROR_TYPEDESCINITIALIZER_DEFINE

} // namespace mirror

#endif