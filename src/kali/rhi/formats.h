#pragma once

#include "core/platform.h"

#include <slang-gfx.h>

namespace kali {

enum class Format : uint32_t {
    unknown,

    r32g32b32a32_typeless,
    r32g32b32_typeless,
    r32g32_typeless,
    r32_typeless,

    r16g16b16a16_typeless,
    r16g16_typeless,
    r16_typeless,

    r8g8b8a8_typeless,
    r8g8_typeless,
    r8_typeless,
    b8g8r8a8_typeless,

    r32g32b32a32_float,
    r32g32b32_float,
    r32g32_float,
    r32_float,

    r16g16b16a16_float,
    r16g16_float,
    r16_float,

    r32g32b32a32_uint,
    r32g32b32_uint,
    r32g32_uint,
    r32_uint,

    r16g16b16a16_uint,
    r16g16_uint,
    r16_uint,

    r8g8b8a8_uint,
    r8g8_uint,
    r8_uint,

    r32g32b32a32_sint,
    r32g32b32_sint,
    r32g32_sint,
    r32_sint,

    r16g16b16a16_sint,
    r16g16_sint,
    r16_sint,

    r8g8b8a8_sint,
    r8g8_sint,
    r8_sint,

    r16g16b16a16_unorm,
    r16g16_unorm,
    r16_unorm,

    r8g8b8a8_unorm,
    r8g8b8a8_unorm_srgb,
    r8g8_unorm,
    r8_unorm,
    b8g8r8a8_unorm,
    b8g8r8a8_unorm_srgb,
    b8g8r8x8_unorm,
    b8g8r8x8_unorm_srgb,

    r16g16b16a16_snorm,
    r16g16_snorm,
    r16_snorm,

    r8g8b8a8_snorm,
    r8g8_snorm,
    r8_snorm,

    d32_float,
    d16_unorm,
    d32_float_s8_uint,
    r32_float_x32_typeless,

    b4g4r4a4_unorm,
    b5g6r5_unorm,
    b5g5r5a1_unorm,

    r9g9b9e5_sharedexp,
    r10g10b10a2_typeless,
    r10g10b10a2_unorm,
    r10g10b10a2_uint,
    r11g11b10_float,

    bc1_unorm,
    bc1_unorm_srgb,
    bc2_unorm,
    bc2_unorm_srgb,
    bc3_unorm,
    bc3_unorm_srgb,
    bc4_unorm,
    bc4_snorm,
    bc5_unorm,
    bc5_snorm,
    bc6h_uf16,
    bc6h_sf16,
    bc7_unorm,
    bc7_unorm_srgb,
};

KALI_API gfx::Format get_gfx_format(Format format);

}; // namespace kali
