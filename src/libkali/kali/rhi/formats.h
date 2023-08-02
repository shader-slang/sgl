#pragma once

#include "kali/core/macros.h"
#include "kali/core/enum.h"
#include "kali/math/vector_types.h"

#include <slang-gfx.h>

namespace kali {

/// Resource formats.
enum class Format : uint32_t {
    unknown,

    rgba32_typeless,
    rgb32_typeless,
    rg32_typeless,
    r32_typeless,

    rgba16_typeless,
    rg16_typeless,
    r16_typeless,

    rgba8_typeless,
    rg8_typeless,
    r8_typeless,
    bgra8_typeless,

    rgba32_float,
    rgb32_float,
    rg32_float,
    r32_float,

    rgba16_float,
    rg16_float,
    r16_float,

    rgba32_uint,
    rgb32_uint,
    rg32_uint,
    r32_uint,

    rgba16_uint,
    rg16_uint,
    r16_uint,

    rgba8_uint,
    rg8_uint,
    r8_uint,

    rgba32_sint,
    rgb32_sint,
    rg32_sint,
    r32_sint,

    rgba16_sint,
    rg16_sint,
    r16_sint,

    rgba8_sint,
    rg8_sint,
    r8_sint,

    rgba16_unorm,
    rg16_unorm,
    r16_unorm,

    rgba8_unorm,
    rgba8_unorm_srgb,
    rg8_unorm,
    r8_unorm,
    bgra8_unorm,
    bgra8_unorm_srgb,
    bgrx8_unorm,
    bgrx8_unorm_srgb,

    rgba16_snorm,
    rg16_snorm,
    r16_snorm,

    rgba8_snorm,
    rg8_snorm,
    r8_snorm,

    d32_float,
    d16_unorm,
    d32_float_s8_uint,
    r32_float_x32_typeless,

    bgra4_unorm,
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

KALI_API gfx::Format get_gfx_format(Format format);

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
