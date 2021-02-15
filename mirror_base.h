#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <set>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <assert.h>

#include <mirror_types.h>

namespace mirror
{
	class Class;
	class TypeDesc;
	typedef void* (*FactoryFunctionPtr)();

	struct MetaData
	{
		MetaData(const char* _name, const char* _data);

		const char* getName() const;

		bool asBool() const;
		int asInt() const;
		float asFloat() const;
		const char* asString() const;

	private:
		std::string m_name;
		std::string m_data;
	};

	struct MetaDataSet
	{
		MetaDataSet(const char* _metaDataString);
		const MetaData* findMetaData(const char* _key) const;

	private:
		std::unordered_map<uint32_t, MetaData> m_metaData;
	};

	template <typename T> void* SimpleFactory() { return new T; }

	class TypeDesc
	{
	public:
		TypeDesc(Type _type, const char* _name, size_t _typeHash, size_t _size, FactoryFunctionPtr _factory = nullptr)
			: m_type(_type)
			, m_name(_name)
			, m_typeHash(_typeHash)
			, m_size(_size)
			, m_factory(_factory)
		{}

		// NOTE(Remi|2020/04/26): virtual specifier is not needed, but is added to allow the debugger to show inherited types
		virtual ~TypeDesc() {}

		Type getType() const { return m_type; }
		const char* getName() const { return m_name.c_str(); }
		size_t getTypeHash() const { return m_typeHash; }
		size_t getSize() const { return m_size; }

		// @TODO(2021/02/15|Remi): Allow the user to choose their allocator
		void setFactory(FactoryFunctionPtr _factory);
		bool hasFactory() const;
		void* instantiate() const;

	private:
		Type m_type = Type_none;
		std::string m_name;
		size_t m_typeHash = 0;
		size_t m_size = 0;
		FactoryFunctionPtr m_factory = nullptr;
	};

	class PointerTypeDesc : public TypeDesc
	{
	public:
		PointerTypeDesc(size_t _typeHash, TypeDesc* _subType, FactoryFunctionPtr _factory = nullptr);

		TypeDesc* getSubType() const { return m_subType; }

	private:
		TypeDesc* m_subType;
	};

	class FixedSizeArrayTypeDesc : public TypeDesc
	{
	public:
		FixedSizeArrayTypeDesc(size_t _typeHash, TypeDesc* _subType, size_t _elementCount, size_t _size);

		TypeDesc* getSubType() const { return m_subType; }
		size_t getElementCount() const { return m_elementCount; }

	private:
		TypeDesc* m_subType;
		size_t m_elementCount;
	};

	class ClassMember
	{
		friend class Class;

	public:
		ClassMember(const char* _name, size_t _offset, TypeDesc* _type, const char* _metaDataString);

		const char* getName() const	{ return m_name.c_str(); }
		Class* getClass() const { return m_class; }
		size_t getOffset() const { return m_offset;	}
		TypeDesc* getType() const { return m_type; }

		void* getInstanceMemberPointer(void* _classInstancePointer) const;
		const MetaDataSet& GetMetaDataSet() const { return m_metaDataSet; }

	private:
		Class* m_class = nullptr;
		std::string m_name;
		size_t m_offset;
		TypeDesc* m_type = nullptr;
		MetaDataSet m_metaDataSet;
	};

	class Class : public TypeDesc
	{
	public:
		Class(const char* _name, size_t _typeHash, size_t _size);
		virtual ~Class();

		void getMembers(std::vector<ClassMember*>& _outMemberList, bool _includeInheritedMembers = true) const;
		ClassMember* findMemberByName(const char* _name, bool _includeInheritedMembers = true) const;

		const std::set<Class*>& getParents() const { return m_parents; }
		const std::set<Class*>& getChildren() const { return m_children; }

		bool isChildOf(const Class* _class, bool _checkSelf = true) const;

		void addMember(ClassMember* _member);
		void addParent(Class* _parent);

	private:
		std::set<Class*> m_parents;
		std::set<Class*> m_children;
		std::vector<ClassMember*> m_members;
		std::unordered_map<uint32_t, ClassMember*> m_membersByName;
	};

	class EnumValue
	{
	public:
		EnumValue(const char* _name, int64_t _value);
		
		const char* getName() const;
		int64_t getValue() const;

	private:
		std::string m_name;
		int64_t m_value;
	};

	class Enum : public TypeDesc
	{
	public:
		Enum(const char* _name, size_t _typeHash, TypeDesc* _subType = nullptr);

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

	class TypeSet
	{
	public:
		~TypeSet();

		TypeDesc* findTypeByTypeHash(size_t _typeHash);

		void addType(TypeDesc* _type);
		void removeType(TypeDesc* _type);

		const std::set<TypeDesc*>& getTypes() const;

	private:
		std::set<TypeDesc*> m_types;
		std::unordered_map<size_t, TypeDesc*> m_typesByTypeHash;
	};

	template <typename T, typename IsArray = void, typename IsPointer = void, typename IsEnum = void, typename IsFunction = void>
	struct TypeDescGetter
	{
		static TypeDesc* Get()
		{
			return T::GetClass();
		}
	};

	template <typename T>
	struct TypeDescGetter<T, std::enable_if_t<std::is_array<T>::value>>
	{
		static TypeDesc* Get()
		{
			using type = typename typename std::remove_extent<T>::type;
			static FixedSizeArrayTypeDesc s_fixedSizeArrayTypeDesc(typeid(T).hash_code(), TypeDescGetter<type>::Get(), std::extent<T>::value, sizeof(T));
			return &s_fixedSizeArrayTypeDesc;
		}
	};

