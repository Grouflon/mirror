#pragma once

#ifdef MIRROR_EXPORT
#define MIRROR_API __declspec(dllexport)
#else
#define MIRROR_API __declspec(dllimport)
#endif

#pragma warning( push )
#pragma warning( disable : 4251 ) // deactivate '...': class '...' needs to have dll-interface to be used by clients of class '...' for std containers until I make my own some day. The code will be duplicated but it should be fine

#include "mirror_base.h"
#include "mirror_std.h"
#include "mirror_macros.h"

#pragma warning( pop )
