#pragma once

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
	void WriteDebugLogMessageIDE(const char* utf8Message);
}