	template <typename T>
	struct TypeDescGetter<T, void, std::enable_if_t<std::is_pointer<T>::value>>
	{
		static TypeDesc* Get()
		{
			using type = typename std::remove_pointer<T>::type;
			static auto s_factory = []() -> void* { return new T(); };
			static PointerTypeDesc s_pointerTypeDesc(typeid(T).hash_code(), TypeDescGetter<type>::Get(), s_factory);
			return &s_pointerTypeDesc;
		}
	};

	template <> struct TypeDescGetter<void> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_void, "void", typeid(void).hash_code(), 0, nullptr); return &s_typeDesc; } };
	template <> struct TypeDescGetter<bool> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_bool, "bool", typeid(bool).hash_code(), sizeof(bool), &SimpleFactory<bool>); return &s_typeDesc; } };
	template <> struct TypeDescGetter<char> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_char, "char", typeid(char).hash_code(), sizeof(char), &SimpleFactory<char>); return &s_typeDesc; } };
	template <> struct TypeDescGetter<int8_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_int8, "int8", typeid(int8_t).hash_code(), sizeof(int8_t), &SimpleFactory<int8_t>); return &s_typeDesc; } };
	template <> struct TypeDescGetter<int16_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_int16, "int16", typeid(int16_t).hash_code(), sizeof(int16_t), &SimpleFactory<int16_t>); return &s_typeDesc; } };
	template <> struct TypeDescGetter<int32_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_int32, "int32", typeid(int32_t).hash_code(), sizeof(int32_t), &SimpleFactory<int32_t>); return &s_typeDesc; } };
	template <> struct TypeDescGetter<int64_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_int64, "int64", typeid(int64_t).hash_code(), sizeof(int64_t), &SimpleFactory<int64_t>); return &s_typeDesc; } };
	template <> struct TypeDescGetter<uint8_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_uint8, "uint8", typeid(uint8_t).hash_code(), sizeof(uint8_t), &SimpleFactory<uint8_t>); return &s_typeDesc; } };
	template <> struct TypeDescGetter<uint16_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_uint16, "uint16", typeid(uint16_t).hash_code(), sizeof(uint16_t), &SimpleFactory<uint16_t>); return &s_typeDesc; } };
	template <> struct TypeDescGetter<uint32_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_uint32, "uint32", typeid(uint32_t).hash_code(), sizeof(uint32_t), &SimpleFactory<uint32_t>); return &s_typeDesc; } };
	template <> struct TypeDescGetter<uint64_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_uint64, "uint64", typeid(uint64_t).hash_code(), sizeof(uint64_t), &SimpleFactory<uint64_t>); return &s_typeDesc; } };
	template <> struct TypeDescGetter<float> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_float, "float", typeid(float).hash_code(), sizeof(float), &SimpleFactory<float>); return &s_typeDesc; } };
	template <> struct TypeDescGetter<double> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_double, "double", typeid(double).hash_code(), sizeof(double), &SimpleFactory<double>); return &s_typeDesc; } };

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

	template <typename DestType, typename SourceType, typename IsDestLastPointer = void, typename IsSourceLastPointer = void>
	struct CastClassesUnpiler
	{
		static_assert(!std::is_pointer<DestType>::value, "Mismatching pointer count between cast source and cast destination (DestType is not a pointer).");
		static_assert(!std::is_pointer<SourceType>::value, "Mismatching pointer count between cast source and cast destination (SourceType is not a pointer).");

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

	class StaticFunction : public TypeDesc
	{
	public:
		StaticFunction(size_t _typeHash)
			: TypeDesc(Type_StaticFunction, "StaticFunction", _typeHash, sizeof(void*))
		{
		}

		template <typename T>
		void setReturnType()
		{
			m_returnType = TypeDescGetter<T>::Get();
		}

		template <typename T>
		void addArgument() {
			TypeDesc* typeDesc = TypeDescGetter<T>::Get();
			m_argumentTypes.push_back(typeDesc);
		}

	private:
		TypeDesc* m_returnType = nullptr;
		std::vector<TypeDesc*> m_argumentTypes;
	};

	template <typename T>
	struct TypeDescGetter<T, void, void, void, std::enable_if_t<std::is_function<T>::value>>
	{
		static TypeDesc* Get()
		{
			static StaticFunction* s_staticFunction = nullptr;
			if (s_staticFunction == nullptr)
			{
				size_t typeHash = typeid(T).hash_code();
				s_staticFunction = new StaticFunction(typeHash);

				using function_pointer_t = typename std::add_pointer<T>::type;

				// Return type
				using ReturnType = typename FunctionTraits<function_pointer_t>::result;
				s_staticFunction->setReturnType<ReturnType>();

				// Arguments
				constexpr size_t argumentsCount = std::tuple_size<FunctionArguments_T<function_pointer_t>>::value;
				FunctionArgumentsUnpiler<function_pointer_t, std::size_t, 0, argumentsCount>::Unpile(s_staticFunction);
			}
			return s_staticFunction;
		}
	};

	template <typename F>
	StaticFunction* GetStaticFunctionType(F* _function)
	{
		return static_cast<StaticFunction*>(TypeDescGetter<F>::Get());
	}

	uint32_t Hash32(const void* _data, size_t _size);
	uint32_t HashCString(const char* _str);

	extern TypeSet g_typeSet;

}

