#pragma once

#include "kali/core/macros.h"
#include "kali/core/enum.h"
#include "kali/math/vector_types.h"

#include <slang-gfx.h>

namespace kali {

/// Resource formats.
enum class Format : uint32_t {
    unknown = gfx::Format::Unknown,

    rgba32_typeless = gfx::Format::R32G32B32A32_TYPELESS,
    rgb32_typeless = gfx::Format::R32G32B32_TYPELESS,
    rg32_typeless = gfx::Format::R32G32_TYPELESS,
    r32_typeless = gfx::Format::R32_TYPELESS,

    rgba16_typeless = gfx::Format::R16G16B16A16_TYPELESS,
    rg16_typeless = gfx::Format::R16G16_TYPELESS,
    r16_typeless = gfx::Format::R16_TYPELESS,

    rgba8_typeless = gfx::Format::R8G8B8A8_TYPELESS,
    rg8_typeless = gfx::Format::R8G8_TYPELESS,
    r8_typeless = gfx::Format::R8_TYPELESS,
    bgra8_typeless = gfx::Format::B8G8R8A8_TYPELESS,

    rgba32_float = gfx::Format::R32G32B32A32_FLOAT,
    rgb32_float = gfx::Format::R32G32B32_FLOAT,
    rg32_float = gfx::Format::R32G32_FLOAT,
    r32_float = gfx::Format::R32_FLOAT,

    rgba16_float = gfx::Format::R16G16B16A16_FLOAT,
    rg16_float = gfx::Format::R16G16_FLOAT,
    r16_float = gfx::Format::R16_FLOAT,

    rgba32_uint = gfx::Format::R32G32B32A32_UINT,
    rgb32_uint = gfx::Format::R32G32B32_UINT,
    rg32_uint = gfx::Format::R32G32_UINT,
    r32_uint = gfx::Format::R32_UINT,

    rgba16_uint = gfx::Format::R16G16B16A16_UINT,
    rg16_uint = gfx::Format::R16G16_UINT,
    r16_uint = gfx::Format::R16_UINT,

    rgba8_uint = gfx::Format::R8G8B8A8_UINT,
    rg8_uint = gfx::Format::R8G8_UINT,
    r8_uint = gfx::Format::R8_UINT,

    rgba32_sint = gfx::Format::R32G32B32A32_SINT,
    rgb32_sint = gfx::Format::R32G32B32_SINT,
    rg32_sint = gfx::Format::R32G32_SINT,
    r32_sint = gfx::Format::R32_SINT,

    rgba16_sint = gfx::Format::R16G16B16A16_SINT,
    rg16_sint = gfx::Format::R16G16_SINT,
    r16_sint = gfx::Format::R16_SINT,

    rgba8_sint = gfx::Format::R8G8B8A8_SINT,
    rg8_sint = gfx::Format::R8G8_SINT,
    r8_sint = gfx::Format::R8_SINT,

    rgba16_unorm = gfx::Format::R16G16B16A16_UNORM,
    rg16_unorm = gfx::Format::R16G16_UNORM,
    r16_unorm = gfx::Format::R16_UNORM,

    rgba8_unorm = gfx::Format::R8G8B8A8_UNORM,
    rgba8_unorm_srgb = gfx::Format::R8G8B8A8_UNORM_SRGB,
    rg8_unorm = gfx::Format::R8G8_UNORM,
    r8_unorm = gfx::Format::R8_UNORM,
    bgra8_unorm = gfx::Format::B8G8R8A8_UNORM,
    bgra8_unorm_srgb = gfx::Format::B8G8R8A8_UNORM_SRGB,
    bgrx8_unorm = gfx::Format::B8G8R8X8_UNORM,
    bgrx8_unorm_srgb = gfx::Format::B8G8R8X8_UNORM_SRGB,

    rgba16_snorm = gfx::Format::R16G16B16A16_SNORM,
    rg16_snorm = gfx::Format::R16G16_SNORM,
    r16_snorm = gfx::Format::R16_SNORM,

    rgba8_snorm = gfx::Format::R8G8B8A8_SNORM,
    rg8_snorm = gfx::Format::R8G8_SNORM,
    r8_snorm = gfx::Format::R8_SNORM,

