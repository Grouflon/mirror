#include "mirror_std.h"

namespace mirror
{
	const mirror::TypeDesc* GetTypeDesc(const std::string&) { static TypeDesc s_typeDesc = TypeDesc(Type_std_string); return &s_typeDesc; }

	StdVectorTypeDesc::StdVectorTypeDesc(size_t _typeHash)
		: TypeDesc(Type_std_vector)
	{

	}

}