#pragma once
#include "ermy_api.h"
#include <string>
#include <vector>

namespace ermy
{
    class MappedFileWriter {
        u8* data = nullptr;

    public:
        MappedFileWriter(void* dataPtr)
            : data((u8*)dataPtr) {
        }

        ~MappedFileWriter() = default;
        MappedFileWriter(const MappedFileWriter&) = default;

        void WriteBytes(int numBytes, const void* inData) {
            memcpy(data, inData, numBytes);
            data += numBytes;
        }

        void WriteU32(u32 value) {
            WriteBytes(sizeof(value), &value);
        }

        void WriteU16(u16 value) {
            WriteBytes(sizeof(value), &value);
        }

        void WriteU64(u64 value) {
            WriteBytes(sizeof(value), &value);
        }

        void WriteU8(u8 value) {
            WriteBytes(sizeof(value), &value);
        }
    };
}