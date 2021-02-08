#pragma once

namespace mirror
{
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

		Type_std_string,
		Type_std_vector,

		Type_Class,

		Type_Pointer,
		Type_FixedSizeArray,

		Type_StaticFunction,

		Type_COUNT,
	};
}
