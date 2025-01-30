#pragma once

// Define platform macros based on the detected platform
#if defined(_WIN32)
#define ERMY_OS_WINDOWS
#elif defined(__ANDROID__)
#define ERMY_OS_ANDROID
#elif defined(__linux__)
#define ERMY_OS_LINUX
#elif defined(__APPLE__)
#include "TargetConditionals.h"
#if TARGET_OS_MAC
#define ERMY_OS_MACOS
#endif
#elif defined(__EMSCRIPTEN__)
#define ERMY_OS_EMSCRIPTEN
#else
#error "Unsupported platform"
#endif

//#define ERMY_OS_WINDOWS
//#define ERMY_OS_LINUX
//#define ERMY_OS_MACOS
//#define ERMY_OS_ANDROID
//#define ERMY_OS_EMSCRIPTEN

#define ERMY_PLATFORM_NONE
//#define ERMY_PLATFORM_STEAM
//#define ERMY_PLATFORM_GOOGLEPLAY
//#define ERMY_PLATFORM_APPLESTORE
//#define ERMY_PLATFORM_HORIZON
//#define ERMY_PLATFORM_WEB

//#define ERMY_GAPI_VULKAN
#define ERMY_GAPI_D3D12
//#define ERMY_GAPI_METAL
//#define ERMY_GAPI_WEBGPU

#define ERMY_DEVICE_PC
//#define ERMY_DEVICE_HEADSET
//#define ERMY_DEVICE_MOBILE
//#define ERMY_DEVICE_HANDHELD

#define ERMY_SOUND_NONE
//#define ERMY_SOUND_MINIAUDIO
//#define ERMY_SOUND_FMOD
//#define ERMY_SOUND_WWISE

#define ERMY_PHYSICS_NONE
//#define ERMY_PHYSICS_BULLET
//#define ERMY_PHYSICS_PHYSX

#define ERMY_XR_NONE
//#define ERMY_XR_OPENXR
//#define ERMY_XR_WEBXR
//#define ERMY_XR_APPLE_REALITY_KIT

#ifdef ERMY_OS_WINDOWS
#include <xmmintrin.h>
#endif

#ifdef ERMY_OS_ANDROID
#include <arm_neon.h>
#endif

namespace ermy
{
	typedef unsigned	long long	u64;
	typedef signed		long long	i64;
	typedef unsigned	int			u32;
	typedef	signed		int			i32;
	typedef unsigned	short		u16;
	typedef	signed		short		i16;
	typedef unsigned	char		u8;
	typedef	signed		char		i8;

	typedef float					f32;
	typedef double					f64;

	//TODO: if supported f16
	typedef	unsigned short			f16;

#ifdef ERMY_OS_WINDOWS
	typedef __m128					f32v4;
#endif

#ifdef ERMY_OS_ANDROID
	typedef float32x4_t				f32v4;
#endif

	unsigned long long operator ""_KB(unsigned long long v);

	unsigned long long operator ""_MB(unsigned long long v);

	unsigned long long operator ""_GB(unsigned long long v);

	struct Handle32
	{
		static constexpr u32 InvalidValue = 0xFFFFFFFFu;

		u32 handle = InvalidValue;

		bool isValid() const
		{
			return handle != InvalidValue;
		}
		u32 Value()
		{
			return handle;
		}
	};

	struct Handle64
	{
		static constexpr u64 InvalidValue = 0xFFFFFFFFFFFFFFFFu;

		u64 handle = InvalidValue;

		bool isValid() const
		{
			return handle != InvalidValue;
		}

		u64 Value()
		{
			return handle;
		}
	};

	struct Handle16
	{
		static constexpr u16 InvalidValue = 0xFFFFu;

		u16 handle = InvalidValue;

		bool isValid() const
		{
			return handle != InvalidValue;
		}

		u16 Value()
		{
			return handle;
		}
	};

	struct Handle8
	{
		static constexpr u8 InvalidValue = 0xFFu;

		u8 handle = InvalidValue;

		bool isValid() const
		{
			return handle != InvalidValue;
		}

		u8 Value()
		{
			return handle;
		}
	};
}