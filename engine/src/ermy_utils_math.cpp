#include "ermy_utils.h"
#include <limits.h>

ermy::u64 ermy_utils::math::alignUpPow2(ermy::u64 in)
{
    #if defined(__GNUC__) && defined(__aarch64__)
        uint64_t x = (uint64_t)in * 2 - 1;
    #ifdef __builtin_arm_rbitll
        x = __builtin_arm_rbitll(x);
    #else
        __asm__("rbit %0, %1" : "=r"(x) : "r"(x));
    #endif
        uint64_t y = x & -x;
    #ifdef __builtin_arm_rbitll
        y = __builtin_arm_rbitll(y);
    #else
        __asm__("rbit %0, %1" : "=r"(y) : "r"(y));
    #endif
        return (uint32_t)y;
    #elif defined(__GNUC__) 
        if (in > UINT32_MAX) __builtin_unreachable();
        unsigned long long inT2M1 = 2 * (unsigned long long)in - 1, clz;
        clz = __builtin_clzll(inT2M1) ^ (sizeof(long long) * 8 - 1);
        return (uint32_t)((unsigned long long)1 << clz);
    #elif defined(_MSC_VER) 
        unsigned __int64 inT2M1 = 2 * (unsigned __int64)in - 1;
        unsigned long res = (unsigned long)inT2M1;
        _BitScanReverse64(&res, inT2M1);
        return (uint32_t)((__int64)1 << res);
    #else // otherwise, use the slow fallback
        in -= 1;
        if (in >> 27) in |= in >> 27; // very unlikely, so free
        in |= (in >> 9) | (in >> 18), in |= (in >> 3) | (in >> 6);
        return (in | (in >> 1) | (in >> 2)) + 1;
    #endif
}

ermy::u64 ermy_utils::math::alignUp(ermy::u64 value, ermy::u32 align)
{
    __assume(align > 0);

    return ((value + align - 1) / align) * align;
}