#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <set>
#include <type_traits>
#include <typeinfo>
#include <assert.h>

#include "mirror_types.h"

namespace mirror
{
	typedef size_t TypeID;
	const size_t UNDEFINED_TYPEID = 0;

	uint32_t Hash32(const void* _data, size_t _size);
	uint32_t HashCString(const char* _str);

	class Class;
	class TypeDesc;

	struct MIRROR_API MetaData
	{
		MetaData(const char* _name, const char* _data);
		~MetaData();

		const char* getName() const;

		bool asBool() const;
		int asInt() const;
		float asFloat() const;
		const char* asString() const;

	private:
		char* m_name;
		char* m_data;
	};

	struct MIRROR_API MetaDataSet
	{
		MetaDataSet(const char* _metaDataString);
		MetaDataSet(const MetaDataSet& _other);
		const MetaData* findMetaData(const char* _key) const;

	private:
		std::unordered_map<uint32_t, MetaData> m_metaData;
	};

	template <typename T>
	constexpr TypeID GetTypeID()
	{
		return typeid(T).hash_code();
	}

	class MIRROR_API VirtualTypeWrapper
	{
	public:
		TypeID getTypeID() const { return m_typeID; }
		size_t getSize() const { return m_size; }

		virtual bool hasFactory() const { return false; }
		virtual void* instantiate() const { return nullptr; }

		virtual Class* unsafeVirtualGetClass(void* _object) const { return nullptr; }

	protected:
		TypeID m_typeID = UNDEFINED_TYPEID;
		size_t m_size = 0;
	};

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

	class MIRROR_API TypeDesc
	{
	public:
		TypeDesc(Type _type, VirtualTypeWrapper* _virtualTypeWrapper);
		TypeDesc(Type _type, const char* _name, VirtualTypeWrapper* _virtualTypeWrapper);
		// NOTE(Remi|2020/04/26): virtual specifier is not needed, but is added to allow the debugger to show inherited types
		virtual ~TypeDesc();

		Type getType() const { return m_type; }
		const char* getName() const { return m_name; }
		TypeID getTypeID() const { return m_virtualTypeWrapper->getTypeID(); }
		size_t getSize() const { return m_virtualTypeWrapper->getSize(); }

		// @TODO(2021/02/15|Remi): Allow the user to choose their allocator
		bool hasFactory() const;
		void* instantiate() const;

	protected:
		const VirtualTypeWrapper* getVirtualTypeWrapper() const { return m_virtualTypeWrapper; }
		void setName(const char* _name);

	private:
		char* m_name;
		Type m_type = Type_none;
		VirtualTypeWrapper* m_virtualTypeWrapper = nullptr;
	};

	class MIRROR_API PointerTypeDesc : public TypeDesc
	{
	public:
		PointerTypeDesc(TypeID _subType, VirtualTypeWrapper* _virtualTypeWrapper);

		TypeDesc* getSubType() const;

	private:
		TypeID m_subType = UNDEFINED_TYPEID;
	};

	class MIRROR_API FixedSizeArrayTypeDesc : public TypeDesc
	{
	public:
		FixedSizeArrayTypeDesc(TypeID _subType, size_t _elementCount, VirtualTypeWrapper* _virtualTypeWrapper);

		TypeDesc* getSubType() const;
		size_t getElementCount() const { return m_elementCount; }

	private:
		TypeID m_subType = UNDEFINED_TYPEID;
		size_t m_elementCount;
	};

	class MIRROR_API ClassMember
	{
		friend class Class;

	public:
		ClassMember(const char* _name, size_t _offset, TypeID _type, const char* _metaDataString);
		~ClassMember();

		const char* getName() const	{ return m_name; }
		Class* getClass() const { return m_class; }
		size_t getOffset() const { return m_offset;	}
		TypeDesc* getType() const;

		void* getInstanceMemberPointer(void* _classInstancePointer) const;
		const MetaDataSet& GetMetaDataSet() const { return m_metaDataSet; }

	private:
		Class* m_class = nullptr;
		char* m_name;
		size_t m_offset;
		TypeID m_type = UNDEFINED_TYPEID;
		MetaDataSet m_metaDataSet;
	};

	class MIRROR_API Class : public TypeDesc
	{
	public:
		Class(const char* _name, VirtualTypeWrapper* _virtualTypeWrapper, const char* _metaDataString);
		Class(const char* _name, VirtualTypeWrapper* _virtualTypeWrapper, const MetaDataSet& _metaDataSet);
		virtual ~Class();

		void getMembers(std::vector<ClassMember*>& _outMemberList, bool _includeInheritedMembers = true) const;
		ClassMember* findMemberByName(const char* _name, bool _includeInheritedMembers = true) const;

		const std::set<Class*>& getParents() const { return m_parents; }
		const std::set<Class*>& getChildren() const { return m_children; }

		bool isChildOf(const Class* _class, bool _checkSelf = true) const;

		void addMember(ClassMember* _member);
		void addParent(Class* _parent);

		const MetaDataSet& getMetaDataSet() const { return m_metaDataSet; }

		Class* unsafeVirtualGetClass(void* _object) const;

	private:
		std::set<Class*> m_parents;
		std::set<Class*> m_children;
		std::vector<ClassMember*> m_members;
		std::unordered_map<uint32_t, ClassMember*> m_membersByName;
		MetaDataSet m_metaDataSet;
	};

	class MIRROR_API EnumValue
	{
	public:
		EnumValue(const char* _name, int64_t _value);
		~EnumValue();
		
		const char* getName() const;
		int64_t getValue() const;

	private:
		char* m_name;
		int64_t m_value;
	};

