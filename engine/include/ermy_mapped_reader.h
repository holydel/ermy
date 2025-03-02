#pragma once
#include "ermy_api.h"
#include <string>
#include <vector>

namespace ermy
{
	class MappedFileReader
	{
		u8* data = nullptr;

	public:
		MappedFileReader(void* dataPtr)
			:data((u8*)dataPtr)
		{

		}

		~MappedFileReader() = default;
		MappedFileReader(const MappedFileReader&) = default;

		void* CurrentPointer()
		{
			return data;
		}

		void Advance(int numBytes)
		{
			data += numBytes;
		}

		void ReadBytes(int numBytes, void* outData)
		{
			memcpy(outData, data, numBytes);
			Advance(numBytes);
		}

		u32 ReadU32()
		{
			u32 result = 0;
			ReadBytes(sizeof(result), &result);
			return result;
		}

		u16 ReadU16()
		{
			u16 result;
			ReadBytes(sizeof(result), &result);
			return result;
		}

		u64 ReadU64()
		{
			u64 result;
			ReadBytes(sizeof(result), &result);
			return result;
		}

		u8 ReadU8()
		{
			u8 result;
			ReadBytes(sizeof(result), &result);
			return result;
		}

		template<typename T>
		void ReadVector(std::vector<T>& out)
		{
			size_t size = ReadU64();
			out.resize(size);
			ReadBytes(static_cast<u32>(sizeof(T) * size), out.data());
		}
	};
}