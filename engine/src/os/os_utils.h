#pragma once
#include <ermy_api.h>
#include <ermy_log.h>
#include <ermy_os_utils.h>

namespace os
{
	void SetNativeThreadName(void* nativeThreadHandle, const char* utf8threadName);
	
	void UTF8ToWCS(const char* utf8string, wchar_t* outBuff, int maxBuffSize);
	void UTF8ToWCS(const char8_t* utf8string, wchar_t* outBuff, int maxBuffSize);
	void WCSToUTF8(const wchar_t* wcsString, char* outBuff, int maxBuffSize);
	void WCSToUTF8(const wchar_t* wcsString, char8_t* outBuff, int maxBuffSize);

	template<int N>
	void UTF8ToWCS(const char* utf8string, wchar_t (&outBuff)[N])
	{
		UTF8ToWCS(utf8string, outBuff, N * sizeof(wchar_t));
	}

	template<int N>
	void WCSToUTF8(const wchar_t* wcsString, char(&outBuff)[N])
	{
		WCSToUTF8(wcsString, outBuff, N * sizeof(char));
	}

	template<int N>
	void UTF8ToWCS(const char8_t* utf8string, wchar_t(&outBuff)[N])
	{
		UTF8ToWCS(utf8string, outBuff, N * sizeof(wchar_t));
	}

	template<int N>
	void WCSToUTF8(const wchar_t* wcsString, char8_t(&outBuff)[N])
	{
		WCSToUTF8(wcsString, outBuff, N * sizeof(char8_t));
	}

	void Sleep(int ms);
	void WriteDebugLogMessageIDE(ermy::LogSeverity severity, const char8_t* utf8Message);
	void WriteDebugLogMessageConsole(ermy::LogSeverity severity, const char8_t* utf8Message);
	void WriteDebugLogMessageFile(ermy::LogSeverity severity, const char8_t* utf8Message);

	void* LoadSharedLibrary(const char8_t* utf8libname);
	const char* GetSharedLibraryFullFilename(void* libHandle);
	bool UnloadSharedLibrary(void* library);

	void* GetFuncPtrImpl(void* library, const char* funcName);

	template <typename T>
	T GetFuncPtr(void* library, const char* funcName)
	{
		return reinterpret_cast<T>(GetFuncPtrImpl(library, funcName));

	}

	void FatalFail(const char* reason);

#ifdef ERMY_GAPI_VULKAN
	const char8_t* GetVulkanRuntimeLibraryName();
#endif

#ifdef ERMY_XR_OPENXR
	const char8_t* GetOpenXRLoaderLibraryName();
#endif

	ermy::i64 GetCurrentTimestamp();	
}

#define LOAD_FUNC_BY_NAME(libHandle,funcPtrOut,funcName) funcPtrOut = os::GetFuncPtr<decltype(funcPtrOut)>(libHandle,funcName);

#define LOAD_FUNC_PTR(libHandle,funcPtrOut) LOAD_FUNC_BY_NAME(libHandle,funcPtrOut,#funcPtrOut)

#define ERMY_ASSERT(assert) if(!(assert)) {os::FatalFail(#assert);}