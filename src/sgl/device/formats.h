// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/macros.h"
#include "sgl/core/enum.h"
#include "sgl/math/vector_types.h"

#include <slang-gfx.h>

#include <array>

namespace sgl {

/// Resource formats.
enum class Format : uint32_t {
    unknown = static_cast<uint32_t>(gfx::Format::Unknown),

    rgba32_typeless = static_cast<uint32_t>(gfx::Format::R32G32B32A32_TYPELESS),
    rgb32_typeless = static_cast<uint32_t>(gfx::Format::R32G32B32_TYPELESS),
    rg32_typeless = static_cast<uint32_t>(gfx::Format::R32G32_TYPELESS),
    r32_typeless = static_cast<uint32_t>(gfx::Format::R32_TYPELESS),

    rgba16_typeless = static_cast<uint32_t>(gfx::Format::R16G16B16A16_TYPELESS),
    rg16_typeless = static_cast<uint32_t>(gfx::Format::R16G16_TYPELESS),
    r16_typeless = static_cast<uint32_t>(gfx::Format::R16_TYPELESS),

    rgba8_typeless = static_cast<uint32_t>(gfx::Format::R8G8B8A8_TYPELESS),
    rg8_typeless = static_cast<uint32_t>(gfx::Format::R8G8_TYPELESS),
    r8_typeless = static_cast<uint32_t>(gfx::Format::R8_TYPELESS),
    bgra8_typeless = static_cast<uint32_t>(gfx::Format::B8G8R8A8_TYPELESS),

    rgba32_float = static_cast<uint32_t>(gfx::Format::R32G32B32A32_FLOAT),
    rgb32_float = static_cast<uint32_t>(gfx::Format::R32G32B32_FLOAT),
    rg32_float = static_cast<uint32_t>(gfx::Format::R32G32_FLOAT),
    r32_float = static_cast<uint32_t>(gfx::Format::R32_FLOAT),

    rgba16_float = static_cast<uint32_t>(gfx::Format::R16G16B16A16_FLOAT),
    rg16_float = static_cast<uint32_t>(gfx::Format::R16G16_FLOAT),
    r16_float = static_cast<uint32_t>(gfx::Format::R16_FLOAT),

    rgba32_uint = static_cast<uint32_t>(gfx::Format::R32G32B32A32_UINT),
    rgb32_uint = static_cast<uint32_t>(gfx::Format::R32G32B32_UINT),
    rg32_uint = static_cast<uint32_t>(gfx::Format::R32G32_UINT),
    r32_uint = static_cast<uint32_t>(gfx::Format::R32_UINT),

    rgba16_uint = static_cast<uint32_t>(gfx::Format::R16G16B16A16_UINT),
    rg16_uint = static_cast<uint32_t>(gfx::Format::R16G16_UINT),
    r16_uint = static_cast<uint32_t>(gfx::Format::R16_UINT),

    rgba8_uint = static_cast<uint32_t>(gfx::Format::R8G8B8A8_UINT),
    rg8_uint = static_cast<uint32_t>(gfx::Format::R8G8_UINT),
    r8_uint = static_cast<uint32_t>(gfx::Format::R8_UINT),

    rgba32_sint = static_cast<uint32_t>(gfx::Format::R32G32B32A32_SINT),
    rgb32_sint = static_cast<uint32_t>(gfx::Format::R32G32B32_SINT),
    rg32_sint = static_cast<uint32_t>(gfx::Format::R32G32_SINT),
    r32_sint = static_cast<uint32_t>(gfx::Format::R32_SINT),

    rgba16_sint = static_cast<uint32_t>(gfx::Format::R16G16B16A16_SINT),
    rg16_sint = static_cast<uint32_t>(gfx::Format::R16G16_SINT),
    r16_sint = static_cast<uint32_t>(gfx::Format::R16_SINT),

