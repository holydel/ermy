
#include "android_utils.h"
#include <stdio.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/log.h>
#include <android/input.h>

#include <android/ndk-version.h>

#include <string>
#include <thread>
#include <atomic>

#include <dlfcn.h>
#include <unordered_map>

#include <fcntl.h>      // For open, O_RDONLY, O_RDWR, O_CREAT, etc.
#include <sys/mman.h>   // For mmap, munmap
#include <sys/stat.h>   // For fstat
#include <unistd.h>     // For close, ftruncate
#include <cstdint>      // For u64
#include <cstring>      // For strerror
#include <cerrno>       // For errno
#include <stdexcept>    // For std::runtime_error

void os::SetNativeThreadName(void* nativeThreadHandle, const char* utf8threadName)
{
	wchar_t outbuff[256] = {};
	os::UTF8ToWCS(utf8threadName, outbuff);

	//TODO: implement
}

void os::UTF8ToWCS(const char* utf8string, wchar_t* outBuff, int maxBuffSize)
{
	//MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, outBuff, maxBuffSize);

	//TODO: implement
}

void os::WCSToUTF8(const wchar_t* wcs8string, char* outBuff, int maxBuffSize)
{
	//WideCharToMultiByte(CP_UTF8, 0, wcs8string, -1, outBuff, maxBuffSize, nullptr, nullptr);

	//TODO: implement
}

void os::Sleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void os::WriteDebugLogMessageIDE(ermy::LogSeverity severity, const char* utf8Message)
{
	__android_log_print(ANDROID_LOG_DEBUG, "TAG", "%s", utf8Message);
	//TODO: research output debug string for Android Studio
	//OutputDebugStringA(utf8Message);
}

void os::WriteDebugLogMessageConsole(ermy::LogSeverity severity, const char* utf8Message)
{

}

void os::WriteDebugLogMessageFile(ermy::LogSeverity severity, const char* utf8Message)
{

}

void* os::LoadSharedLibrary(const char* utf8libname)
{
	return dlopen(utf8libname, RTLD_NOW);
}

bool os::UnloadSharedLibrary(void* library)
{
	dlclose(library);
	return true;
}

std::unordered_map<void*, void*> ptrToLibHandleMap;

void* os::GetFuncPtrImpl(void* library, const char* funcName)
{
	auto result = dlsym(library, funcName);

	ptrToLibHandleMap[library] = result;
	return result;
}

void os::FatalFail(const char* reason)
{
	//mercury::write_log_message("%s\n", reason);
	//DebugBreak();
}

const char* os::GetSharedLibraryFullFilename(void* libHandle)
{
	static Dl_info dl_info = {};
	dladdr(ptrToLibHandleMap[libHandle], &dl_info);

	return dl_info.dli_fname;
}

#ifdef ERMY_GAPI_VULKAN
const char* os::GetVulkanRuntimeLibraryName()
{
	return "libvulkan.so";
}
#endif

#ifdef ERMY_XR_OPENXR
const char* os::GetOpenXRLoaderLibraryName()
{
	return "libopenxr_loader.so";
}
#endif

namespace ermy::os_utils
{
	struct MappedFileInfoPosix
	{
		int fd = -1;                // File descriptor
		void* pData = nullptr;      // Pointer to mapped memory
		size_t fileSize = 0;        // Size of the file
	};

	MappedFileHandle MapFileReadOnly(const char* pathUtf8)
	{
		MappedFileInfoPosix* m = new MappedFileInfoPosix();

		// Open the file
		m->fd = open(pathUtf8, O_RDONLY);
		if (m->fd == -1) {
			return m;
		}

		// Get the file size
		struct stat fileStat;
		if (fstat(m->fd, &fileStat)) {
			close(m->fd);
			return m;
		}

		m->fileSize = fileStat.st_size;

			// Map the file into memory
			m->pData = mmap(nullptr, m->fileSize, PROT_READ, MAP_PRIVATE, m->fd, 0);
			if (m->pData == MAP_FAILED) {
				close(m->fd);
				return m;
			}

		return m;
	}

	MappedFileHandle MapFileWrite(const char* pathUtf8, u64 filesize)
	{
		MappedFileInfoPosix* m = new MappedFileInfoPosix();

		// Open the file for writing
		m->fd = open(pathUtf8, O_RDWR | O_CREAT | O_TRUNC, 0644);
		if (m->fd == -1) {
			return m;
		}

		// Set the file size
		if (ftruncate(m->fd, filesize)) {
			close(m->fd);
			return m;
		}
		m->fileSize = filesize;

		// Map the file into memory
		m->pData = mmap(nullptr, m->fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, m->fd, 0);
		if (m->pData == MAP_FAILED) {
			close(m->fd);
			return m;
		}

		return m;
	}

	void* GetPointer(MappedFileHandle mfile)
	{
		MappedFileInfoPosix* m = reinterpret_cast<MappedFileInfoPosix*>(mfile);
		return m->pData;
	}

	void CloseMappedFile(MappedFileHandle mfile)
	{
		MappedFileInfoPosix* m = reinterpret_cast<MappedFileInfoPosix*>(mfile);

		if (m->pData) {
			munmap(m->pData, m->fileSize);
		}
		if (m->fd != -1) {
			close(m->fd);
		}
		delete m;
	}
}