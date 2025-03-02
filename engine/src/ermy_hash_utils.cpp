#include "ermy_utils.h"
#include <array>


ermy::u64 ermy_utils::hash::CalculateCRC64(const ermy::u8* data, const ermy::u32 size)
{
    class CRC64 {
    public:
        // Constructor initializes the CRC64 lookup table
        CRC64() {
            generateTable();
        }

        // Compute CRC64 hash for a given data buffer
        uint64_t compute(const ermy::u8* data, const ermy::u32 size) {
            uint64_t crc = 0;

            for (ermy::u32 i = 0; i < size; ++i) {
                crc = (crc >> 8) ^ table[(crc ^ data[i]) & 0xFF];
            }

            return crc;
        }

    private:
        // CRC64 lookup table
        std::array<uint64_t, 256> table;

        // Generate the CRC64 lookup table
        void generateTable() {
            const uint64_t polynomial = 0xC96C5795D7870F42; // CRC-64 ECMA-182 polynomial

            for (uint32_t i = 0; i < 256; ++i) {
                uint64_t crc = i;

                for (uint32_t j = 0; j < 8; ++j) {
                    if (crc & 1) {
                        crc = (crc >> 1) ^ polynomial;
                    }
                    else {
                        crc >>= 1;
                    }
                }

                table[i] = crc;
            }
        }
    };

    static CRC64 gCRC64;
    return gCRC64.compute(data, size);
}