    rgba8_sint = static_cast<uint32_t>(gfx::Format::R8G8B8A8_SINT),
    rg8_sint = static_cast<uint32_t>(gfx::Format::R8G8_SINT),
    r8_sint = static_cast<uint32_t>(gfx::Format::R8_SINT),

    rgba16_unorm = static_cast<uint32_t>(gfx::Format::R16G16B16A16_UNORM),
    rg16_unorm = static_cast<uint32_t>(gfx::Format::R16G16_UNORM),
    r16_unorm = static_cast<uint32_t>(gfx::Format::R16_UNORM),

    rgba8_unorm = static_cast<uint32_t>(gfx::Format::R8G8B8A8_UNORM),
    rgba8_unorm_srgb = static_cast<uint32_t>(gfx::Format::R8G8B8A8_UNORM_SRGB),
    rg8_unorm = static_cast<uint32_t>(gfx::Format::R8G8_UNORM),
    r8_unorm = static_cast<uint32_t>(gfx::Format::R8_UNORM),
    bgra8_unorm = static_cast<uint32_t>(gfx::Format::B8G8R8A8_UNORM),
    bgra8_unorm_srgb = static_cast<uint32_t>(gfx::Format::B8G8R8A8_UNORM_SRGB),
    bgrx8_unorm = static_cast<uint32_t>(gfx::Format::B8G8R8X8_UNORM),
    bgrx8_unorm_srgb = static_cast<uint32_t>(gfx::Format::B8G8R8X8_UNORM_SRGB),

    rgba16_snorm = static_cast<uint32_t>(gfx::Format::R16G16B16A16_SNORM),
    rg16_snorm = static_cast<uint32_t>(gfx::Format::R16G16_SNORM),
    r16_snorm = static_cast<uint32_t>(gfx::Format::R16_SNORM),

    rgba8_snorm = static_cast<uint32_t>(gfx::Format::R8G8B8A8_SNORM),
    rg8_snorm = static_cast<uint32_t>(gfx::Format::R8G8_SNORM),
    r8_snorm = static_cast<uint32_t>(gfx::Format::R8_SNORM),

    d32_float = static_cast<uint32_t>(gfx::Format::D32_FLOAT),
    d16_unorm = static_cast<uint32_t>(gfx::Format::D16_UNORM),
    d32_float_s8_uint = static_cast<uint32_t>(gfx::Format::D32_FLOAT_S8_UINT),
    r32_float_x32_typeless = static_cast<uint32_t>(gfx::Format::R32_FLOAT_X32_TYPELESS),

    bgra4_unorm = static_cast<uint32_t>(gfx::Format::B4G4R4A4_UNORM),
    b5g6r5_unorm = static_cast<uint32_t>(gfx::Format::B5G6R5_UNORM),
    b5g5r5a1_unorm = static_cast<uint32_t>(gfx::Format::B5G5R5A1_UNORM),

    r9g9b9e5_sharedexp = static_cast<uint32_t>(gfx::Format::R9G9B9E5_SHAREDEXP),
    r10g10b10a2_typeless = static_cast<uint32_t>(gfx::Format::R10G10B10A2_TYPELESS),
    r10g10b10a2_unorm = static_cast<uint32_t>(gfx::Format::R10G10B10A2_UNORM),
    r10g10b10a2_uint = static_cast<uint32_t>(gfx::Format::R10G10B10A2_UINT),
    r11g11b10_float = static_cast<uint32_t>(gfx::Format::R11G11B10_FLOAT),

