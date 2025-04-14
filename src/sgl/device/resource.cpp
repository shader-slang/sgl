// SPDX-License-Identifier: Apache-2.0

#include "resource.h"

#include "sgl/device/device.h"
#include "sgl/device/command.h"
#include "sgl/device/helpers.h"
#include "sgl/device/native_handle_traits.h"
#include "sgl/device/cuda_utils.h"

#include "sgl/core/config.h"
#include "sgl/core/error.h"
#include "sgl/core/string.h"
#include "sgl/core/maths.h"
#include "sgl/core/bitmap.h"
#include "sgl/core/static_vector.h"

#include "sgl/stl/bit.h" // Replace with <bit> when available on all platforms.

namespace sgl {

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

SubresourceLayout layout_from_rhilayout(const rhi::SubresourceLayout& rhi_layout)
{
    return {
        .size = {rhi_layout.size.width, rhi_layout.size.height, rhi_layout.size.depth},
        .col_pitch = rhi_layout.colPitch,
        .row_pitch = rhi_layout.rowPitch,
        .slice_pitch = rhi_layout.slicePitch,
        .size_in_bytes = rhi_layout.sizeInBytes,
        .block_width = rhi_layout.blockWidth,
        .block_height = rhi_layout.blockHeight,
        .row_count = rhi_layout.rowCount,
    };
}

// ----------------------------------------------------------------------------
// Resource
// ----------------------------------------------------------------------------

Resource::Resource(ref<Device> device)
    : DeviceResource(std::move(device))
{
}

Resource::~Resource() { }

NativeHandle Resource::get_native_handle() const
{
    rhi::NativeHandle rhi_handle = {};
    SLANG_CALL(rhi_resource()->getNativeHandle(&rhi_handle));
    return NativeHandle(rhi_handle);
}

// ----------------------------------------------------------------------------
// Buffer
// ----------------------------------------------------------------------------

Buffer::Buffer(ref<Device> device, BufferDesc desc)
    : Resource(std::move(device))
    , m_desc(std::move(desc))
{
    SGL_CHECK(m_desc.size == 0 || m_desc.element_count == 0, "Only one of 'size' or 'element_count' must be set.");
    SGL_CHECK(
        m_desc.struct_size == 0 || m_desc.struct_type == nullptr,
        "Only one of 'struct_size' or 'struct_type' must be set."
    );

    // Derive buffer size from initial data.
    if (m_desc.size == 0 && m_desc.element_count == 0 && m_desc.data && m_desc.data_size > 0)
        m_desc.size = m_desc.data_size;

    // Derive struct size from struct type.
    if (m_desc.struct_type) {
        ref<const TypeLayoutReflection> type = m_desc.struct_type->unwrap_array();
        if (type->element_type_layout())
            type = type->element_type_layout();
        SGL_CHECK(type, "Invalid struct type.");
        m_desc.struct_size = type->stride();
        m_desc.struct_type = nullptr;
    }

    // Derive buffer size from element count and struct size.
    SGL_CHECK(
        m_desc.element_count == 0 || m_desc.struct_size > 0,
        "'element_count' can only be used with 'struct_size' or 'struct_type' set."
    );
    if (m_desc.element_count > 0) {
        m_desc.size = m_desc.element_count * m_desc.struct_size;
        m_desc.element_count = 0;
    }

    // TODO check init_data size
    SGL_ASSERT(m_desc.size > 0);

    SGL_CHECK(
        (m_desc.data == nullptr && m_desc.data_size == 0) || m_desc.data_size == m_desc.size,
        "Invalid data size (got {} bytes, expected {} bytes)",
        m_desc.data_size,
        m_desc.size
    );

    rhi::BufferDesc rhi_desc;
    rhi_desc.size = static_cast<uint64_t>(m_desc.size);
    rhi_desc.elementSize = static_cast<uint32_t>(m_desc.struct_size);
    rhi_desc.format = static_cast<rhi::Format>(m_desc.format);
    rhi_desc.memoryType = static_cast<rhi::MemoryType>(m_desc.memory_type);
    rhi_desc.usage = static_cast<rhi::BufferUsage>(m_desc.usage);
    rhi_desc.defaultState = static_cast<rhi::ResourceState>(m_desc.default_state);
    rhi_desc.label = m_desc.label.empty() ? nullptr : m_desc.label.c_str();

    if (m_desc.memory_type == MemoryType::device_local)
        rhi_desc.usage |= rhi::BufferUsage::CopySource | rhi::BufferUsage::CopyDestination;

    SLANG_CALL(m_device->rhi_device()->createBuffer(rhi_desc, nullptr, m_rhi_buffer.writeRef()));

    // Upload init data.
    if (m_desc.data)
        set_data(m_desc.data, m_desc.data_size);

    // Clear initial data fields in desc.
    m_desc.data = nullptr;
    m_desc.data_size = 0;
}

Buffer::~Buffer()
{
    m_cuda_memory.reset();
}

void* Buffer::map() const
{
    SGL_ASSERT(m_desc.memory_type != MemoryType::device_local);
    SGL_ASSERT(m_mapped_ptr == nullptr);
    rhi::CpuAccessMode mode
        = m_desc.memory_type == MemoryType::upload ? rhi::CpuAccessMode::Write : rhi::CpuAccessMode::Read;
    SLANG_CALL(m_device->rhi_device()->mapBuffer(m_rhi_buffer, mode, &m_mapped_ptr));
    return m_mapped_ptr;
}

void Buffer::unmap() const
{
    SGL_ASSERT(m_desc.memory_type != MemoryType::device_local);
    SGL_ASSERT(m_mapped_ptr != nullptr);
    SLANG_CALL(m_device->rhi_device()->unmapBuffer(m_rhi_buffer));
    m_mapped_ptr = nullptr;
}

void* Buffer::cuda_memory() const
{
    SGL_CHECK(m_device->supports_cuda_interop(), "Device does not support CUDA interop");
    if (!m_cuda_memory)
        m_cuda_memory = make_ref<cuda::ExternalMemory>(this);
    return m_cuda_memory->mapped_data();
}

void Buffer::set_data(const void* data, size_t size, DeviceOffset offset)
{
    SGL_CHECK(
        offset + size <= m_desc.size,
        "'offset' ({}) and 'size' ({}) don't fit the buffer size {}.",
        offset,
        size,
        m_desc.size
    );

    switch (m_desc.memory_type) {
    case MemoryType::device_local:
        m_device->upload_buffer_data(this, offset, size, data);
        break;
    case MemoryType::upload: {
        bool was_mapped = is_mapped();
        uint8_t* dst = map<uint8_t>() + offset;
        std::memcpy(dst, data, size);
        if (!was_mapped)
            unmap();
        // TODO invalidate views
        break;
    }
    case MemoryType::read_back:
        SGL_THROW("Cannot write data to buffer with memory type 'read_back'.");
    }
}

void Buffer::get_data(void* data, size_t size, DeviceOffset offset)
{
    SGL_CHECK(
        offset + size <= m_desc.size,
        "'offset' ({}) and 'size' ({}) don't fit the buffer size {}.",
        offset,
        size,
        m_desc.size
    );

    switch (m_desc.memory_type) {
    case MemoryType::device_local:
        m_device->read_buffer_data(this, data, size, offset);
        break;
    case MemoryType::upload:
        SGL_THROW("Cannot read data from buffer with memory type 'upload'.");
    case MemoryType::read_back: {
        bool was_mapped = is_mapped();
        const uint8_t* src = map<uint8_t>() + offset;
        std::memcpy(data, src, size);
        if (!was_mapped)
            unmap();
        break;
    }
    }
}

ref<BufferView> Buffer::create_view(BufferViewDesc desc)
{
    return m_device->create_buffer_view(this, std::move(desc));
}

NativeHandle Buffer::get_shared_handle() const
{
    rhi::NativeHandle rhi_handle = {};
    SLANG_CALL(m_rhi_buffer->getSharedHandle(&rhi_handle));
    return NativeHandle(rhi_handle);
}

DeviceResource::MemoryUsage Buffer::memory_usage() const
{
    return {.device = m_desc.size};
}

std::string Buffer::to_string() const
{
    return fmt::format(
        "Buffer(\n"
        "  device = {},\n"
        "  size = {},\n"
        "  struct_size = {},\n"
        "  format = {},\n"
        "  usage = {},\n"
        "  memory_type = {},\n"
        "  memory_usage = {},\n"
        "  label = {}\n"
        ")",
        m_device,
        m_desc.size,
        m_desc.struct_size,
        m_desc.format,
        m_desc.usage,
        m_desc.memory_type,
        string::format_byte_size(memory_usage().device),
        m_desc.label
    );
}

// ----------------------------------------------------------------------------
// BufferView
// ----------------------------------------------------------------------------

BufferView::BufferView(ref<Device> device, ref<Buffer> buffer, BufferViewDesc desc)
    : DeviceResource(std::move(device))
    , m_buffer(std::move(buffer))
    , m_desc(std::move(desc))
{
    size_t size = m_buffer->size();
    SGL_CHECK(m_desc.range.offset < size, "'offset' out of range");
    SGL_CHECK(
        m_desc.range.size == BufferRange::ALL || m_desc.range.offset + m_desc.range.size <= size,
        "'size' out of range"
    );
}

NativeHandle BufferView::get_native_handle() const
{
    // TODO
    return {};
}

std::string BufferView::to_string() const
{
    return fmt::format(
        "BufferView(\n"
        "  device = {},\n"
        "  buffer = {},\n"
        "  format = {},\n"
        "  range = {},\n"
        "  label = {}\n"
        ")",
        m_device,
        string::indent(m_buffer->to_string()),
        m_desc.format,
        m_desc.range.to_string(),
        m_desc.label
    );
}

// ----------------------------------------------------------------------------
// Texture
// ----------------------------------------------------------------------------

inline void process_texture_desc(TextureDesc& desc)
{
    SGL_CHECK(desc.format != Format::undefined, "Invalid texture format.");

    switch (desc.type) {
    case TextureType::texture_1d:
    case TextureType::texture_1d_array:
        SGL_CHECK(
            desc.width > 0 && desc.height == 1 && desc.depth == 1,
            "Invalid dimensions (width={}, height={}, depth={}) for 1D texture.",
            desc.width,
            desc.height,
            desc.depth
        );
        break;
    case TextureType::texture_2d:
    case TextureType::texture_2d_array:
    case TextureType::texture_2d_ms:
    case TextureType::texture_2d_ms_array:
        SGL_CHECK(
            desc.width > 0 && desc.height > 0 && desc.depth == 1,
            "Invalid dimensions (width={}, height={}, depth={}) for 2D texture.",
            desc.width,
            desc.height,
            desc.depth
        );
        break;
    case TextureType::texture_3d:
        SGL_CHECK(
            desc.width > 0 && desc.height > 0 && desc.depth > 0,
            "Invalid dimensions (width={}, height={}, depth={}) for 3D texture.",
            desc.width,
            desc.height,
            desc.depth
        );
        break;
    case TextureType::texture_cube:
    case TextureType::texture_cube_array:
        SGL_CHECK(
            desc.width > 0 && desc.height > 0 && desc.depth == 1,
            "Invalid dimensions (width={}, height={}, depth={}) for cube texture.",
            desc.width,
            desc.height,
            desc.depth
        );
        break;
    }

    switch (desc.type) {
    case TextureType::texture_1d:
    case TextureType::texture_2d:
    case TextureType::texture_2d_ms:
    case TextureType::texture_3d:
    case TextureType::texture_cube:
        SGL_CHECK(desc.array_length == 1, "Invalid array length ({}) for non-array texture.", desc.array_length);
        break;
    case TextureType::texture_1d_array:
    case TextureType::texture_2d_array:
    case TextureType::texture_2d_ms_array:
    case TextureType::texture_cube_array:
        SGL_CHECK(desc.array_length >= 1, "Invalid array length ({}) for array texture.", desc.array_length);
        break;
    }

    if (desc.type == TextureType::texture_2d_ms || desc.type == TextureType::texture_2d_ms_array) {
        SGL_CHECK(desc.sample_count >= 1, "Invalid sample count ({}) for multisampled texture.", desc.sample_count);
    } else {
        SGL_CHECK(desc.sample_count == 1, "Invalid sample count ({}) for non-multisampled texture.", desc.sample_count);
    }

    if (desc.mip_count == ALL_MIP_LEVELS)
        desc.mip_count = stdx::bit_width(std::max({desc.width, desc.height, desc.depth}));
}

Texture::Texture(ref<Device> device, TextureDesc desc)
    : Resource(std::move(device))
    , m_desc(std::move(desc))
{
    process_texture_desc(m_desc);

    rhi::TextureDesc rhi_desc;
    rhi_desc.type = static_cast<rhi::TextureType>(m_desc.type);
    rhi_desc.memoryType = static_cast<rhi::MemoryType>(m_desc.memory_type);
    rhi_desc.usage = static_cast<rhi::TextureUsage>(m_desc.usage);
    rhi_desc.defaultState = static_cast<rhi::ResourceState>(m_desc.default_state);
    rhi_desc.size.width = static_cast<rhi::Size>(m_desc.width);
    rhi_desc.size.height = static_cast<rhi::Size>(m_desc.height);
    rhi_desc.size.depth = static_cast<rhi::Size>(m_desc.depth);
    rhi_desc.arrayLength = m_desc.array_length;
    rhi_desc.mipLevelCount = m_desc.mip_count;
    rhi_desc.format = static_cast<rhi::Format>(m_desc.format);
    rhi_desc.sampleCount = m_desc.sample_count;
    rhi_desc.sampleQuality = m_desc.sample_quality;
    rhi_desc.optimalClearValue = nullptr; // TODO(slang-rhi)
    rhi_desc.label = m_desc.label.empty() ? nullptr : m_desc.label.c_str();

    if (m_desc.memory_type == MemoryType::device_local)
        rhi_desc.usage |= rhi::TextureUsage::CopySource | rhi::TextureUsage::CopyDestination;

    SLANG_CALL(m_device->rhi_device()->createTexture(rhi_desc, nullptr, m_rhi_texture.writeRef()));

    // Upload init data.
    if (!m_desc.data.empty()) {
        for (uint32_t subresource = 0; subresource < m_desc.data.size(); ++subresource) {
            uint32_t layer = subresource / m_desc.mip_count;
            uint32_t mip_level = subresource % m_desc.mip_count;

            SubresourceData subresource_data = m_desc.data[subresource];
            if (subresource_data.row_pitch == 0 && subresource_data.slice_pitch == 0 && subresource_data.size == 0) {
                SubresourceLayout subresource_layout = get_subresource_layout(0, 1);
                subresource_data.row_pitch = subresource_layout.row_pitch;
                subresource_data.slice_pitch = subresource_layout.slice_pitch;
                subresource_data.size = subresource_layout.size_in_bytes;
            }
            SGL_CHECK(subresource_data.row_pitch > 0, "Invalid row pitch.");
            SGL_CHECK(subresource_data.slice_pitch > 0, "Invalid slice pitch.");
            SGL_CHECK(subresource_data.size > 0, "Invalid size.");

            set_subresource_data(layer, mip_level, m_desc.data[subresource]);
        }
        if (m_desc.mip_count > 1) {
            // TODO generate mip maps
        }
    }

    // Clear initial data field in desc.
    m_desc.data = {};
}

Texture::Texture(ref<Device> device, TextureDesc desc, rhi::ITexture* resource)
    : Resource(std::move(device))
    , m_desc(std::move(desc))
{
    process_texture_desc(m_desc);

    m_rhi_texture = resource;
}

Texture::~Texture() { }

SubresourceLayout Texture::get_subresource_layout(uint32_t mip_level, uint32_t row_alignment) const
{
    SGL_CHECK_LT(mip_level, mip_count());

    rhi::SubresourceLayout rhi_layout;
    SLANG_CALL(m_rhi_texture->getSubresourceLayout(mip_level, row_alignment, &rhi_layout));

    return layout_from_rhilayout(rhi_layout);
}

void Texture::set_subresource_data(uint32_t layer, uint32_t mip_level, SubresourceData subresource_data)
{
    SGL_CHECK_LT(layer, layer_count());
    SGL_CHECK_LT(mip_level, mip_count());

    m_device->upload_texture_data(this, layer, mip_level, subresource_data);
}

OwnedSubresourceData Texture::get_subresource_data(uint32_t layer, uint32_t mip_level) const
{
    SGL_CHECK_LT(layer, layer_count());
    SGL_CHECK_LT(mip_level, mip_count());

    return m_device->read_texture_data(this, layer, mip_level);
}

ref<TextureView> Texture::create_view(TextureViewDesc desc)
{
    return m_device->create_texture_view(this, std::move(desc));
}

NativeHandle Texture::get_shared_handle() const
{
    rhi::NativeHandle rhi_handle = {};
    SLANG_CALL(m_rhi_texture->getSharedHandle(&rhi_handle));
    return NativeHandle(rhi_handle);
}

DeviceResource::MemoryUsage Texture::memory_usage() const
{
    rhi::Size size = 0, alignment = 0;
    SLANG_CALL(m_device->rhi_device()->getTextureAllocationInfo(m_rhi_texture->getDesc(), &size, &alignment));
    return {.device = size};
}

ref<Bitmap> Texture::to_bitmap(uint32_t layer, uint32_t mip_level) const
{
    SGL_CHECK_LT(layer, layer_count());
    SGL_CHECK_LT(mip_level, mip_count());
    SGL_CHECK(
        m_desc.type == TextureType::texture_2d || m_desc.type == TextureType::texture_2d_array,
        "Cannot convert non-2D texture to bitmap."
    );

    const FormatInfo& info = get_format_info(m_desc.format);
    if (info.is_compressed)
        SGL_THROW("Cannot convert compressed texture to bitmap.");
    if (info.is_depth_stencil())
        SGL_THROW("Cannot convert depth/stencil texture to bitmap.");
    if (!info.has_equal_channel_bits())
        SGL_THROW("Cannot convert texture with unequal channel bits to bitmap.");
    uint32_t channel_bit_count = info.channel_bit_count[0];
    if (channel_bit_count != 8 && channel_bit_count != 16 && channel_bit_count != 32 && channel_bit_count != 64)
        SGL_THROW("Cannot convert texture with non-8/16/32/64 bit channels to bitmap.");

    static const std::map<uint32_t, Bitmap::PixelFormat> pixel_format_map = {
        {1, Bitmap::PixelFormat::r},
        {2, Bitmap::PixelFormat::rg},
        {3, Bitmap::PixelFormat::rgb},
        {4, Bitmap::PixelFormat::rgba},
    };

    static const std::map<FormatType, std::map<uint32_t, Bitmap::ComponentType>> component_type_map = {
        {FormatType::float_,
         {
             {16, Bitmap::ComponentType::float16},
             {32, Bitmap::ComponentType::float32},
             {64, Bitmap::ComponentType::float64},
         }},
        {FormatType::unorm,
         {
             {8, Bitmap::ComponentType::uint8},
             {16, Bitmap::ComponentType::uint16},
             {32, Bitmap::ComponentType::uint32},
             {64, Bitmap::ComponentType::uint64},
         }},
        {FormatType::unorm_srgb,
         {
             {8, Bitmap::ComponentType::uint8},
             {16, Bitmap::ComponentType::uint16},
             {32, Bitmap::ComponentType::uint32},
             {64, Bitmap::ComponentType::uint64},
         }},
        {FormatType::snorm,
         {
             {8, Bitmap::ComponentType::int8},
             {16, Bitmap::ComponentType::int16},
             {32, Bitmap::ComponentType::int32},
             {64, Bitmap::ComponentType::int64},
         }},
        {FormatType::uint,
         {
             {8, Bitmap::ComponentType::uint8},
             {16, Bitmap::ComponentType::uint16},
             {32, Bitmap::ComponentType::uint32},
             {64, Bitmap::ComponentType::uint64},
         }},
        {FormatType::sint,
         {
             {8, Bitmap::ComponentType::int8},
             {16, Bitmap::ComponentType::int16},
             {32, Bitmap::ComponentType::int32},
             {64, Bitmap::ComponentType::int64},
         }},
    };

    auto it = pixel_format_map.find(info.channel_count);
    if (it == pixel_format_map.end())
        SGL_THROW("Unsupported channel count.");
    Bitmap::PixelFormat pixel_format = it->second;

    auto it1 = component_type_map.find(info.type);
    if (it1 == component_type_map.end())
        SGL_THROW("Unsupported format type.");
    auto it2 = it1->second.find(channel_bit_count);
    if (it2 == it1->second.end())
        SGL_THROW("Unsupported channel bits.");
    Bitmap::ComponentType component_type = it2->second;

    OwnedSubresourceData subresource_data = get_subresource_data(layer, mip_level);

    uint32_t width = get_mip_width(mip_level);
    uint32_t height = get_mip_height(mip_level);

    ref<Bitmap> bitmap = ref<Bitmap>(new Bitmap(pixel_format, component_type, width, height));
    bitmap->set_srgb_gamma(info.is_srgb_format());

    // TODO would be better to avoid this extra copy
    SGL_ASSERT(bitmap->buffer_size() == subresource_data.size);
    std::memcpy(bitmap->data(), subresource_data.data, subresource_data.size);

    return bitmap;
}

std::string Texture::to_string() const
{
    return fmt::format(
        "Texture(\n"
        "  device = {},\n"
        "  type = {},\n"
        "  width = {},\n"
        "  height = {},\n"
        "  depth = {},\n"
        "  mip_count = {},\n"
        "  array_length = {},\n"
        "  sample_count = {},\n"
        "  format = {},\n"
        "  usage = {},\n"
        "  memory_type = {},\n"
        "  size = {},\n"
        "  label = {}\n"
        ")",
        m_device,
        m_desc.type,
        m_desc.width,
        m_desc.height,
        m_desc.depth,
        m_desc.mip_count,
        m_desc.array_length,
        m_desc.sample_count,
        m_desc.format,
        m_desc.usage,
        m_desc.memory_type,
        string::format_byte_size(memory_usage().device),
        m_desc.label
    );
}

// ----------------------------------------------------------------------------
// TextureView
// ----------------------------------------------------------------------------

TextureView::TextureView(ref<Device> device, ref<Texture> texture, TextureViewDesc desc)
    : DeviceResource(std::move(device))
    , m_texture(std::move(texture))
    , m_desc(std::move(desc))
{
    if (m_desc.format == Format::undefined)
        m_desc.format = m_texture->format();

    uint32_t mip_count = m_texture->mip_count();
    SGL_CHECK(m_desc.subresource_range.mip_level < mip_count, "'mip_level' out of range");
    SGL_CHECK(
        m_desc.subresource_range.mip_count == SubresourceRange::ALL
            || m_desc.subresource_range.mip_level + m_desc.subresource_range.mip_count <= mip_count,
        "'mip_count' out of range"
    );
    if (m_desc.subresource_range.mip_count == SubresourceRange::ALL) {
        m_desc.subresource_range.mip_count = mip_count - m_desc.subresource_range.mip_level;
    }
    uint32_t layer_count = m_texture->layer_count();
    SGL_CHECK(m_desc.subresource_range.base_array_layer < layer_count, "'base_array_layer' out of range");
    SGL_CHECK(
        (m_desc.subresource_range.layer_count == SubresourceRange::ALL)
            || (m_desc.subresource_range.base_array_layer + m_desc.subresource_range.layer_count <= layer_count),
        "'layer_count' out of range"
    );
    if (m_desc.subresource_range.layer_count == SubresourceRange::ALL) {
        m_desc.subresource_range.layer_count = layer_count - m_desc.subresource_range.base_array_layer;
    }

    rhi::TextureViewDesc rhi_desc{
        .format = static_cast<rhi::Format>(m_desc.format),
        .aspect = static_cast<rhi::TextureAspect>(m_desc.aspect),
        .subresourceRange{
            .layer = m_desc.subresource_range.base_array_layer,
            .layerCount = m_desc.subresource_range.layer_count,
            .mipLevel = m_desc.subresource_range.mip_level,
            .mipLevelCount = m_desc.subresource_range.mip_count,
        },
        .label = m_desc.label.empty() ? nullptr : m_desc.label.c_str(),
    };
    SLANG_CALL(
        m_device->rhi_device()->createTextureView(m_texture->rhi_texture(), rhi_desc, m_rhi_texture_view.writeRef())
    );
}

NativeHandle TextureView::get_native_handle() const
{
    rhi::NativeHandle rhi_handle = {};
    SLANG_CALL(m_rhi_texture_view->getNativeHandle(&rhi_handle));
    return NativeHandle(rhi_handle);
}

std::string TextureView::to_string() const
{
    return fmt::format(
        "TextureView(\n"
        "  device = {},\n"
        "  texture = {},\n"
        "  format = {},\n"
        "  aspect = {},\n"
        "  subresource_range = {},\n"
        "  label = {}\n"
        ")",
        m_device,
        string::indent(m_texture->to_string()),
        m_desc.format,
        m_desc.aspect,
        m_desc.subresource_range.to_string(),
        m_desc.label
    );
}


#if 0
ResourceView::ResourceView(const ResourceViewDesc& desc, Buffer* buffer)
    : m_desc(desc)
    , m_resource(buffer)
{
    SGL_ASSERT(buffer);

    SGL_ASSERT(m_desc.type == ResourceViewType::shader_resource || m_desc.type == ResourceViewType::unordered_access);

    rhi::IResourceView::Desc rhi_desc{
        .type = static_cast<rhi::IResourceView::Type>(m_desc.type),
        .format = static_cast<rhi::Format>(m_desc.format),
        .bufferRange{
            .offset = m_desc.buffer_range.offset,
            .size = m_desc.buffer_range.size,
        },
    };
    // TODO handle uav counter
    SLANG_CALL(buffer->m_device->rhi_device()
                   ->createBufferView(buffer->rhi_buffer_resource(), nullptr, rhi_desc, m_rhi_resource_view.writeRef())
    );
}

ResourceView::ResourceView(const ResourceViewDesc& desc, Texture* texture)
    : m_desc(desc)
    , m_resource(texture)
{
    SGL_ASSERT(texture);

    SGL_ASSERT(
        m_desc.type == ResourceViewType::shader_resource || m_desc.type == ResourceViewType::unordered_access
        || m_desc.type == ResourceViewType::render_target || m_desc.type == ResourceViewType::depth_stencil
    );

    // Check if view covers all subresources.
    m_all_subresources = m_desc.subresource_range.mip_level == 0
        && m_desc.subresource_range.mip_count == texture->mip_count() && m_desc.subresource_range.base_array_layer == 0
        && m_desc.subresource_range.layer_count == texture->array_size();

    rhi::IResourceView::Desc rhi_desc{
        .type = static_cast<rhi::IResourceView::Type>(m_desc.type),
        .format = static_cast<rhi::Format>(m_desc.format),
        .renderTarget{
            .shape = (m_desc.type == ResourceViewType::render_target || m_desc.type == ResourceViewType::depth_stencil)
                ? static_cast<rhi::IResource::Type>(texture->type())
                : rhi::IResource::Type::Unknown,
        },
        .subresourceRange{
            .aspectMask = static_cast<rhi::TextureAspect>(m_desc.subresource_range.texture_aspect),
            .mipLevel = static_cast<rhi::GfxIndex>(m_desc.subresource_range.mip_level),
            .mipLevelCount = static_cast<rhi::GfxIndex>(m_desc.subresource_range.mip_count),
            .baseArrayLayer = static_cast<rhi::GfxIndex>(m_desc.subresource_range.base_array_layer),
            .layerCount = static_cast<rhi::GfxIndex>(m_desc.subresource_range.layer_count),
        },
    };
    SLANG_CALL(texture->m_device->rhi_device()
                   ->createTextureView(texture->rhi_texture_resource(), rhi_desc, m_rhi_resource_view.writeRef()));
}

ResourceView::~ResourceView()
{
    if (m_resource)
        m_resource->device()->deferred_release(m_rhi_resource_view);
}

void ResourceView::invalidate(bool deferred_release)
{
    if (m_resource) {
        if (deferred_release)
            m_resource->device()->deferred_release(m_rhi_resource_view);
        m_resource = nullptr;
    }
}

NativeHandle ResourceView::get_native_handle() const
{
    if (!m_resource)
        return {};
    rhi::InteropHandle handle = {};
    SLANG_CALL(m_rhi_resource_view->getNativeHandle(&handle));
#if SGL_HAS_D3D12
    if (m_resource->m_device->type() == DeviceType::d3d12)
        return NativeHandle(D3D12_CPU_DESCRIPTOR_HANDLE{handle.handleValue});
#endif
#if SGL_HAS_VULKAN
    if (m_resource->m_device->type() == DeviceType::vulkan) {
        if (m_resource) {
            if (m_resource->type() == ResourceType::buffer) {
                if (m_desc.format == Format::undefined)
                    return NativeHandle(reinterpret_cast<VkBuffer>(handle.handleValue));
                else
                    return NativeHandle(reinterpret_cast<VkBufferView>(handle.handleValue));
            } else {
                return NativeHandle(reinterpret_cast<VkImageView>(handle.handleValue));
            }
        }
    }
#endif
    return {};
}

std::string ResourceView::to_string() const
{
    return fmt::format(
        "ResourceView(\n"
        "  type = {},\n"
        "  format = {},\n"
        "  buffer_range = {},\n"
        "  subresource_range = {},\n"
        "  resource = {}\n"
        ")",
        m_desc.type,
        m_desc.format,
        m_desc.buffer_range.to_string(),
        m_desc.subresource_range.to_string(),
        string::indent(m_resource ? m_resource->to_string() : "null")
    );
}
#endif

} // namespace sgl