    d32_float = gfx::Format::D32_FLOAT,
    d16_unorm = gfx::Format::D16_UNORM,
    d32_float_s8_uint = gfx::Format::D32_FLOAT_S8_UINT,
    r32_float_x32_typeless = gfx::Format::R32_FLOAT_X32_TYPELESS,

    bgra4_unorm = gfx::Format::B4G4R4A4_UNORM,
    b5g6r5_unorm = gfx::Format::B5G6R5_UNORM,
    b5g5r5a1_unorm = gfx::Format::B5G5R5A1_UNORM,

    r9g9b9e5_sharedexp = gfx::Format::R9G9B9E5_SHAREDEXP,
    r10g10b10a2_typeless = gfx::Format::R10G10B10A2_TYPELESS,
    r10g10b10a2_unorm = gfx::Format::R10G10B10A2_UNORM,
    r10g10b10a2_uint = gfx::Format::R10G10B10A2_UINT,
    r11g11b10_float = gfx::Format::R11G11B10_FLOAT,

    bc1_unorm = gfx::Format::BC1_UNORM,
    bc1_unorm_srgb = gfx::Format::BC1_UNORM_SRGB,
    bc2_unorm = gfx::Format::BC2_UNORM,
    bc2_unorm_srgb = gfx::Format::BC2_UNORM_SRGB,
    bc3_unorm = gfx::Format::BC3_UNORM,
    bc3_unorm_srgb = gfx::Format::BC3_UNORM_SRGB,
    bc4_unorm = gfx::Format::BC4_UNORM,
    bc4_snorm = gfx::Format::BC4_SNORM,
    bc5_unorm = gfx::Format::BC5_UNORM,
    bc5_snorm = gfx::Format::BC5_SNORM,
    bc6h_uf16 = gfx::Format::BC6H_UF16,
    bc6h_sf16 = gfx::Format::BC6H_SF16,
    bc7_unorm = gfx::Format::BC7_UNORM,
    bc7_unorm_srgb = gfx::Format::BC7_UNORM_SRGB,

    count,
};

/// Resource format types.
enum class FormatType {
    unknown,    ///< Unknown format
    typeless,   ///< Typeless formats
    float_,     ///< Floating-point formats
    unorm,      ///< Unsigned normalized formats
    unorm_srgb, ///< Unsigned normalized SRGB formats
    snorm,      ///< Signed normalized formats
    uint,       ///< Unsigned integer formats
    sint        ///< Signed integer formats
};

KALI_ENUM_INFO(
    FormatType,
    {
        {FormatType::unknown, "unknown"},
        {FormatType::typeless, "typeless"},
        {FormatType::float_, "float"},
        {FormatType::unorm, "unorm"},
        {FormatType::unorm_srgb, "unorm_srgb"},
        {FormatType::snorm, "snorm"},
        {FormatType::uint, "uint"},
        {FormatType::sint, "sint"},
    }
);
KALI_ENUM_REGISTER(FormatType);

enum class TextureChannelFlags : uint32_t {
    none = 0x0,
    r = 0x1,
    g = 0x2,
    b = 0x4,
    a = 0x8,
    rg = 0x3,
    rgb = 0x7,
    rgba = 0xf,
};

KALI_ENUM_CLASS_OPERATORS(TextureChannelFlags);

/// Resource format information.
struct FormatInfo {
    Format format;
    std::string name;
    uint32_t bytes_per_block;
    uint32_t channel_count;
    FormatType type;
    bool is_depth;
    bool is_stencil;
    bool is_compressed;
    uint32_t block_width;
    uint32_t block_height;
    int channel_bit_count[4];
    uint32_t dxgi_format;
    uint32_t vk_format;

    bool is_depth_stencil() const { return is_depth || is_stencil; }

    bool is_typeless_format() const { return type == FormatType::typeless; }
    bool is_float_format() const { return type == FormatType::float_; }
    bool is_integer_format() const { return type == FormatType::uint || type == FormatType::sint; }
    bool is_normalized_format() const
    {
        return type == FormatType::unorm || type == FormatType::unorm_srgb || type == FormatType::snorm;
    }
    bool is_srgb_format() const { return type == FormatType::unorm_srgb; }