    bc1_unorm = static_cast<uint32_t>(gfx::Format::BC1_UNORM),
    bc1_unorm_srgb = static_cast<uint32_t>(gfx::Format::BC1_UNORM_SRGB),
    bc2_unorm = static_cast<uint32_t>(gfx::Format::BC2_UNORM),
    bc2_unorm_srgb = static_cast<uint32_t>(gfx::Format::BC2_UNORM_SRGB),
    bc3_unorm = static_cast<uint32_t>(gfx::Format::BC3_UNORM),
    bc3_unorm_srgb = static_cast<uint32_t>(gfx::Format::BC3_UNORM_SRGB),
    bc4_unorm = static_cast<uint32_t>(gfx::Format::BC4_UNORM),
    bc4_snorm = static_cast<uint32_t>(gfx::Format::BC4_SNORM),
    bc5_unorm = static_cast<uint32_t>(gfx::Format::BC5_UNORM),
    bc5_snorm = static_cast<uint32_t>(gfx::Format::BC5_SNORM),
    bc6h_uf16 = static_cast<uint32_t>(gfx::Format::BC6H_UF16),
    bc6h_sf16 = static_cast<uint32_t>(gfx::Format::BC6H_SF16),
    bc7_unorm = static_cast<uint32_t>(gfx::Format::BC7_UNORM),
    bc7_unorm_srgb = static_cast<uint32_t>(gfx::Format::BC7_UNORM_SRGB),