	class MIRROR_API Enum : public TypeDesc
	{
	public:
		Enum(const char* _name, VirtualTypeWrapper* _virtualTypeWrapper, TypeDesc* _subType = nullptr);

		template <typename T>
		bool getValueFromString(const char* _string, T& _outValue) const
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
		bool getStringFromValue(T _value, const char*& _outString) const
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

		const std::vector<EnumValue*>& getValues() const;
		void addValue(EnumValue* _value);

		TypeDesc* getSubType() const { return m_subType; }

	private:
		std::vector<EnumValue*> m_values;
		std::unordered_map<size_t, EnumValue*> m_valuesByNameHash;

		TypeDesc* m_subType;
	};

	template<typename T>
	Enum* GetEnum()
	{
		TypeDesc* type = TypeDescGetter<T>::Get();
		if (type->getType() != Type_Enum)
			return nullptr;

		return static_cast<Enum*>(type);
	}

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

	template <typename T, typename IsArray = void, typename IsPointer = void, typename IsEnum = void, typename IsFunction = void>
	struct TypeDescGetter
	{
		static TypeDesc* Get()
		{
			TypeID typeID = GetTypeID<T>();
			return GetTypeSet()->findTypeByID(typeID);
		}
	};

	template <typename T>
	struct TypeDescGetter<T, std::enable_if_t<std::is_array<T>::value>>
	{
		static TypeDesc* Get()
		{
			using type = typename typename std::remove_extent<T>::type;
			static FixedSizeArrayTypeDesc s_fixedSizeArrayTypeDesc(TypeDescGetter<type>::Get(), std::extent<T>::value , new TVirtualTypeWrapper<T>());
			return &s_fixedSizeArrayTypeDesc;
		}
	};

	template <typename T>
	struct TypeDescGetter<T, void, std::enable_if_t<std::is_pointer<T>::value>>
	{
		static TypeDesc* Get()
		{
			static PointerTypeDescInitializer<T> s_pointerTypeDescInitializer;
			/*TypeDesc* typeDesc = GetTypeSet()->findTypeByID(GetTypeID<T>());
			if (!typeDesc)
			{
				using type = typename std::remove_pointer<T>::type;
				PointerTypeDesc* pointerTypeDesc = new PointerTypeDesc(TypeDescGetter<type>::Get()->getTypeID(), new TVirtualTypeWrapper<T>());
				GetTypeSet()->addType(pointerTypeDesc);
				typeDesc = pointerTypeDesc;
			}*/
			//return s_pointerTypeDescInitializer->typeDesc;
			return GetTypeSet()->findTypeByID(GetTypeID<T>());
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

	template <typename T>
	TypeDesc* GetTypeDesc() { return TypeDescGetter<T>::Get(); }

	template <typename T>
	TypeDesc* GetTypeDesc(T&) { return TypeDescGetter<T>::Get(); }

	template <typename T>
	Class* GetClass()
	{ 
		TypeDesc* typeDesc = TypeDescGetter<T>::Get();
		if (typeDesc->getType() == Type_Class)
		{
			return static_cast<Class*>(typeDesc);
		}
		return nullptr;
	}

	TypeDesc* FindTypeByName(const char* _name);

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
			if (destClass->isChildOf(sourceClass) // upcast
				|| sourceClass->isChildOf(destClass)) // downcast
			{
				return reinterpret_cast<DestType>(_o);
			}
		}
		return nullptr;
	}

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

		constexpr size_t argumentsCount = std::tuple_size<FunctionTraits<F>::args>::value;
		assert(argumentsCount == _memberCount);
		FunctionArgumentsFiller<0, argumentsCount>::Fill(_classInstance, _memberNames, _memberCount, arguments);

		return CallFunction(_functionPointer, arguments);
	}

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

	template <typename T>
	struct TypeDescGetter<T, void, void, void, std::enable_if_t<std::is_function<T>::value>>
	{
		static TypeDesc* Get()
		{
			TypeDesc* typeDesc = GetTypeSet()->findTypeByID(GetTypeID<T>());
			if (typeDesc == nullptr)
			{
				using function_pointer_t = typename std::add_pointer<T>::type;

				StaticFunctionTypeDesc* staticFunctionTypeDesc = new StaticFunctionTypeDesc(new TVirtualTypeWrapper<T, false>());
				GetTypeSet()->addType(staticFunctionTypeDesc);

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

	template <typename T, bool HasFactory = true>
	struct TypeDescInitializer
	{
		TypeDescInitializer(Type _type, const char* _name)
		{
			typeDesc = new TypeDesc(_type, _name, new TVirtualTypeWrapper<T, HasFactory>());
			GetTypeSet()->addType(typeDesc);
		}
		~TypeDescInitializer()
		{
			GetTypeSet()->removeType(typeDesc);
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
			GetTypeSet()->addType(clss);
		}
		~ClassInitializer()
		{
			GetTypeSet()->removeType(clss);
			delete clss;
		}
		Class* clss = nullptr;
	};

	template <typename T>
	struct PointerTypeDescInitializer
	{
		PointerTypeDescInitializer()
		{
			using type = typename std::remove_pointer<T>::type;
			typeDesc = new PointerTypeDesc(TypeDescGetter<type>::Get()->getTypeID(), new TVirtualTypeWrapper<T>());
			GetTypeSet()->addType(typeDesc);
		}
		~PointerTypeDescInitializer()
		{
			GetTypeSet()->removeType(typeDesc);
			delete typeDesc;
		}
		PointerTypeDesc* typeDesc = nullptr;
	};

    MIRROR_API TypeSet* GetTypeSet();
	extern TypeSet g_typeSet;


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

#undef TYPEDESC_INITIALIZER
}

