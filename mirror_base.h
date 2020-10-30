#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <set>
#include <string>
#include <type_traits>
#include <typeinfo>

#include <mirror_types.h>

namespace mirror
{
	class Class;
	class TypeDesc;

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
		PointerTypeDesc(TypeDesc* _subType);

		TypeDesc* getSubType() const { return m_subType; }

	private:
		TypeDesc* m_subType;
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

		MetaData* getMetaData(const char* _key) const;

	private:
		Class* m_class = nullptr;
		std::string m_name;
		size_t m_offset;
		TypeDesc* m_type = nullptr;
		std::unordered_map<uint32_t, MetaData> m_metaData;
	};


	class Class : public TypeDesc
	{
	public:
		Class(const char* _name, size_t _typeHash);
		virtual ~Class();

		void getMembers(std::vector<ClassMember*>& _outMemberList, bool _includeInheritedMembers = true) const;
		const char* getName() const { return m_name.c_str(); }
		size_t getTypeHash() const { return m_typeHash; }

		const std::set<Class*>& getParents() const { return m_parents; }
		const std::set<Class*>& getChildren() const { return m_children; }

		bool isChildOf(const Class* _class, bool _checkSelf = true) const;

		void addMember(ClassMember* _member);
		void addParent(Class* _parent);

	private:
		std::set<Class*> m_parents;
		std::set<Class*> m_children;
		std::vector<ClassMember*> m_members;
		size_t m_typeHash;
		std::string m_name;
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
		static TypeDesc* Get()
		{
			return T::GetClass();
		}
	};

	template <typename T>
	struct TypeDescGetter<T, std::enable_if_t<std::is_pointer<T>::value>>
	{
		static TypeDesc* Get()
		{
			using type = typename std::remove_pointer<T>::type;
			static PointerTypeDesc s_pointerTypeDesc(TypeDescGetter<type>::Get()); return &s_pointerTypeDesc;
		}
	};

	template <> struct TypeDescGetter<void> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_void); return &s_typeDesc; } };
	template <> struct TypeDescGetter<bool> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_bool); return &s_typeDesc; } };
	template <> struct TypeDescGetter<char> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_char); return &s_typeDesc; } };
	template <> struct TypeDescGetter<int8_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_int8); return &s_typeDesc; } };
	template <> struct TypeDescGetter<int16_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_int16); return &s_typeDesc; } };
	template <> struct TypeDescGetter<int32_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_int32); return &s_typeDesc; } };
	template <> struct TypeDescGetter<int64_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_int64); return &s_typeDesc; } };
	template <> struct TypeDescGetter<uint8_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_uint8); return &s_typeDesc; } };
	template <> struct TypeDescGetter<uint16_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_uint16); return &s_typeDesc; } };
	template <> struct TypeDescGetter<uint32_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_uint32); return &s_typeDesc; } };
	template <> struct TypeDescGetter<uint64_t> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_uint64); return &s_typeDesc; } };
	template <> struct TypeDescGetter<float> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_float); return &s_typeDesc; } };
	template <> struct TypeDescGetter<double> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_double); return &s_typeDesc; } };

	template <typename T>
	struct TypeDescGetter<T, void, std::enable_if_t<std::is_enum<T>::value>>
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
	TypeDesc* GetTypeDesc(T) { return TypeDescGetter<T>::Get(); }

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
	struct function_traits {}; /// #1

	template<typename R, typename... Args>
	struct function_traits<R(*)(Args...)> {
		using result = R;
		using args = std::tuple<Args...>;
	};

	template<typename F>
	using function_arguments_t = typename function_traits<F>::args;


	template <std::size_t N, typename F>
	using function_argument_t = typename std::tuple_element<N, function_arguments_t<F>>::type;

	template <typename T, T Begin,  class Func, T ...Is>
	constexpr void static_for_impl( Func &&f, std::integer_sequence<T, Is...> )
	{
		( f( std::integral_constant<T, Begin + Is>{ } ),... );
	}

	template <typename T, T Begin, T End, class Func >
	constexpr void static_for( Func &&f )
	{
		static_for_impl<T, Begin>( std::forward<Func>(f), std::make_integer_sequence<T, End - Begin>{ } );
	}

	template <class Tuple>
	constexpr std::size_t tuple_size( const Tuple & )
	{
		return std::tuple_size<Tuple>::value;
	}

	template <typename F>
    class Function : public TypeDesc
    {
    public:

        Function(F nativeFunction, const char *_name, size_t _typeHash):
                TypeDesc(Type_Function),
                m_nativeFunction(nativeFunction),
                m_name(_name),
                m_typeHash(_typeHash)
        {
        }

        void setReturnType(const TypeDesc* t){ m_returnType = t; }
        const TypeDesc* getReturnType()const { return m_returnType; }
        const char* getName() const { return m_name.c_str(); }
        F getNativeFunction() const { return m_nativeFunction; }
        size_t getTypeHash() const { return m_typeHash; }

        void getArgTypes(std::vector<const TypeDesc*> &_outFunctionArgTypes) const {
            _outFunctionArgTypes.insert(_outFunctionArgTypes.end(), m_argTypes.begin(), m_argTypes.end());
        }

        void addArg(const TypeDesc* _arg) {
            m_argTypes.push_back(_arg);
        }

    private:
        const TypeDesc* m_returnType;
        std::vector<const TypeDesc*> m_argTypes;
        size_t m_typeHash;
        std::string m_name;
        F m_nativeFunction;
    };

    template <typename F>
    static Function<F>* CreateFunctionInstance(const char* _name, F _function)
    {
        auto newFunction = new mirror::Function<F>( _function, _name, typeid(_function).hash_code());

        using R = typename mirror::function_traits<F>::result;
        newFunction->setReturnType(mirror::TypeDescGetter<R>::Get() );

        constexpr size_t argCount = std::tuple_size<mirror::function_arguments_t<F>>::value;

        mirror::static_for<std::size_t, 0, argCount>([&](auto eachArgIndex)
                                                     {
                                                         using each_arg_T = mirror::function_argument_t<eachArgIndex, F>;
                                                         auto typeDesc = mirror::TypeDescGetter<each_arg_T>().Get();
                                                         newFunction->addArg(typeDesc);
                                                     });

        return newFunction;
    }

	uint32_t Hash32(const void* _data, size_t _size);
	uint32_t HashCString(const char* _str);

	extern ClassSet	g_classSet;
}