    count,
};

/// Resource format types.
enum class FormatType {
    /// Unknown format.
    unknown,
    /// Typeless formats.
    typeless,
    /// Floating-point formats.
    float_,
    /// Unsigned normalized formats.
    unorm,
    /// Unsigned normalized SRGB formats.
    unorm_srgb,
    /// Signed normalized formats.
    snorm,
    /// Unsigned integer formats.
    uint,
    /// Signed integer formats.
    sint
};

SGL_ENUM_INFO(
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
SGL_ENUM_REGISTER(FormatType);

enum class FormatChannels : uint32_t {
    none = 0x0,
    r = 0x1,
    g = 0x2,
    b = 0x4,
    a = 0x8,
    rg = r | g,
    rgb = r | g | b,
    rgba = r | g | b | a,
};

SGL_ENUM_CLASS_OPERATORS(FormatChannels);
SGL_ENUM_INFO(
    FormatChannels,
    {
        {FormatChannels::none, "none"},
        {FormatChannels::r, "r"},
        {FormatChannels::g, "g"},
        {FormatChannels::b, "b"},
        {FormatChannels::a, "a"},
        {FormatChannels::rg, "rg"},
        {FormatChannels::rgb, "rgb"},
        {FormatChannels::rgba, "rgba"},
    }
);
SGL_ENUM_REGISTER(FormatChannels);

/// Resource format information.
struct SGL_API FormatInfo {
    /// Resource format.
    Format format;
    /// Format name.
    std::string name;
    /// Number of bytes per block (compressed) or pixel (uncompressed).
    uint32_t bytes_per_block;
    /// Number of channels.
    uint32_t channel_count;
    /// Format type (typeless, float, unorm, unorm_srgb, snorm, uint, sint).
    FormatType type;
    /// True if format has a depth component.
    bool is_depth;
    /// True if format has a stencil component.
    bool is_stencil;
    /// True if format is compressed.
    bool is_compressed;
    /// Block width for compressed formats (1 for uncompressed formats).
    uint32_t block_width;
    /// Block height for compressed formats (1 for uncompressed formats).
    uint32_t block_height;
    /// Number of bits per channel.
    std::array<uint32_t, 4> channel_bit_count;
    /// DXGI format.
    uint32_t dxgi_format;
    /// Vulkan format.
    uint32_t vk_format;

    /// True if format has a depth or stencil component.
    bool is_depth_stencil() const { return is_depth || is_stencil; }

    /// True if format is typeless.
    bool is_typeless_format() const { return type == FormatType::typeless; }
    /// True if format is floating point.
    bool is_float_format() const { return type == FormatType::float_; }
    /// True if format is integer.
    bool is_integer_format() const { return type == FormatType::uint || type == FormatType::sint; }
    /// True if format is normalized.
    bool is_normalized_format() const
    {
        return type == FormatType::unorm || type == FormatType::unorm_srgb || type == FormatType::snorm;
    }
    /// True if format is sRGB.
    bool is_srgb_format() const { return type == FormatType::unorm_srgb; }

    /// Get the channels for the format (only for color formats).
    FormatChannels get_channels() const
    {
        FormatChannels channels = FormatChannels::none;
        channels |= channel_count >= 1 ? FormatChannels::r : FormatChannels::none;
        channels |= channel_count >= 2 ? FormatChannels::g : FormatChannels::none;
        channels |= channel_count >= 3 ? FormatChannels::b : FormatChannels::none;
        channels |= channel_count >= 4 ? FormatChannels::a : FormatChannels::none;
        return channels;
    }

    /// Get the number of bits for the specified channels.
    uint32_t get_channel_bits(FormatChannels channels) const
    {
        uint32_t bits = 0;
        bits += (is_set(channels, FormatChannels::r)) ? channel_bit_count[0] : 0;
        bits += (is_set(channels, FormatChannels::g)) ? channel_bit_count[1] : 0;
        bits += (is_set(channels, FormatChannels::b)) ? channel_bit_count[2] : 0;
        bits += (is_set(channels, FormatChannels::a)) ? channel_bit_count[3] : 0;
        return bits;
    }

    /// Check if all channels have the same number of bits.
    bool has_equal_channel_bits() const
    {
        uint32_t bits = channel_bit_count[0];
        for (uint32_t i = 1; i < channel_count; ++i)
            if (channel_bit_count[i] != bits)
                return false;
        return true;
    }

    std::string to_string() const;
};

SGL_API const FormatInfo& get_format_info(Format format);

namespace detail {
    template<typename T>
    struct HostTypeToFormat {
        static constexpr Format value = Format::unknown;
    };

#define SGL_HOST_TYPE_TO_FORMAT(type, format)                                                                          \
    template<>                                                                                                         \
    struct HostTypeToFormat<type> {                                                                                    \
        static constexpr Format value = format;                                                                        \
    }

    SGL_HOST_TYPE_TO_FORMAT(int8_t, Format::r8_sint);
    SGL_HOST_TYPE_TO_FORMAT(uint8_t, Format::r8_uint);
    SGL_HOST_TYPE_TO_FORMAT(int16_t, Format::r16_sint);
    SGL_HOST_TYPE_TO_FORMAT(uint16_t, Format::r16_uint);
    SGL_HOST_TYPE_TO_FORMAT(int, Format::r32_sint);
    SGL_HOST_TYPE_TO_FORMAT(int2, Format::rg32_sint);
    SGL_HOST_TYPE_TO_FORMAT(int4, Format::rgba32_sint);
    SGL_HOST_TYPE_TO_FORMAT(uint, Format::r32_uint);
    SGL_HOST_TYPE_TO_FORMAT(uint2, Format::rg32_uint);
    SGL_HOST_TYPE_TO_FORMAT(uint4, Format::rgba32_uint);
    SGL_HOST_TYPE_TO_FORMAT(float16_t, Format::r16_float);
    SGL_HOST_TYPE_TO_FORMAT(float16_t2, Format::rg16_float);
    SGL_HOST_TYPE_TO_FORMAT(float16_t4, Format::rgba16_float);
    SGL_HOST_TYPE_TO_FORMAT(float, Format::r32_float);
    SGL_HOST_TYPE_TO_FORMAT(float2, Format::rg32_float);
    SGL_HOST_TYPE_TO_FORMAT(float3, Format::rgb32_float);
    SGL_HOST_TYPE_TO_FORMAT(float4, Format::rgba32_float);

#undef SGL_HOST_TYPE_TO_FORMAT
} // namespace detail

template<typename T>
inline constexpr Format host_type_to_format()
{
    return detail::HostTypeToFormat<T>::value;
}

// Manually define the struct that SGL_ENUM_INFO(Format) would generate so we
// can use the existing table of resource formats.
struct Format_info {
    static const std::string& name()
    {
        static const std::string name = "Format";
        return name;
    }

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
SGL_ENUM_REGISTER(Format);

} // namespace sgl
