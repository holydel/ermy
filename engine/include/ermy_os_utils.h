#pragma once
#include <ermy_api.h>

namespace ermy
{
    namespace os_utils
    {
        typedef void* MappedFileHandle;

        MappedFileHandle MapFileReadOnly(const char8_t* pathUtf8);
        MappedFileHandle MapFileWrite(const char8_t* pathUtf8, u64 filesize);

        void* GetPointer(MappedFileHandle mfile);
        void CloseMappedFile(MappedFileHandle mfile);

        void SetNativeWindowTitle(const char8_t* title);
        const char* GetOSName();
    }
}