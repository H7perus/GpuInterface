#pragma once

#ifdef _WIN32
	#ifdef GPUI_DLL_EXPORTS
		#define GPUI_DLL_API __declspec(dllexport)
	#else
		#define GPUI_DLL_API __declspec(dllimport)
	#endif
#else
		#define GPUI_DLL_API // Empty for non-Windows platforms
#endif