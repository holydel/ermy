#pragma once
#include <ermy_api.h>
#include <ermy_log.h>

namespace os
{
	void SetNativeThreadName(void* nativeThreadHandle, const char* utf8threadName);
	
	void UTF8ToWCS(const char* utf8string, wchar_t* outBuff, int maxBuffSize);
	void WCSToUTF8(const wchar_t* wcsString, char* outBuff, int maxBuffSize);

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

	void Sleep(int ms);
	void WriteDebugLogMessageIDE(ermy::LogSeverity severity, const char* utf8Message);
	void WriteDebugLogMessageConsole(ermy::LogSeverity severity, const char* utf8Message);
	void WriteDebugLogMessageFile(ermy::LogSeverity severity, const char* utf8Message);

	void* LoadSharedLibrary(const char* utf8libname);
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
	const char* GetVulkanRuntimeLibraryName();
#endif

	ermy::i64 GetCurrentTimestamp();
}

#define LOAD_FUNC_BY_NAME(libHandle,funcPtrOut,funcName) funcPtrOut = os::GetFuncPtr<decltype(funcPtrOut)>(libHandle,funcName);

#define LOAD_FUNC_PTR(libHandle,funcPtrOut) LOAD_FUNC_BY_NAME(libHandle,funcPtrOut,#funcPtrOut)

#define ERMY_ASSERT(assert) if(!(assert)) {os::FatalFail(#assert);}