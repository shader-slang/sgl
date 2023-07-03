#include "testing.h"
#include "rhi/formats.h"
#include "rhi/helpers.h"

using namespace kali;

TEST_SUITE_BEGIN("formats");

TEST_CASE("host_type_to_format")
{
    CHECK_EQ(host_type_to_format<int8_t>(), Format::r8_sint);
    CHECK_EQ(host_type_to_format<uint8_t>(), Format::r8_uint);
    CHECK_EQ(host_type_to_format<int16_t>(), Format::r16_sint);
    CHECK_EQ(host_type_to_format<uint16_t>(), Format::r16_uint);
    CHECK_EQ(host_type_to_format<int>(), Format::r32_sint);
    CHECK_EQ(host_type_to_format<int2>(), Format::rg32_sint);
    CHECK_EQ(host_type_to_format<int4>(), Format::rgba32_sint);
    CHECK_EQ(host_type_to_format<uint>(), Format::r32_uint);
    CHECK_EQ(host_type_to_format<uint2>(), Format::rg32_uint);
    CHECK_EQ(host_type_to_format<uint4>(), Format::rgba32_uint);
    CHECK_EQ(host_type_to_format<float>(), Format::r32_float);
    CHECK_EQ(host_type_to_format<float2>(), Format::rg32_float);
    CHECK_EQ(host_type_to_format<float3>(), Format::rgb32_float);
    CHECK_EQ(host_type_to_format<float4>(), Format::rgba32_float);
}

TEST_CASE("validate_format_infos")
{
    auto format_type_to_slang_type = [](FormatType type) -> SlangScalarType
    {
        switch (type) {
        case FormatType::unknown: return SLANG_SCALAR_TYPE_NONE;
        case FormatType::typeless: return SLANG_SCALAR_TYPE_VOID;
        case FormatType::float_: return SLANG_SCALAR_TYPE_FLOAT32;
        case FormatType::unorm: return SLANG_SCALAR_TYPE_FLOAT32;
        case FormatType::unorm_srgb: return SLANG_SCALAR_TYPE_FLOAT32;
        case FormatType::snorm: return SLANG_SCALAR_TYPE_FLOAT32;
        case FormatType::uint: return SLANG_SCALAR_TYPE_UINT32;
        case FormatType::sint: return SLANG_SCALAR_TYPE_INT32;
        default: return SLANG_SCALAR_TYPE_NONE;
        }
    };

    for (uint32_t i = 0; i < uint32_t(Format::count); ++i) {
        const auto& info = get_format_info(Format(i));
        gfx::FormatInfo gfx_info;
        SLANG_CALL(gfx::gfxGetFormatInfo(gfx::Format(i), &gfx_info));

        CAPTURE(info.name);
        // CHECK_EQ(format_type_to_slang_type(info.type), gfx_info.channelType);
        CHECK_EQ(info.bytes_per_block, gfx_info.blockSizeInBytes);

        // TODO gfx has these incorrect with 4 channels
        if (info.format != Format::bc1_unorm && info.format != Format::bc1_unorm_srgb)
            CHECK_EQ(info.channel_count, gfx_info.channelCount);

        CHECK_EQ(info.block_width * info.block_height, gfx_info.pixelsPerBlock);
        CHECK_EQ(info.block_width, gfx_info.blockWidth);
        CHECK_EQ(info.block_height, gfx_info.blockHeight);
    }
}

TEST_SUITE_END();
