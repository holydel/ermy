#pragma once

#ifdef ERMY_STATIC
#define ERMY_API 
#else
#ifdef ermy_EXPORTS
#define ERMY_API __declspec(dllexport)
#else
#define ERMY_API __declspec(dllimport)
#endif
#endif
