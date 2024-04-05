// SPDX-License-Identifier: Apache-2.0

#include "texture_loader.h"

#include "sgl/device/device.h"
#include "sgl/device/command.h"
#include "sgl/device/blit.h"

#include "sgl/core/error.h"
#include "sgl/core/bitmap.h"

#include <map>

namespace sgl {

static constexpr size_t BATCH_SIZE = 32;

TextureLoader::Options::Options() { }

TextureLoader::TextureLoader(ref<Device> device)
    : m_device(std::move(device))
{
    m_blitter = ref(new Blitter(m_device));
}

TextureLoader::~TextureLoader() = default;

ref<Texture> TextureLoader::load_texture(const Bitmap* bitmap, const Options& options)
{
    ref<CommandBuffer> command_buffer = m_device->create_command_buffer();
    ref<Texture> texture = load_from_bitmap(command_buffer, bitmap, options);
    command_buffer->submit();
    return texture;
}

ref<Texture> TextureLoader::load_texture(const std::filesystem::path& path, const Options& options)
{
    ref<Bitmap> bitmap = ref(new Bitmap(path));
    return load_texture(bitmap, options);
}

std::vector<ref<Texture>> TextureLoader::load_textures(std::span<const Bitmap*> bitmaps, const Options& options)
{
    std::vector<ref<Texture>> textures(bitmaps.size());
    ref<CommandBuffer> command_buffer = m_device->create_command_buffer();
    for (size_t i = 0; i < bitmaps.size(); ++i) {
        textures[i] = load_from_bitmap(command_buffer, bitmaps[i], options);
        if (i && (i % BATCH_SIZE == 0)) {
            command_buffer->submit();
            m_device->run_garbage_collection();
            command_buffer = m_device->create_command_buffer();
        }
    }
    command_buffer->submit();
    return textures;
}

std::vector<ref<Texture>> TextureLoader::load_textures(std::span<std::filesystem::path> paths, const Options& options)
{
    std::vector<ref<Bitmap>> bitmaps = Bitmap::read_multiple(paths);
    std::vector<const Bitmap*> bitmap_ptrs(bitmaps.size());
    for (size_t i = 0; i < bitmaps.size(); ++i)
        bitmap_ptrs[i] = bitmaps[i].get();
    return load_textures(bitmap_ptrs, options);
}

ref<Texture> TextureLoader::load_texture_array(std::span<const Bitmap*> bitmaps, const Options& options)
{
    if (bitmaps.empty())
        return nullptr;

    auto pixel_format = bitmaps[0]->pixel_format();
    auto component_type = bitmaps[0]->component_type();
    auto width = bitmaps[0]->width();
    auto height = bitmaps[0]->height();

    for (size_t i = 1; i < bitmaps.size(); ++i) {
        if (bitmaps[i]->pixel_format() != pixel_format || bitmaps[i]->component_type() != component_type
            || bitmaps[i]->width() != width || bitmaps[i]->height() != height)
            SGL_THROW("All images need to have the same format and dimensions.");
    }

    return load_array_from_bitmaps(bitmaps, options);
}

ref<Texture> TextureLoader::load_texture_array(std::span<std::filesystem::path> paths, const Options& options)
{
    std::vector<ref<Bitmap>> bitmaps = Bitmap::read_multiple(paths);
    std::vector<const Bitmap*> bitmap_ptrs(bitmaps.size());
    for (size_t i = 0; i < bitmaps.size(); ++i)
        bitmap_ptrs[i] = bitmaps[i].get();
    return load_texture_array(bitmap_ptrs, options);
}

std::pair<Format, bool> TextureLoader::determine_texture_format(const Bitmap* bitmap, const Options& options) const
{
    SGL_ASSERT(bitmap != nullptr);

    using PixelFormat = Bitmap::PixelFormat;
    using ComponentType = Bitmap::ComponentType;

    enum class FormatFlags {
        none = 0,
        normalized = 1,
        srgb = 2,
    };

    auto make_key = [](PixelFormat pixel_format, ComponentType component_type, FormatFlags flags = FormatFlags::none
                    ) constexpr -> uint32_t
    {
        static_assert(Bitmap::PIXEL_FORMAT_COUNT <= 8);
        static_assert(Struct::TYPE_COUNT <= 16);
        uint32_t key = 0;
        key |= (1 << uint32_t(pixel_format));
        key |= (1 << uint32_t(component_type)) << 8;
        key |= uint32_t(flags) << 24;
        return key;
    };

    static const std::map<uint32_t, Format> FORMAT_TABLE{
        // PixelFormat::r
        {make_key(PixelFormat::r, ComponentType::int8), Format::r8_sint},
        {make_key(PixelFormat::r, ComponentType::int8, FormatFlags::normalized), Format::r8_snorm},
        {make_key(PixelFormat::r, ComponentType::int16), Format::r16_sint},
        {make_key(PixelFormat::r, ComponentType::int16, FormatFlags::normalized), Format::r16_snorm},
        {make_key(PixelFormat::r, ComponentType::int32), Format::r32_sint},
        {make_key(PixelFormat::r, ComponentType::uint8), Format::r8_uint},
        {make_key(PixelFormat::r, ComponentType::uint8, FormatFlags::normalized), Format::r8_unorm},
        {make_key(PixelFormat::r, ComponentType::uint16), Format::r16_uint},
        {make_key(PixelFormat::r, ComponentType::uint16, FormatFlags::normalized), Format::r16_unorm},
        {make_key(PixelFormat::r, ComponentType::uint32), Format::r32_uint},
        {make_key(PixelFormat::r, ComponentType::float16), Format::r16_float},
        {make_key(PixelFormat::r, ComponentType::float32), Format::r32_float},
        // PixelFormat::rg,
        {make_key(PixelFormat::rg, ComponentType::int8), Format::rg8_sint},
        {make_key(PixelFormat::rg, ComponentType::int8, FormatFlags::normalized), Format::rg8_snorm},
        {make_key(PixelFormat::rg, ComponentType::int16), Format::rg16_sint},
        {make_key(PixelFormat::rg, ComponentType::int16, FormatFlags::normalized), Format::rg16_snorm},
        {make_key(PixelFormat::rg, ComponentType::int32), Format::rg32_sint},
        {make_key(PixelFormat::rg, ComponentType::uint8), Format::rg8_uint},
        {make_key(PixelFormat::rg, ComponentType::uint8, FormatFlags::normalized), Format::rg8_unorm},
        {make_key(PixelFormat::rg, ComponentType::uint16), Format::rg16_uint},
        {make_key(PixelFormat::rg, ComponentType::uint16, FormatFlags::normalized), Format::rg16_unorm},
        {make_key(PixelFormat::rg, ComponentType::uint32), Format::rg32_uint},
        {make_key(PixelFormat::rg, ComponentType::float16), Format::rg16_float},
        {make_key(PixelFormat::rg, ComponentType::float32), Format::rg32_float},
        // PixelFormat::rgb,
        {make_key(PixelFormat::rgb, ComponentType::int32), Format::rgb32_sint},
        {make_key(PixelFormat::rgb, ComponentType::uint32), Format::rgb32_uint},
        {make_key(PixelFormat::rgb, ComponentType::float32), Format::rgb32_float},
        // PixelFormat::rgba,
        {make_key(PixelFormat::rgba, ComponentType::int8), Format::rgba8_sint},
        {make_key(PixelFormat::rgba, ComponentType::int8, FormatFlags::normalized), Format::rgba8_snorm},
        {make_key(PixelFormat::rgba, ComponentType::int16), Format::rgba16_sint},
        {make_key(PixelFormat::rgba, ComponentType::int16, FormatFlags::normalized), Format::rgba16_snorm},
        {make_key(PixelFormat::rgba, ComponentType::int32), Format::rgba32_sint},
        {make_key(PixelFormat::rgba, ComponentType::uint8), Format::rgba8_uint},
        {make_key(PixelFormat::rgba, ComponentType::uint8, FormatFlags::normalized), Format::rgba8_unorm},
        {make_key(PixelFormat::rgba, ComponentType::uint8, FormatFlags::srgb), Format::rgba8_unorm_srgb},
        {make_key(PixelFormat::rgba, ComponentType::uint16), Format::rgba16_uint},
        {make_key(PixelFormat::rgba, ComponentType::uint16, FormatFlags::normalized), Format::rgba16_unorm},
        {make_key(PixelFormat::rgba, ComponentType::uint32), Format::rgba32_uint},
        {make_key(PixelFormat::rgba, ComponentType::float16), Format::rgba16_float},
        {make_key(PixelFormat::rgba, ComponentType::float32), Format::rgba32_float},
    };

    PixelFormat pixel_format = bitmap->pixel_format();
    if (pixel_format == PixelFormat::y)
        pixel_format = PixelFormat::r;
    ComponentType component_type = bitmap->component_type();
    FormatFlags format_flags = FormatFlags::none;
    if (options.load_as_normalized && Struct::is_integer(component_type))
        format_flags = FormatFlags::normalized;

    // Check if bitmap is RGB and we can convert to RGBA.
    bool convert_to_rgba = false;
    if (options.extend_alpha && pixel_format == PixelFormat::rgb) {
        bool rgb_format_supported
            = FORMAT_TABLE.find(make_key(PixelFormat::rgb, component_type, format_flags)) != FORMAT_TABLE.end();
        bool rgba_format_supported
            = FORMAT_TABLE.find(make_key(PixelFormat::rgba, component_type, format_flags)) != FORMAT_TABLE.end();
        if (!rgb_format_supported && rgba_format_supported) {
            convert_to_rgba = true;
            pixel_format = PixelFormat::rgba;
        }
    }

    // Use sRGB format if requested and supported.
    if (options.load_as_srgb && pixel_format == PixelFormat::rgba && component_type == ComponentType::uint8
        && bitmap->srgb_gamma())
        format_flags = FormatFlags::srgb;

    // Find texture format.
    auto it = FORMAT_TABLE.find(make_key(pixel_format, component_type, format_flags));
    if (it == FORMAT_TABLE.end())
        SGL_THROW("Unsupported bitmap format: {} {}", pixel_format, component_type);

    return {it->second, convert_to_rgba};
}

ref<Texture>
TextureLoader::load_from_bitmap(CommandBuffer* command_buffer, const Bitmap* bitmap, const Options& options)
{
    SGL_CHECK_NOT_NULL(bitmap);

    const auto [format, convert_to_rgba] = determine_texture_format(bitmap, options);
    ref<Bitmap> rgba_bitmap;
    if (convert_to_rgba) {
        log_debug("Converting {} RGB bitmap to RGBA.", bitmap->component_type());
        rgba_bitmap = bitmap->convert(Bitmap::PixelFormat::rgba, bitmap->component_type(), bitmap->srgb_gamma());
        bitmap = rgba_bitmap;
    }

    bool allocate_mips = options.allocate_mips || options.generate_mips;

    ResourceUsage usage = options.usage;
    if (options.generate_mips)
        usage |= ResourceUsage::render_target;

    ref<Texture> texture = m_device->create_texture({
        .format = format,
        .width = bitmap->width(),
        .height = bitmap->height(),
        .mip_count = allocate_mips ? 0u : 1u,
        .usage = usage,
    });

    SubresourceData subresource_data{
        .data = bitmap->data(),
        .row_pitch = bitmap->width() * bitmap->bytes_per_pixel(),
    };

    command_buffer->upload_texture_data(texture, 0, subresource_data);
    if (options.generate_mips) {
        m_blitter->generate_mips(command_buffer, texture);
        texture->invalidate_views();
    }

    return texture;
}

ref<Texture> TextureLoader::load_array_from_bitmaps(std::span<const Bitmap*> bitmaps, const Options& options)
{
    if (bitmaps.empty())
        return nullptr;

    const auto [format, convert_to_rgba] = determine_texture_format(bitmaps[0], options);
    std::vector<ref<Bitmap>> rgba_bitmaps;
    std::vector<const Bitmap*> rgba_bitmap_ptrs;
    if (convert_to_rgba) {
        log_debug("Converting {} RGB bitmaps to RGBA.", bitmaps[0]->component_type());
        rgba_bitmaps.resize(bitmaps.size());
        rgba_bitmap_ptrs.resize(bitmaps.size());
        auto component_type = bitmaps[0]->component_type();
        auto srgb_gamma = bitmaps[0]->srgb_gamma();
        for (size_t i = 0; i < bitmaps.size(); ++i) {
            rgba_bitmaps[i] = bitmaps[i]->convert(Bitmap::PixelFormat::rgba, component_type, srgb_gamma);
            rgba_bitmap_ptrs[i] = rgba_bitmaps[i];
        }
        bitmaps = rgba_bitmap_ptrs;
    }

    bool allocate_mips = options.allocate_mips || options.generate_mips;

    ResourceUsage usage = options.usage;
    if (options.generate_mips)
        usage |= ResourceUsage::render_target;

    ref<Texture> texture = m_device->create_texture({
        .format = format,
        .width = bitmaps[0]->width(),
        .height = bitmaps[0]->height(),
        .array_size = narrow_cast<uint32_t>(bitmaps.size()),
        .mip_count = allocate_mips ? 0u : 1u,
        .usage = usage,
    });

    ref<CommandBuffer> command_buffer;

    for (size_t i = 0; i < bitmaps.size(); ++i) {
        if (i % BATCH_SIZE == 0) {
            if (command_buffer)
                command_buffer->submit();
            m_device->run_garbage_collection();
            command_buffer = m_device->create_command_buffer();
        }

        uint32_t subresource = texture->get_subresource_index(0, narrow_cast<uint32_t>(i));
        SubresourceData subresource_data{
            .data = bitmaps[i]->data(),
            .row_pitch = bitmaps[i]->width() * bitmaps[i]->bytes_per_pixel(),
        };
        command_buffer->upload_texture_data(texture, subresource, subresource_data);

        if (options.generate_mips)
            m_blitter->generate_mips(command_buffer, texture, narrow_cast<uint32_t>(i));
    }

    command_buffer->submit();

    if (options.generate_mips)
        texture->invalidate_views();

    return texture;
}

} // namespace sgl
