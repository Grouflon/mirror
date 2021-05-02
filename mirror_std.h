#pragma once

#include "mirror_base.h"

namespace mirror
{
	class StdVectorTypeDesc : public TypeDesc
	{
	public:

		StdVectorTypeDesc(VirtualTypeWrapper* _virtualTypeWrapper) : TypeDesc(Type_std_vector, "std::vector", _virtualTypeWrapper) {}

		virtual void instanceResize(void* _instance, size_t _size) const = 0;
		virtual size_t instanceSize(void* _instance) const = 0;
		virtual void* instanceGetDataPointerAt(void* _instance, size_t _index) const = 0;
		TypeDesc* getSubType() const { return m_subType; }

	protected:
		TypeDesc* m_subType = nullptr;
	};
	template <typename T>
	class TStdVectorTypeDesc : public StdVectorTypeDesc
	{
	public:
		TStdVectorTypeDesc();

		// This class is used as a proxy to access vector's methods without knowing its type.
		// More methods can be added if needed
		virtual void instanceResize(void* _instance, size_t _size) const override;
		virtual size_t instanceSize(void* _instance) const override;
		virtual void* instanceGetDataPointerAt(void* _instance, size_t _index) const override;
	};

	template <> struct TypeDescGetter<std::string> { static TypeDesc* Get() { static TypeDesc s_typeDesc = TypeDesc(Type_std_string, "std::string", new TVirtualTypeWrapper<std::string, true>()); return &s_typeDesc; } };
	template <typename T> struct TypeDescGetter<std::vector<T>> { static TypeDesc* Get() { static TStdVectorTypeDesc<T> s_typeDesc(new TVirtualTypeWrapper<std::string, true>()); return &s_typeDesc; } };
}

// INL
template <typename T>
mirror::TStdVectorTypeDesc<T>::TStdVectorTypeDesc()
{
	m_subType = GetTypeDesc(T());
}

template <typename T>
void* mirror::TStdVectorTypeDesc<T>::instanceGetDataPointerAt(void* _instance, size_t _index) const
{
	return reinterpret_cast<std::vector<T>*>(_instance)->data() + _index;
}

template <typename T>
size_t mirror::TStdVectorTypeDesc<T>::instanceSize(void* _instance) const
{
	return reinterpret_cast<std::vector<T>*>(_instance)->size();
}

template <typename T>
void mirror::TStdVectorTypeDesc<T>::instanceResize(void* _instance, size_t _size) const
{
	reinterpret_cast<std::vector<T>*>(_instance)->resize(_size);
}
