#include "formats.h"

#include "core/assert.h"

namespace kali {

gfx::Format get_gfx_format(Format format)
{
    static_assert(uint32_t(Format::unknown) == uint32_t(gfx::Format::Unknown));
    static_assert(uint32_t(Format::r32g32b32a32_typeless) == uint32_t(gfx::Format::R32G32B32A32_TYPELESS));
    static_assert(uint32_t(Format::r32g32b32_typeless) == uint32_t(gfx::Format::R32G32B32_TYPELESS));
    static_assert(uint32_t(Format::r32g32_typeless) == uint32_t(gfx::Format::R32G32_TYPELESS));
    static_assert(uint32_t(Format::r32_typeless) == uint32_t(gfx::Format::R32_TYPELESS));
    static_assert(uint32_t(Format::r16g16b16a16_typeless) == uint32_t(gfx::Format::R16G16B16A16_TYPELESS));
    static_assert(uint32_t(Format::r16g16_typeless) == uint32_t(gfx::Format::R16G16_TYPELESS));
    static_assert(uint32_t(Format::r16_typeless) == uint32_t(gfx::Format::R16_TYPELESS));
    static_assert(uint32_t(Format::r8g8b8a8_typeless) == uint32_t(gfx::Format::R8G8B8A8_TYPELESS));
    static_assert(uint32_t(Format::r8g8_typeless) == uint32_t(gfx::Format::R8G8_TYPELESS));
    static_assert(uint32_t(Format::r8_typeless) == uint32_t(gfx::Format::R8_TYPELESS));
    static_assert(uint32_t(Format::b8g8r8a8_typeless) == uint32_t(gfx::Format::B8G8R8A8_TYPELESS));
    static_assert(uint32_t(Format::r32g32b32a32_float) == uint32_t(gfx::Format::R32G32B32A32_FLOAT));
    static_assert(uint32_t(Format::r32g32b32_float) == uint32_t(gfx::Format::R32G32B32_FLOAT));
    static_assert(uint32_t(Format::r32g32_float) == uint32_t(gfx::Format::R32G32_FLOAT));
    static_assert(uint32_t(Format::r32_float) == uint32_t(gfx::Format::R32_FLOAT));
    static_assert(uint32_t(Format::r16g16b16a16_float) == uint32_t(gfx::Format::R16G16B16A16_FLOAT));
    static_assert(uint32_t(Format::r16g16_float) == uint32_t(gfx::Format::R16G16_FLOAT));
    static_assert(uint32_t(Format::r16_float) == uint32_t(gfx::Format::R16_FLOAT));
    static_assert(uint32_t(Format::r32g32b32a32_uint) == uint32_t(gfx::Format::R32G32B32A32_UINT));
    static_assert(uint32_t(Format::r32g32b32_uint) == uint32_t(gfx::Format::R32G32B32_UINT));
    static_assert(uint32_t(Format::r32g32_uint) == uint32_t(gfx::Format::R32G32_UINT));
    static_assert(uint32_t(Format::r32_uint) == uint32_t(gfx::Format::R32_UINT));
    static_assert(uint32_t(Format::r16g16b16a16_uint) == uint32_t(gfx::Format::R16G16B16A16_UINT));
    static_assert(uint32_t(Format::r16g16_uint) == uint32_t(gfx::Format::R16G16_UINT));
    static_assert(uint32_t(Format::r16_uint) == uint32_t(gfx::Format::R16_UINT));
    static_assert(uint32_t(Format::r8g8b8a8_uint) == uint32_t(gfx::Format::R8G8B8A8_UINT));
    static_assert(uint32_t(Format::r8g8_uint) == uint32_t(gfx::Format::R8G8_UINT));
    static_assert(uint32_t(Format::r8_uint) == uint32_t(gfx::Format::R8_UINT));
    static_assert(uint32_t(Format::r32g32b32a32_sint) == uint32_t(gfx::Format::R32G32B32A32_SINT));
    static_assert(uint32_t(Format::r32g32b32_sint) == uint32_t(gfx::Format::R32G32B32_SINT));
    static_assert(uint32_t(Format::r32g32_sint) == uint32_t(gfx::Format::R32G32_SINT));
    static_assert(uint32_t(Format::r32_sint) == uint32_t(gfx::Format::R32_SINT));
    static_assert(uint32_t(Format::r16g16b16a16_sint) == uint32_t(gfx::Format::R16G16B16A16_SINT));
    static_assert(uint32_t(Format::r16g16_sint) == uint32_t(gfx::Format::R16G16_SINT));
    static_assert(uint32_t(Format::r16_sint) == uint32_t(gfx::Format::R16_SINT));
    static_assert(uint32_t(Format::r8g8b8a8_sint) == uint32_t(gfx::Format::R8G8B8A8_SINT));
    static_assert(uint32_t(Format::r8g8_sint) == uint32_t(gfx::Format::R8G8_SINT));
    static_assert(uint32_t(Format::r8_sint) == uint32_t(gfx::Format::R8_SINT));
    static_assert(uint32_t(Format::r16g16b16a16_unorm) == uint32_t(gfx::Format::R16G16B16A16_UNORM));
    static_assert(uint32_t(Format::r16g16_unorm) == uint32_t(gfx::Format::R16G16_UNORM));
    static_assert(uint32_t(Format::r16_unorm) == uint32_t(gfx::Format::R16_UNORM));
    static_assert(uint32_t(Format::r8g8b8a8_unorm) == uint32_t(gfx::Format::R8G8B8A8_UNORM));
    static_assert(uint32_t(Format::r8g8b8a8_unorm_srgb) == uint32_t(gfx::Format::R8G8B8A8_UNORM_SRGB));
    static_assert(uint32_t(Format::r8g8_unorm) == uint32_t(gfx::Format::R8G8_UNORM));
    static_assert(uint32_t(Format::r8_unorm) == uint32_t(gfx::Format::R8_UNORM));
    static_assert(uint32_t(Format::b8g8r8a8_unorm) == uint32_t(gfx::Format::B8G8R8A8_UNORM));
    static_assert(uint32_t(Format::b8g8r8a8_unorm_srgb) == uint32_t(gfx::Format::B8G8R8A8_UNORM_SRGB));
    static_assert(uint32_t(Format::b8g8r8x8_unorm) == uint32_t(gfx::Format::B8G8R8X8_UNORM));
    static_assert(uint32_t(Format::b8g8r8x8_unorm_srgb) == uint32_t(gfx::Format::B8G8R8X8_UNORM_SRGB));
    static_assert(uint32_t(Format::r16g16b16a16_snorm) == uint32_t(gfx::Format::R16G16B16A16_SNORM));
    static_assert(uint32_t(Format::r16g16_snorm) == uint32_t(gfx::Format::R16G16_SNORM));
    static_assert(uint32_t(Format::r16_snorm) == uint32_t(gfx::Format::R16_SNORM));
    static_assert(uint32_t(Format::r8g8b8a8_snorm) == uint32_t(gfx::Format::R8G8B8A8_SNORM));
    static_assert(uint32_t(Format::r8g8_snorm) == uint32_t(gfx::Format::R8G8_SNORM));
    static_assert(uint32_t(Format::r8_snorm) == uint32_t(gfx::Format::R8_SNORM));
    static_assert(uint32_t(Format::d32_float) == uint32_t(gfx::Format::D32_FLOAT));
    static_assert(uint32_t(Format::d16_unorm) == uint32_t(gfx::Format::D16_UNORM));
    static_assert(uint32_t(Format::d32_float_s8_uint) == uint32_t(gfx::Format::D32_FLOAT_S8_UINT));
    static_assert(uint32_t(Format::r32_float_x32_typeless) == uint32_t(gfx::Format::R32_FLOAT_X32_TYPELESS));
    static_assert(uint32_t(Format::b4g4r4a4_unorm) == uint32_t(gfx::Format::B4G4R4A4_UNORM));
    static_assert(uint32_t(Format::b5g6r5_unorm) == uint32_t(gfx::Format::B5G6R5_UNORM));
    static_assert(uint32_t(Format::b5g5r5a1_unorm) == uint32_t(gfx::Format::B5G5R5A1_UNORM));
    static_assert(uint32_t(Format::r9g9b9e5_sharedexp) == uint32_t(gfx::Format::R9G9B9E5_SHAREDEXP));
    static_assert(uint32_t(Format::r10g10b10a2_typeless) == uint32_t(gfx::Format::R10G10B10A2_TYPELESS));
    static_assert(uint32_t(Format::r10g10b10a2_unorm) == uint32_t(gfx::Format::R10G10B10A2_UNORM));
    static_assert(uint32_t(Format::r10g10b10a2_uint) == uint32_t(gfx::Format::R10G10B10A2_UINT));
    static_assert(uint32_t(Format::r11g11b10_float) == uint32_t(gfx::Format::R11G11B10_FLOAT));
    static_assert(uint32_t(Format::bc1_unorm) == uint32_t(gfx::Format::BC1_UNORM));
    static_assert(uint32_t(Format::bc1_unorm_srgb) == uint32_t(gfx::Format::BC1_UNORM_SRGB));
    static_assert(uint32_t(Format::bc2_unorm) == uint32_t(gfx::Format::BC2_UNORM));
    static_assert(uint32_t(Format::bc2_unorm_srgb) == uint32_t(gfx::Format::BC2_UNORM_SRGB));
    static_assert(uint32_t(Format::bc3_unorm) == uint32_t(gfx::Format::BC3_UNORM));
    static_assert(uint32_t(Format::bc3_unorm_srgb) == uint32_t(gfx::Format::BC3_UNORM_SRGB));
    static_assert(uint32_t(Format::bc4_unorm) == uint32_t(gfx::Format::BC4_UNORM));
    static_assert(uint32_t(Format::bc4_snorm) == uint32_t(gfx::Format::BC4_SNORM));
    static_assert(uint32_t(Format::bc5_unorm) == uint32_t(gfx::Format::BC5_UNORM));
    static_assert(uint32_t(Format::bc5_snorm) == uint32_t(gfx::Format::BC5_SNORM));
    static_assert(uint32_t(Format::bc6h_uf16) == uint32_t(gfx::Format::BC6H_UF16));
    static_assert(uint32_t(Format::bc6h_sf16) == uint32_t(gfx::Format::BC6H_SF16));
    static_assert(uint32_t(Format::bc7_unorm) == uint32_t(gfx::Format::BC7_UNORM));
    static_assert(uint32_t(Format::bc7_unorm_srgb) == uint32_t(gfx::Format::BC7_UNORM_SRGB));
    KALI_ASSERT(uint32_t(format) <= uint32_t(Format::bc7_unorm_srgb));
    return gfx::Format(format);
}

} // namespace kali
