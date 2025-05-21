#include "../os_utils.h"
#include "win_utils.h"
#include <iostream>

void os::SetNativeThreadName(void* nativeThreadHandle, const char* utf8threadName)
{
	wchar_t outbuff[256] = {};
	os::UTF8ToWCS(utf8threadName, outbuff);

	HRESULT hr = SetThreadDescription(nativeThreadHandle, outbuff);
}

void os::UTF8ToWCS(const char* utf8string, wchar_t* outBuff, int maxBuffSize)
{
	MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, outBuff, maxBuffSize);
}

void os::WCSToUTF8(const wchar_t* wcs8string, char* outBuff, int maxBuffSize)
{
	WideCharToMultiByte(CP_UTF8, 0, wcs8string, -1, outBuff, maxBuffSize, nullptr, nullptr);
}

void os::UTF8ToWCS(const char8_t* utf8string, wchar_t* outBuff, int maxBuffSize)
{
	MultiByteToWideChar(CP_UTF8, 0, (const char*)utf8string, -1, outBuff, maxBuffSize);
}

void os::WCSToUTF8(const wchar_t* wcs8string, char8_t* outBuff, int maxBuffSize)
{
	WideCharToMultiByte(CP_UTF8, 0, wcs8string, -1, (char*)outBuff, maxBuffSize, nullptr, nullptr);
}

void os::Sleep(int ms)
{
	::Sleep(ms);
}

void os::WriteDebugLogMessageIDE(ermy::LogSeverity severity, const char8_t* utf8Message)
{
	WCHAR wcsMessage[1024] = {};
	std::mbstowcs(wcsMessage, (const char*)utf8Message, sizeof(wcsMessage) / sizeof(WCHAR));
	OutputDebugStringW(wcsMessage);
}

void print_mb(const char8_t* utf8Message)
{
	WCHAR wcsMessage[1024] = {0};
	std::mbstowcs(wcsMessage, (const char*)utf8Message, sizeof(wcsMessage) / sizeof(WCHAR));
	std::wcout << wcsMessage;
	OutputDebugStringW(wcsMessage);
}

void os::WriteDebugLogMessageConsole(ermy::LogSeverity severity, const char8_t* utf8Message)
{
	//change console color
	switch (severity)
	{
	case ermy::LogSeverity::Verbose:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
		break;
	case ermy::LogSeverity::Debug:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case ermy::LogSeverity::Warning:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_RED);
		break;
	case ermy::LogSeverity::Error:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
		break;
	case ermy::LogSeverity::Fatal:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	default:
		break;
	}

	print_mb(utf8Message);

	//restore previous console color
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
}

void os::WriteDebugLogMessageFile(ermy::LogSeverity severity, const char8_t* utf8Message)
{

}

void* os::LoadSharedLibrary(const char8_t* utf8libname)
{
	wchar_t utf8libnameW[1024] = {};
	MultiByteToWideChar(CP_UTF8, 0, (const char*)utf8libname, -1, utf8libnameW, sizeof(utf8libnameW) / sizeof(wchar_t));
	return LoadLibraryW(utf8libnameW);
}

bool os::UnloadSharedLibrary(void* library)
{
	return FreeLibrary(static_cast<HMODULE>(library));
}

void* os::GetFuncPtrImpl(void* library, const char* funcName)
{
	return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(library), funcName));
}

void os::FatalFail(const char* reason)
{
	ERMY_FATAL(u8"Fatal error: %s", reason);
	DebugBreak();
}

const char* os::GetSharedLibraryFullFilename(void* libHandle)
{
	static char buffer[1024];
	GetModuleFileNameA(static_cast<HMODULE>(libHandle), buffer, sizeof(buffer));
	return buffer;
}

#ifdef ERMY_GAPI_VULKAN
const char8_t* os::GetVulkanRuntimeLibraryName()
{
	return u8"vulkan-1.dll";
}
#endif

#ifdef ERMY_XR_OPENXR
const char8_t* os::GetOpenXRLoaderLibraryName()
{
	return u8"openxr_loader.dll";
}
#endif