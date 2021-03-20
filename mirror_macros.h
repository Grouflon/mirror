#pragma once

#include <mirror_base.h>

#define MIRROR_CLASS(_class, ...)\
public:\
	virtual ::mirror::Class* getClass() const { return _class::GetClass(); }\
	__MIRROR_CLASS_CONSTRUCTION(_class, __VA_ARGS__)


#define MIRROR_CLASS_NOVIRTUAL(_class, ...)\
public:\
	::mirror::Class* getClass() const { return _class::GetClass(); }\
	__MIRROR_CLASS_CONSTRUCTION(_class, __VA_ARGS__)

#define __MIRROR_CLASS_CONSTRUCTION(_class, ...)\
	static inline ::mirror::TypeDescInitializer<_class> __MirrorInitializer;\
	static ::mirror::Class* GetClass()\
	{\
		using classType = _class; \
		static ::mirror::Class* s_class = nullptr;\
		if (!s_class)\
		{\
			{\
				const char* metaDataString = #__VA_ARGS__##"";\
				mirror::MetaDataSet metaDataSet(metaDataString);\
				mirror::VirtualTypeWrapper* virtualTypeWrapper = nullptr;\
				if (metaDataSet.findMetaData("Factory"))\
				{\
					virtualTypeWrapper = new mirror::TVirtualTypeWrapper<classType, true, true>();\
				}\
				else\
				{\
					virtualTypeWrapper = new mirror::TVirtualTypeWrapper<classType, false, true>();\
				}\
				s_class = new ::mirror::Class(#_class, virtualTypeWrapper, metaDataSet);\
			}\
			::mirror::g_typeSet.addType(s_class);\
			char fakePrototype[sizeof(_class)] = {};\
			_class* prototypePtr = reinterpret_cast<_class*>(fakePrototype);\
			__MIRROR_CLASS_CONTENT

#define __MIRROR_CLASS_CONTENT(...)\
			__VA_ARGS__\
		}\
		return s_class;\
	}

#define MIRROR_MEMBER(_memberName)\
	{\
		size_t offset = reinterpret_cast<size_t>(&(prototypePtr->_memberName)) - reinterpret_cast<size_t>(prototypePtr);\
		::mirror::TypeDesc* type = ::mirror::GetTypeDesc(prototypePtr->_memberName);\
		const char* memberName = #_memberName;\
		__MIRROR_MEMBER_CONTENT

#define __MIRROR_MEMBER_CONTENT(...)\
		const char* metaDataString = #__VA_ARGS__##"";\
		::mirror::ClassMember* classMember = new ::mirror::ClassMember(memberName, offset, type, metaDataString);\
		s_class->addMember(classMember);\
	}

#define MIRROR_PARENT(_parentClass)\
	{\
		s_class->addParent(::mirror::GetClass<_parentClass>());\
	}

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
		::mirror::g_typeSet.addType(s_enum);\
	}\
	return s_enum;\
}};

#define _MIRROR_ENUM_VALUE_CONTENT(...)
