#pragma once

#ifdef MIRROR_EXPORT
#define MIRROR_API __declspec(dllexport)
#else
#define MIRROR_API __declspec(dllimport)
#endif

#include <mirror_base.h>
#include <mirror_std.h>
#include <mirror_macros.h>
