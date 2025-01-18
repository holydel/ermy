#include "../ermy_os_utils.h"
#include "win_utils.h"


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

void os::sleep(int ms)
{
	Sleep(ms);
}