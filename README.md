# mirror
mirror is a lightweight single header C++ reflection library that aims at providing a simple and expressive way of adding reflection/introspection to a C++ project.

## Manifesto (enforced in this library, but actually applies to any coding project)
- Simple ideas should be simple to write.
- If you are not using it, it should not do or cost anything.
- We should take care of our tools, and design them so that we are happy to use them.
- Having to do redundant declaration consumes our joy of programming. We must work to prevent it.

## How to use
This library is loosely inspired from [single header stb libraries](https://github.com/nothings/stb).
All the library is contained in the header file and the .h that you should include as any header when you want to use the library.
In addition to that, you should select exactly one C++ source file that actually instantiates the code. This file should define the following macro before including the header
```C++
#define MIRROR_IMPLEMENTATION
#include <mirror.h>
```
You should then call `mirror::InitNewTypes()` as soon as you can in your main.cpp.

## Reflecting basic types
### Basic struct/class reflection
Mirror aims to be the less intrusive possible, therefore for the most common use case you don't have to write anything in the header file of your reflected class.
```C++
// .h
namespace MyNamespace {
struct MyStruct
{
	bool a;
	float b;
	void** c;
	MyStruct*** d;
};
}

// .cpp
#include <mirror.h>

// MIRROR_CLASS macro must always be called from the root namespace
// The type given as argument must always be the full path from the root namespace to the type
MIRROR_CLASS(MyNamespace::MyStruct) 
(
	MIRROR_MEMBER(a);
	MIRROR_MEMBER(b);
	MIRROR_MEMBER(c);
	MIRROR_MEMBER(d);
);
```
### Basic enum reflection
```C++
// .h
namespace MyNamespace {
enum MyEnum {
	EnumValue0 = 5,
	EnumValue1 = 12,
	EnumValue2 = 64000
};
enum class MyEnumClass : uint8_t {
	EnumValue0 = 5,
	EnumValue1,
	EnumValue2
};
}
// .cpp
#include <mirror.h>

MIRROR_ENUM(MyNamespace::MyEnum)
(
	MIRROR_ENUM_VALUE(MyNamespace::EnumValue0);
	MIRROR_ENUM_VALUE(MyNamespace::EnumValue1);
	MIRROR_ENUM_VALUE(MyNamespace::EnumValue2);
);
MIRROR_ENUM(MyNamespace::MyEnumClass)
(
	MIRROR_ENUM_VALUE(MyNamespace::MyEnumClass::EnumValue0);
	MIRROR_ENUM_VALUE(MyNamespace::MyEnumClass::EnumValue1);
	MIRROR_ENUM_VALUE(MyNamespace::MyEnumClass::EnumValue2);
);
```

## Using reflection data
```C++
{
	// basic types
	{
		uint8_t u8;
		// GetType and other type accessors work with both template parameter or typed argument
		const mirror::Type* type = mirror::GetType<uint8_t>();
		const mirror::Type* type = mirror::GetType(u8);

		printf("name: %s, size: %d\n", type->getName(), type->getSize());
	}

	// classes/structs
	{
		const mirror::Class* type = mirror::GetClass<MyClass>();
		printf("name: %s, size: %d\n", type->getName(), type->getSize());

		printf("members:\n");
		for (const mirror::ClassMember* member : type->getMembers())
		{
			printf("\t%s : %s\n", member->getName(), member->getType()->getName());
		}
	}

	// enum
	{
		const mirror::Enum* myEnum = mirror::GetEnum<MyEnum>();
		printf("name: %s, size: %d\n", myEnum->getName(), myEnum->getSize());

		printf("values (%d):\n", myEnum->getValues().size());
		for (const mirror::EnumValue* value : myEnum->getValues())
		{
			printf("\t%s: %lld\n", value->getName(), value->getValue());
		}
	}

	// dynamic cast
	{
		MyParent* baseInstance = ...;
		MyClass* instance = mirror::Cast<MyClass*>(baseInstance); // This will return nullptr if baseInstance is not of type MyClass.
	}

	// factory
	{
		const mirror::Class* type = mirror::FindClassByName("MyClass");
		void* instance = type->instantiate(); // This calls the default empty constructor of the type
	}

	// This is only a part of the API. Look at the header file for an exhaustive view of the library's functionalities.
}
```

## Advanced use
### Complex classes
```C++
// .h
#include <mirror.h>

class MyClass : public MyParent
{
	MIRROR_GETCLASS_VIRTUAL(); // Gives your class a virtual getClass() method that will return the correct class of your instance, even when called from the base class. This is required if you want to perform a dynamic cast on your instances.
	MIRROR_FRIEND(); // Allow the reflection of protected and privated members.

private:
	bool a;
	float b;
	void** c;
	MyStruct*** d;
	Enum e;
	bool (*f)(int, float, char*);
};

// .cpp
MIRROR_CLASS(MyClass, SerializeAs = Vector3) // You can add as many key=value metadata pairs as you like after the type argument
(
	MIRROR_PARENT(MyParent); // Declare MyClass as a child of MyParent

	MIRROR_MEMBER(a);
	MIRROR_MEMBER(b, Transient); // Members can have metadata too.
	MIRROR_MEMBER(c, ShowOnGUI, Width = 100);
	MIRROR_MEMBER(d, Version = 12);
	MIRROR_MEMBER(e);
	MIRROR_MEMBER(f, Deprecated);
);
```
### Custom types
You can extend the reflection capabilities of mirror by creating your own custom types. This can be useful for reflecting your own templated types such as containers.
To do this, you must create a file where you'll write your new types and append it to mirror.h by defining the MIRROR_EXTENSION_FILE define as the path to your file in your build system.
#### Type declaration
```C++
// Example for reflecting an Array type
#pragma once

template <typename T> class Array; // Array forward declaration

namespace mirror {
class YAE_API ArrayType : public Type
{
	public:
		Type* getSubType() const { return GetType(m_subType); }

		// Type agnostic methods
		uint32_t getSize(void* _arrayPointer) const { return m_getSizeFunction(_arrayPointer); }
		void setSize(void* _arrayPointer, uint32_t _newSize) const { m_setSizeFunction(_arrayPointer, _newSize); }
		void* getData(void* _arrayPointer) const { return m_getDataFunction(_arrayPointer); }
		void* getDataAt(void* _arrayPointer, uint32_t _index) const { return (char*)getData(_arrayPointer) + _index * getSubType()->getSize(); }
		void erase(void* _arrayPointer, uint32_t _index, uint32_t _count) const { m_eraseFunction(_arrayPointer, _index, _count); }
		void swap(void* _arrayPointer, uint32_t _indexA, uint32_t _indexB) const { m_swapFunction(_arrayPointer, _indexA, _indexB); }

		// internal
		ArrayType(
			TypeID _subType,
			uint32_t(*_getSizeFunction)(void*),
			void(*_setSizeFunction)(void* , uint32_t),
			void*(*_getDataFunction)(void*),
			void(*_eraseFunction)(void*, uint32_t, uint32_t),
			void(*_swapFunction)(void*, uint32_t, uint32_t)
		)
			: Type(TypeInfo_Custom, "")
			, m_subType(_subType)
			, m_getSizeFunction(_getSizeFunction)
			, m_setSizeFunction(_setSizeFunction)
			, m_getDataFunction(_getDataFunction)
			, m_eraseFunction(_eraseFunction)
			, m_swapFunction(_swapFunction)
		{
		}

		virtual void init() override
		{
			// TypeID should not be resolved until the init phase as it is possible that the type does not exists yet.
			// That's why we are only solving the name here at init time
			char buf[512];
			snprintf(buf, sizeof(buf), "Array_%s", AsType(m_subType)->getName());
			setName(buf);
			setCustomTypeName("Array");
		}

		TypeID m_subType = UNDEFINED_TYPEID;
		uint32_t(*m_getSizeFunction)(void*);
		void(*m_setSizeFunction)(void* , uint32_t);
		void*(*m_getDataFunction)(void*);
		void(*m_eraseFunction)(void* , uint32_t, uint32_t);
		void(*m_swapFunction)(void* , uint32_t, uint32_t);
};

template <typename T>
struct CustomTypeFactory<yae::Array<T>>
{
	static Type* Create()
	{
		auto getSize = [](void* _arrayPointer) -> uint32_t
		{
			yae::Array<T>* arrayPointer = (yae::Array<T>*)_arrayPointer;
			return arrayPointer->size();
		};

		auto setSize = [](void* _arrayPointer, uint32_t _newSize)
		{
			yae::Array<T>* arrayPointer = (yae::Array<T>*)_arrayPointer;
			arrayPointer->resize(_newSize);
		};

		auto getData = [](void* _arrayPointer) -> void*
		{
			yae::Array<T>* arrayPointer = (yae::Array<T>*)_arrayPointer;
			return arrayPointer->data();
		};

		auto erase = [](void* _arrayPointer, uint32_t _index, uint32_t _count)
		{
			yae::Array<T>* arrayPointer = (yae::Array<T>*)_arrayPointer;
			arrayPointer->erase(_index, _count);
		};

		auto swap = [](void* _arrayPointer, uint32_t _indexA, uint32_t _indexB)
		{
			yae::Array<T>* arrayPointer = (yae::Array<T>*)_arrayPointer;
			arrayPointer->swap(_indexA, _indexB);
		};

		GetType<T>(); // Ensures that subType reflection is correctly initialized (might not be the case if the subtype is a dynamic type, such as another array)
		return new ArrayType(GetTypeID<T>(), getSize, setSize, getData, erase, swap);
	}
};

} // namespace mirror
```
#### Usage
```C++
{
	const mirror::Type* type = mirror::FindTypeByName("Array_Vector3");
	if (type->isCustomType("Array"))
	{
		const mirror::ArrayType* arrayType = (const mirror::ArrayType*)type;
		void* arrayInstance = ...;

		// Type agnostic access to the array methods
		for (int i = 0; i < arrayType->getSize(arrayInstance); ++i)
		{
			void* vector3Ptr = arrayType->getDataAt(arrayInstance, i);
			...
		}
		arrayType->erase(arrayInstance, 2);
	}
}

```

## Contributing
mirror is still an early prototype, you can contribute on providing me feedback and use cases, that would actually help a lot.

## Roadmap (in random order)
- Multiple constructors
- Function/method reflection
- Enum metadata
- Custom allocators + get rid of std containers

## Licence
The MIT License (MIT)
Copyright © 2023 Rémi Bismuth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
The Software is provided “as is”, without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software or the use or other dealings in the Software.