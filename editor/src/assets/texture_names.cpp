#include "assets/texture_asset.h"

// Define the arrays with explicit sizes
const char* TexturePurposeNames[TEXTURE_PURPOSE_COUNT] = { 
    "Albedo (sRGB Compressed)",
    "Utility (Linear Compressed)",
    "Normal Map (Linear RG Compressed)",
    "HDR (Linear)",
    "UI (sRGB Uncompressed)",
    "Source (User-Defined)" 
};

const char* TextureCompressionNames[TEXTURE_COMPRESSION_COUNT] = {
    "None (source)",

    // HDR
    "HDR 32bit (R11G11B10F)",
    "HDR 32bit (RGB9E5)",
    // BC
    "BC1 (sRGB RGB, 4 bpp)",
    "BC1 (Linear RGB, 4 bpp)",
    "BC4 (Linear R, 4 bpp)",
    "BC5 (Linear RG, 8 bpp)",
    "BC6H (HDR RGB, 8 bpp)",
    "BC7 (sRGB RGBA, 8 bpp)",
    "BC7 (Linear RGBA, 8 bpp)",

    // ASTC LDR
    "ASTC LDR 4x4 (sRGB RGB, 8 bpp)",
    "ASTC LDR 4x4 (sRGB RGBA, 8 bpp)",
    "ASTC LDR 4x4 (Linear RGB, 8 bpp)",
    "ASTC LDR 4x4 (Linear RGBA, 8 bpp)",
    "ASTC LDR 6x6 (sRGB RGB, 3.56 bpp)",
    "ASTC LDR 6x6 (sRGB RGBA, 3.56 bpp)",
    "ASTC LDR 6x6 (Linear RGB, 3.56 bpp)",
    "ASTC LDR 6x6 (Linear RGBA, 3.56 bpp)",
    "ASTC LDR 8x8 (sRGB RGB, 2 bpp)",
    "ASTC LDR 8x8 (sRGB RGBA, 2 bpp)",
    "ASTC LDR 8x8 (Linear RGB, 2 bpp)",
    "ASTC LDR 8x8 (Linear RGBA, 2 bpp)",

    // ASTC HDR
    "ASTC HDR 4x4 (HDR RGB, 8 bpp)",
    "ASTC HDR 4x4 (HDR RGBA, 8 bpp)",
    "ASTC HDR 6x6 (HDR RGB, 3.56 bpp)",
    "ASTC HDR 6x6 (HDR RGBA, 3.56 bpp)",
    "ASTC HDR 8x8 (HDR RGB, 2 bpp)",
    "ASTC HDR 8x8 (HDR RGBA, 2 bpp)",

    // ETC
    "ETC1 (sRGB RGB, 4 bpp)",
    "ETC2 (sRGB RGB, 4 bpp)",
    "ETC2 (sRGB RGBA, 4 bpp)",
    "ETC2 (Linear RGB, 4 bpp)",
    "ETC2 (Linear RGBA, 4 bpp)",

    // PVRTC
    "PVRTC 4bpp (sRGB RGB)",
    "PVRTC 4bpp (sRGB RGBA)",
    "PVRTC 4bpp (Linear RGB)",
    "PVRTC 4bpp (Linear RGBA)",
    "PVRTC 2bpp (sRGB RGB)",
    "PVRTC 2bpp (sRGB RGBA)",
    "PVRTC 2bpp (Linear RGB)",
    "PVRTC 2bpp (Linear RGBA)",
}; 