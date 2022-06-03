# mirror
mirror is a lightweight C++ reflection library that aims at providing a simple and expressive way of adding reflection/introspection to a C++ project.

## Manifesto (enforced in this library, but actually applies to any coding project)
- Simple ideas should be simple to write.
- If you are not using it, it should not do or cost anything.
- Redundant declaration is evil and should be avoided at all cost.
- A puppy dies each time a C++ developer writes yet another piece of non-generic serialization code in 2022.

## How to install
This library is loosely inspired from [single header stb libraries](https://github.com/nothings/stb).
All the library is contained in the header file and the .h that you should include as any header when you want to use the library.
In addition to that, you should select exactly one C++ source file that actually instantiates the code. This file should define the following macro before including the header
```C++
#define MIRROR_IMPLEMENTATION
#include <mirror.h>
```

## How to declare
- Include '<mirror.h>' in the header of the class you want to reflect.
- Declare your class as reflected inside its scope by using the `MIRROR_CLASS(<className>)(...)` macro (note that this macro leaves the accessibility of your class public)
- Inside the second pair of parenthesis, declare the members that should be reflected using the `MIRROR_MEMBER(<memberName>)(<meta-data>)` macro
- In the second pair of `MIRROR_MEMBER` parenthesis, you can declare key / value metadata pairs to your member separated by `,` (value is optional).
- You can declare inheritance on other mirrored classes by using the `MIRROR_PARENT(<parentClassName>)` macro. Note that mirror supports multiple inheritance.
- Here is an example declaration:

```C++
#include <mirror.h>

class MyClass : public MyParent
{
	bool a;
	float b;
	void** c;
	MyStruct*** d;
	std::vector<std::vector<std::string>> e;
	std::string f;
	Enum g;
	bool (*e)(int, float, char*);

	MIRROR_CLASS(MyClass)
	(
		MIRROR_PARENT(MyParent)

		MIRROR_MEMBER(a)()
		MIRROR_MEMBER(b)(Transient)
		MIRROR_MEMBER(c)(ShowOnGUI, Width = 100)
		MIRROR_MEMBER(d)(Version = 12)
		MIRROR_MEMBER(e)()
		MIRROR_MEMBER(f)(Deprecated)
		MIRROR_MEMBER(g)()
		MIRROR_MEMBER(e)()
	)
};

struct MyStruct
{
	int a;

	MIRROR_CLASS_NOVIRTUAL(MyStruct) // Declare your classes that way if you don't want to make it virtual
	(
		MIRROR_MEMBER(a)()
	)
};

enum MyEnum
{
	MyEnum_1,
	MyEnum_2,
	MyEnum_3 = 5,
};

MIRROR_ENUM(MyEnum)
(
	MIRROR_ENUM_VALUE(MyEnum_1)()
	MIRROR_ENUM_VALUE(MyEnum_2)()
	MIRROR_ENUM_VALUE(MyEnum_3)()
)
```

## How to use
- Any reflected class gains a public `GetClass()` static function that allow to iterate through reflected members, access their types and find their address on given instances. You can also access the reflected type one any type from the oustide with the function `mirror::GetTypeDesc<T>()` or `mirror::GetTypeDesc(myVariable)`
- Classes inheritance schemes can be checked at runtime by using the `Class::isChildOf` method.
- A cheap dynamic cast is also available by using the static `mirror::Cast<TargetType>(SourceType)` method.
- You can access a static function return and arguments types by calling `mirror::GetStaticFunctionType()` on a static function pointer.
- You can access an enum type, convert value to string, string to value and access a list of the enum's values with the templated method `mirror::GetEnum<MyEnum>()`.

## Contributing
mirror is till a very early prototype, you can contribute on providing me feedback and use cases, that would actually help a lot.
