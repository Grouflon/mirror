#include <cassert>
#include "mirror_types.h"

namespace mirror::TypeHelper {

    const char *GetTypeName(Type t) {
        switch (t) {
            case Type_none: return "none";
            case Type_void: return "void";
            case Type_bool: return "bool";
            case Type_std_string: return "std::string";
            case Type_double: return "double";
            case Type_char: return "char";
            case Type_int8: return "int8_t";
            case Type_int16: return "int16_t";
            case Type_int32: return "int32_t";
            case Type_int64: return "int64_t";
            case Type_uint8: return "uint8_t";
            case Type_uint16: return "uint16_t";
            case Type_uint32: return "uint32_t";
            case Type_uint64: return "uint64_t";
            case Type_float: return "float";
            case Type_std_vector: return "std::vector";
            case Type_Class: return "Class";
            case Type_Pointer: return "void*";
            case Type_Function: return "R(*)(Args...)";
            default: assert(false); // Type not yet implemented;
        }
    }
}

