#pragma once

#include <string>

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

		Type_std_string,
		Type_std_vector,

		/*Type_c_string,
		Type_c_fixedArray,
		Type_c_dynamicArray,*/

		Type_Class,

		Type_Pointer,

		Type_Function,

		Type_COUNT,
	};

	/**
	 * Provide some useful static methods to play with Type enum.
	 */
	namespace TypeHelper {
	    /**
	     * Given a Type, returns its native type as string.
	     * @param t
	     * @return
	     */
        const char *GetTypeName(Type t);

        /* Templates to get the native type of a Type_ */
        template <Type t> struct GetNative { using type = void; };
        template <> struct GetNative<Type_bool> { using type = bool; };
        template <> struct GetNative<Type_double> { using type = double; };
        template <> struct GetNative<Type_std_string> { using type = std::string; };
    }
}
