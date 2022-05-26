#pragma once


// Dll import/export markup
#ifndef MIRROR_API
	#ifdef MIRROR_EXPORT
		#ifdef _WIN32
	    	#define MIRROR_API __declspec(dllexport)
		#else
	    	#define MIRROR_API // Not done yet
		#endif
	#elif MIRROR_IMPORT
		#ifdef _WIN32
	    	#define MIRROR_API __declspec(dllimport)
		#else
	    	#define MIRROR_API // Not done yet
		#endif
	#else
	    #define MIRROR_API	
	#endif
#endif


#pragma warning( push )
#pragma warning( disable : 4251 ) // deactivate '...': class '...' needs to have dll-interface to be used by clients of class '...' for std containers until I make my own some day. The code will be duplicated but it should be fine

#include "mirror_base.h"
#include "mirror_std.h"
#include "mirror_macros.h"

#pragma warning( pop )