    TextureChannelFlags get_mask() const
    {
        TextureChannelFlags flags = TextureChannelFlags::none;
        flags |= channel_count >= 1 ? TextureChannelFlags::r : TextureChannelFlags::none;
        flags |= channel_count >= 2 ? TextureChannelFlags::g : TextureChannelFlags::none;
        flags |= channel_count >= 3 ? TextureChannelFlags::b : TextureChannelFlags::none;
        flags |= channel_count >= 4 ? TextureChannelFlags::a : TextureChannelFlags::none;
        return flags;
    }

    uint32_t get_channel_bits(TextureChannelFlags flags) const
    {
        uint32_t bits = 0;
        bits += (is_set(flags, TextureChannelFlags::r)) ? channel_bit_count[0] : 0;
        bits += (is_set(flags, TextureChannelFlags::g)) ? channel_bit_count[1] : 0;
        bits += (is_set(flags, TextureChannelFlags::b)) ? channel_bit_count[2] : 0;
        bits += (is_set(flags, TextureChannelFlags::a)) ? channel_bit_count[3] : 0;
        return bits;
    }
};

KALI_API const FormatInfo& get_format_info(Format format);

namespace detail {
    template<typename T>
    struct HostTypeToFormat {
        static constexpr Format value = Format::unknown;
    };

#define KALI_HOST_TYPE_TO_FORMAT(type, format)                                                                         \
    template<>                                                                                                         \
    struct HostTypeToFormat<type> {                                                                                    \
        static constexpr Format value = format;                                                                        \
    }

    KALI_HOST_TYPE_TO_FORMAT(int8_t, Format::r8_sint);
    KALI_HOST_TYPE_TO_FORMAT(uint8_t, Format::r8_uint);
    KALI_HOST_TYPE_TO_FORMAT(int16_t, Format::r16_sint);
    KALI_HOST_TYPE_TO_FORMAT(uint16_t, Format::r16_uint);
    KALI_HOST_TYPE_TO_FORMAT(int, Format::r32_sint);
    KALI_HOST_TYPE_TO_FORMAT(int2, Format::rg32_sint);
    KALI_HOST_TYPE_TO_FORMAT(int4, Format::rgba32_sint);
    KALI_HOST_TYPE_TO_FORMAT(uint, Format::r32_uint);
    KALI_HOST_TYPE_TO_FORMAT(uint2, Format::rg32_uint);
    KALI_HOST_TYPE_TO_FORMAT(uint4, Format::rgba32_uint);
    KALI_HOST_TYPE_TO_FORMAT(float16_t, Format::r16_float);
    KALI_HOST_TYPE_TO_FORMAT(float16_t2, Format::rg16_float);
    KALI_HOST_TYPE_TO_FORMAT(float16_t4, Format::rgba16_float);
    KALI_HOST_TYPE_TO_FORMAT(float, Format::r32_float);
    KALI_HOST_TYPE_TO_FORMAT(float2, Format::rg32_float);
    KALI_HOST_TYPE_TO_FORMAT(float3, Format::rgb32_float);
    KALI_HOST_TYPE_TO_FORMAT(float4, Format::rgba32_float);

#undef KALI_HOST_TYPE_TO_FORMAT
} // namespace detail

template<typename T>
inline constexpr Format host_type_to_format()
{
    return detail::HostTypeToFormat<T>::value;
}

// Manually define the struct that KALI_ENUM_INFO(Format) would generate so we
// can use the existing table of resource formats.
struct Format_info {
    static std::span<std::pair<Format, std::string>> items()
    {
        auto create_items = []()
        {
            std::vector<std::pair<Format, std::string>> items((size_t)Format::count);
            for (size_t i = 0; i < (size_t)Format::count; ++i)
                items[i] = std::make_pair(Format(i), get_format_info(Format(i)).name);
            return items;
        };
        static std::vector<std::pair<Format, std::string>> items = create_items();
        return items;
    }
};
KALI_ENUM_REGISTER(Format);

} // namespace kali
