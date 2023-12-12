#include "resource.h"

#include "kali/device/device.h"
#include "kali/device/helpers.h"
#include "kali/device/native_handle_traits.h"

#include "kali/core/config.h"
#include "kali/core/error.h"
#include "kali/core/string.h"
#include "kali/core/maths.h"
#include "kali/core/bitmap.h"

#include <bit>

namespace kali {

inline gfx::ResourceStateSet gfx_allowed_states(ResourceUsage usage)
{
    gfx::ResourceStateSet states(gfx::ResourceState::CopyDestination, gfx::ResourceState::CopySource);

    if (is_set(usage, ResourceUsage::vertex)) {
        states.add(gfx::ResourceState::VertexBuffer);
        states.add(gfx::ResourceState::AccelerationStructureBuildInput);
    }
    if (is_set(usage, ResourceUsage::index)) {
        states.add(gfx::ResourceState::IndexBuffer);
        states.add(gfx::ResourceState::AccelerationStructureBuildInput);
    }
    if (is_set(usage, ResourceUsage::constant)) {
        states.add(gfx::ResourceState::ConstantBuffer);
    }
    if (is_set(usage, ResourceUsage::stream_output)) {
        states.add(gfx::ResourceState::StreamOutput);
    }
    if (is_set(usage, ResourceUsage::shader_resource)) {
        states.add(gfx::ResourceState::ShaderResource);
    }
    if (is_set(usage, ResourceUsage::unordered_access)) {
        states.add(gfx::ResourceState::UnorderedAccess);
    }
    if (is_set(usage, ResourceUsage::render_target)) {
        states.add(gfx::ResourceState::RenderTarget);
    }
    if (is_set(usage, ResourceUsage::depth_stencil)) {
        states.add(gfx::ResourceState::DepthWrite);
    }
    if (is_set(usage, ResourceUsage::indirect_arg)) {
        states.add(gfx::ResourceState::IndirectArgument);
    }
    if (is_set(usage, ResourceUsage::acceleration_structure)) {
        states.add(gfx::ResourceState::AccelerationStructure);
        states.add(gfx::ResourceState::ShaderResource);
        states.add(gfx::ResourceState::UnorderedAccess);
    }

    return states;
}

inline gfx::ResourceState gfx_initial_state(ResourceUsage usage)
{
    if (is_set(usage, ResourceUsage::acceleration_structure)) {
        return gfx::ResourceState::AccelerationStructure;
    }
    return gfx::ResourceState::General;
}

// ----------------------------------------------------------------------------
// Resource
// ----------------------------------------------------------------------------

Resource::Resource(ref<Device> device, ResourceType type)
    : DeviceResource(std::move(device))
    , m_type(type)
{
}

Resource::~Resource() { }

const char* Resource::debug_name() const
{
    return gfx_resource()->getDebugName();
}

void Resource::set_debug_name(const char* name)
{
    gfx_resource()->setDebugName(name);
}

NativeHandle Resource::get_native_handle() const
{
    gfx::InteropHandle handle = {};
    SLANG_CALL(gfx_resource()->getNativeResourceHandle(&handle));
#if KALI_HAS_D3D12
    if (m_device->type() == DeviceType::d3d12)
        return NativeHandle(reinterpret_cast<ID3D12Resource*>(handle.handleValue));
#endif
#if KALI_HAS_VULKAN
    if (m_device->type() == DeviceType::vulkan) {
        if (m_type == ResourceType::buffer)
            return NativeHandle(reinterpret_cast<VkBuffer>(handle.handleValue));
        else
            return NativeHandle(reinterpret_cast<VkImage>(handle.handleValue));
    }
#endif
    return {};
}

// ----------------------------------------------------------------------------
// ResourceView
// ----------------------------------------------------------------------------

ResourceView::ResourceView(const ResourceViewDesc& desc, const Buffer* buffer)
    : m_desc(desc)
    , m_resource(buffer)
{
    KALI_ASSERT(buffer);

    KALI_ASSERT(m_desc.type == ResourceViewType::shader_resource || m_desc.type == ResourceViewType::unordered_access);

    gfx::IResourceView::Desc gfx_desc{
        .type = static_cast<gfx::IResourceView::Type>(m_desc.type),
        .format = static_cast<gfx::Format>(m_desc.format),
        .bufferRange{
            .firstElement = m_desc.buffer_range.first_element,
            .elementCount = m_desc.buffer_range.element_count,
        },
        .bufferElementSize = m_desc.buffer_element_size,
    };
    // TODO handle uav counter
    SLANG_CALL(buffer->m_device->gfx_device()
                   ->createBufferView(buffer->gfx_buffer_resource(), nullptr, gfx_desc, m_gfx_resource_view.writeRef())
    );
}

ResourceView::ResourceView(const ResourceViewDesc& desc, const Texture* texture)
    : m_desc(desc)
    , m_resource(texture)
{
    KALI_ASSERT(texture);

    KALI_ASSERT(
        m_desc.type == ResourceViewType::shader_resource || m_desc.type == ResourceViewType::unordered_access
        || m_desc.type == ResourceViewType::render_target || m_desc.type == ResourceViewType::depth_stencil
    );

    gfx::IResourceView::Desc gfx_desc{
        .type = static_cast<gfx::IResourceView::Type>(m_desc.type),
        .format = static_cast<gfx::Format>(m_desc.format),
        .subresourceRange{
            .aspectMask = static_cast<gfx::TextureAspect>(m_desc.subresource_range.texture_aspect),
            .mipLevel = static_cast<gfx::GfxIndex>(m_desc.subresource_range.mip_level),
            .mipLevelCount = static_cast<gfx::GfxIndex>(m_desc.subresource_range.mip_count),
            .baseArrayLayer = static_cast<gfx::GfxIndex>(m_desc.subresource_range.base_array_layer),
            .layerCount = static_cast<gfx::GfxIndex>(m_desc.subresource_range.layer_count),
        },
    };
    SLANG_CALL(texture->m_device->gfx_device()
                   ->createTextureView(texture->gfx_texture_resource(), gfx_desc, m_gfx_resource_view.writeRef()));
}

NativeHandle ResourceView::get_native_handle() const
{
    if (!m_resource)
        return {};
    gfx::InteropHandle handle = {};
    SLANG_CALL(m_gfx_resource_view->getNativeHandle(&handle));
#if KALI_HAS_D3D12
    if (m_resource->m_device->type() == DeviceType::d3d12)
        return NativeHandle(D3D12_CPU_DESCRIPTOR_HANDLE{handle.handleValue});
#endif
#if KALI_HAS_VULKAN
    if (m_resource->m_device->type() == DeviceType::vulkan) {
        if (m_resource) {
            if (m_resource->type() == ResourceType::buffer) {
                if (m_desc.format == Format::unknown)
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

// ----------------------------------------------------------------------------
// Buffer
// ----------------------------------------------------------------------------

Buffer::Buffer(ref<Device> device, BufferDesc desc, const void* init_data, size_t init_data_size)
    : Resource(std::move(device), ResourceType::buffer)
    , m_desc(std::move(desc))
{
    // Derive buffer size from init data.
    if (m_desc.size == 0 && init_data && init_data_size > 0)
        m_desc.size = init_data_size;

    // TODO check init_data size
    KALI_UNUSED(init_data_size);
    KALI_ASSERT(m_desc.size > 0);
    KALI_ASSERT(m_desc.struct_size == 0 || m_desc.format == Format::unknown);
    KALI_ASSERT(m_desc.struct_size == 0 || m_desc.size % m_desc.struct_size == 0);

    KALI_CHECK(
        (init_data == nullptr && init_data_size == 0) || init_data_size == m_desc.size,
        "Invalid init data size (got {} bytes, expected {} bytes)",
        init_data_size,
        m_desc.size
    );

    gfx::IBufferResource::Desc gfx_desc{};
    gfx_desc.type = gfx::IResource::Type::Buffer;
    gfx_desc.defaultState = static_cast<gfx::ResourceState>(m_desc.initial_state);
    gfx_desc.allowedStates = gfx_allowed_states(m_desc.usage);
    gfx_desc.memoryType = static_cast<gfx::MemoryType>(m_desc.memory_type);
    // TODO(@skallweit): add support for existing handles
    // gfx_desc.existingHandle =
    gfx_desc.isShared = is_set(m_desc.usage, ResourceUsage::shared);
    gfx_desc.sizeInBytes = static_cast<gfx::Size>(m_desc.size);
    gfx_desc.elementSize = static_cast<gfx::Size>(m_desc.struct_size);
    gfx_desc.format = static_cast<gfx::Format>(m_desc.format);

    SLANG_CALL(m_device->gfx_device()->createBufferResource(gfx_desc, init_data, m_gfx_buffer.writeRef()));
}

inline BufferDesc to_buffer_desc(StructuredBufferDesc desc)
{
    KALI_CHECK(desc.struct_size > 0 || desc.struct_type, "Either 'struct_size' or 'struct_type' must be set.");

    size_t struct_size = desc.struct_size;
    if (desc.struct_type) {
        const TypeLayoutReflection* type = desc.struct_type->unwrap_array()->element_type_layout();
        if (type)
            struct_size = type->size();
    }

    return {
        .size = desc.element_count * struct_size,
        .struct_size = struct_size,
        .initial_state = desc.initial_state,
        .usage = desc.usage,
        .memory_type = desc.memory_type,
        .debug_name = desc.debug_name,
    };
}

Buffer::Buffer(ref<Device> device, StructuredBufferDesc desc, const void* init_data, size_t init_data_size)
    : Buffer(std::move(device), to_buffer_desc(std::move(desc)), init_data, init_data_size)
{
}

inline BufferDesc to_buffer_desc(TypedBufferDesc desc)
{
    KALI_CHECK(desc.format != Format::unknown, "Invalid format.");

    return {
        .size = desc.element_count * get_format_info(desc.format).bytes_per_block,
        .format = desc.format,
        .initial_state = desc.initial_state,
        .usage = desc.usage,
        .memory_type = desc.memory_type,
        .debug_name = desc.debug_name,
    };
}

Buffer::Buffer(ref<Device> device, TypedBufferDesc desc, const void* init_data, size_t init_data_size)
    : Buffer(std::move(device), to_buffer_desc(std::move(desc)), init_data, init_data_size)
{
}

size_t Buffer::element_size() const
{
    if (is_structured())
        return m_desc.struct_size;
    else if (is_typed())
        return get_format_info(m_desc.format).bytes_per_block;
    else
        return 4;
}

size_t Buffer::element_count() const
{
    return size() / element_size();
}

void* Buffer::map() const
{
    KALI_ASSERT(m_desc.memory_type != MemoryType::device_local);
    KALI_ASSERT(m_mapped_ptr == nullptr);
    SLANG_CALL(m_gfx_buffer->map(nullptr, &m_mapped_ptr));
    return m_mapped_ptr;
}

void* Buffer::map(DeviceAddress offset, DeviceSize size_in_bytes) const
{
    KALI_ASSERT(m_desc.memory_type != MemoryType::device_local);
    KALI_ASSERT(m_mapped_ptr == nullptr);
    gfx::MemoryRange gfx_read_range{
        .offset = offset,
        .size = size_in_bytes,
    };
    SLANG_CALL(m_gfx_buffer->map(&gfx_read_range, &m_mapped_ptr));
    return m_mapped_ptr;
}

void Buffer::unmap() const
{
    KALI_ASSERT(m_desc.memory_type != MemoryType::device_local);
    KALI_ASSERT(m_mapped_ptr != nullptr);
    SLANG_CALL(m_gfx_buffer->unmap(nullptr));
    m_mapped_ptr = nullptr;
}

ref<ResourceView> Buffer::get_view(ResourceViewDesc desc) const
{
    size_t element_count = this->element_count();
    KALI_CHECK(desc.buffer_range.first_element < element_count, "'first_element' out of range");
    KALI_CHECK(
        (desc.buffer_range.element_count == BufferRange::ALL)
            || (desc.buffer_range.first_element + desc.buffer_range.element_count <= element_count),
        "'element_count' out of range"
    );

    if (desc.buffer_range.element_count == BufferRange::ALL) {
        desc.buffer_range.element_count = element_count - desc.buffer_range.first_element;
    }

    auto it = m_views.find(desc);
    if (it != m_views.end())
        return it->second;
    auto view = make_ref<ResourceView>(desc, this);
    m_views.emplace(desc, view);
    return view;
}

ref<ResourceView> Buffer::get_srv(uint64_t first_element, uint64_t element_count) const
{
    return get_view({
        .type = ResourceViewType::shader_resource,
        .format = m_desc.format,
        .buffer_range{.first_element = first_element, .element_count = element_count},
        .buffer_element_size = m_desc.struct_size,
    });
}

ref<ResourceView> Buffer::get_uav(uint64_t first_element, uint64_t element_count) const
{
    return get_view({
        .type = ResourceViewType::unordered_access,
        .format = m_desc.format,
        .buffer_range{.first_element = first_element, .element_count = element_count},
        .buffer_element_size = m_desc.struct_size,
    });
}

ref<ResourceView> Buffer::get_srv() const
{
    return get_srv(0);
}

ref<ResourceView> Buffer::get_uav() const
{
    return get_uav(0);
}

DeviceResource::MemoryUsage Buffer::memory_usage() const
{
    return {.device = m_desc.size};
}

std::string Buffer::to_string() const
{
    return fmt::format(
        "Buffer(\n"
        "  device={},\n"
        "  size={},\n"
        "  struct_size={},\n"
        "  format={},\n"
        "  usage={},\n"
        "  memory_type={},\n"
        "  memory_usage={}\n"
        ")",
        m_device,
        m_desc.size,
        m_desc.struct_size,
        m_desc.format,
        m_desc.usage,
        m_desc.memory_type,
        string::format_byte_size(memory_usage().device)
    );
}

// ----------------------------------------------------------------------------
// Texture
// ----------------------------------------------------------------------------

inline void process_texture_desc(TextureDesc& desc)
{
    KALI_CHECK(desc.format != Format::unknown, "Invalid texture format.");

    // Try to infer texture type from dimensions.
    if (desc.type == TextureType::unknown) {
        if (desc.width > 0 && desc.height == 0 && desc.depth == 0) {
            desc.type = TextureType::texture_1d;
        } else if (desc.width > 0 && desc.height > 0 && desc.depth == 0) {
            desc.type = TextureType::texture_2d;
        } else if (desc.width > 0 && desc.height > 0 && desc.depth > 0) {
            desc.type = TextureType::texture_3d;
        } else {
            KALI_THROW("Failed to infer texture type from dimensions.");
        }
    } else {
        switch (desc.type) {
        case TextureType::texture_1d:
            KALI_CHECK(desc.width > 0 && desc.height == 0 && desc.depth == 0, "Invalid dimensions for 1D texture.");
            break;
        case TextureType::texture_2d:
            KALI_CHECK(desc.width > 0 && desc.height > 0 && desc.depth == 0, "Invalid dimensions for 2D texture.");
            break;
        case TextureType::texture_3d:
            KALI_CHECK(desc.width > 0 && desc.height > 0 && desc.depth > 0, "Invalid dimensions for 3D texture.");
            break;
        case TextureType::texture_cube:
            KALI_CHECK(desc.width > 0 && desc.height > 0 && desc.depth == 0, "Invalid dimensions for cube texture.");
            break;
        }
    }

    if (desc.mip_count == 0) {
        desc.mip_count = std::bit_width(std::max({desc.width, desc.height, desc.depth}));
    }
    KALI_CHECK(desc.array_size >= 1, "Invalid array size.");
    KALI_CHECK(desc.sample_count >= 1, "Invalid sample count.");
}

Texture::Texture(ref<Device> device, TextureDesc desc, const void* init_data, size_t init_data_size)
    : Resource(std::move(device), ResourceType(desc.type))
    , m_desc(std::move(desc))
{
    process_texture_desc(m_desc);

    gfx::ITextureResource::Desc gfx_desc{};
    gfx_desc.type = static_cast<gfx::IResource::Type>(m_desc.type);
    gfx_desc.defaultState = static_cast<gfx::ResourceState>(m_desc.initial_state);
    gfx_desc.allowedStates = gfx_allowed_states(m_desc.usage);
    gfx_desc.memoryType = static_cast<gfx::MemoryType>(m_desc.memory_type);
    // TODO(@skallweit): add support for existing handles
    // gfx_desc.existingHandle =
    gfx_desc.isShared = is_set(m_desc.usage, ResourceUsage::shared);
    gfx_desc.size.width = static_cast<gfx::Size>(m_desc.width);
    gfx_desc.size.height = static_cast<gfx::Size>(m_desc.height);
    gfx_desc.size.depth = static_cast<gfx::Size>(m_desc.depth);
    gfx_desc.numMipLevels = static_cast<gfx::Size>(m_desc.mip_count);
    gfx_desc.arraySize = static_cast<gfx::Size>(m_desc.array_size);
    gfx_desc.format = static_cast<gfx::Format>(m_desc.format);
    gfx_desc.sampleDesc.numSamples = static_cast<gfx::GfxCount>(m_desc.sample_count);
    gfx_desc.sampleDesc.quality = m_desc.quality;

    // TODO(@skallweit): add support for init data
    KALI_UNUSED(init_data, init_data_size);
    gfx::ITextureResource::SubresourceData* gfx_init_data{nullptr};

    SLANG_CALL(m_device->gfx_device()->createTextureResource(gfx_desc, gfx_init_data, m_gfx_texture.writeRef()));
}

Texture::Texture(ref<Device> device, TextureDesc desc, gfx::ITextureResource* resource)
    : Resource(std::move(device), ResourceType(desc.type))
    , m_desc(std::move(desc))
{
    process_texture_desc(m_desc);

    m_gfx_texture = resource;
}

SubresourceLayout Texture::get_subresource_layout(uint32_t subresource) const
{
    KALI_CHECK_LT(subresource, subresource_count());

    gfx::FormatInfo gfx_format_info;
    SLANG_CALL(gfx::gfxGetFormatInfo(gfx_texture_resource()->getDesc()->format, &gfx_format_info));
    size_t alignment;
    SLANG_CALL(m_device->gfx_device()->getTextureRowAlignment(&alignment));

    uint32_t mip_level = get_subresource_mip_level(subresource);
    uint3 mip_dimensions = get_mip_dimensions(mip_level);

    SubresourceLayout layout;
    layout.row_size
        = div_round_up(mip_dimensions.x, uint32_t(gfx_format_info.blockWidth)) * gfx_format_info.blockSizeInBytes;
    layout.row_size_aligned = align_to(alignment, layout.row_size);
    layout.row_count = div_round_up(mip_dimensions.y, uint32_t(gfx_format_info.blockHeight));
    layout.depth = mip_dimensions.z;
    return layout;
}

ref<ResourceView> Texture::get_view(ResourceViewDesc desc) const
{
    uint32_t mip_count = this->mip_count();
    KALI_CHECK(desc.subresource_range.mip_level < mip_count, "'mip_level' out of range");
    KALI_CHECK(
        desc.subresource_range.mip_count == SubresourceRange::ALL
            || desc.subresource_range.mip_level + desc.subresource_range.mip_count <= mip_count,
        "'mip_count' out of range"
    );
    if (desc.subresource_range.mip_count == SubresourceRange::ALL) {
        desc.subresource_range.mip_count = mip_count - desc.subresource_range.mip_level;
    }
    uint32_t array_size = this->array_size();
    KALI_CHECK(desc.subresource_range.base_array_layer < array_size, "'base_array_layer' out of range");
    KALI_CHECK(
        (desc.subresource_range.layer_count == SubresourceRange::ALL)
            || (desc.subresource_range.base_array_layer + desc.subresource_range.layer_count <= array_size),
        "'layer_count' out of range"
    );
    if (desc.subresource_range.layer_count == SubresourceRange::ALL) {
        desc.subresource_range.layer_count = array_size - desc.subresource_range.base_array_layer;
    }

    auto it = m_views.find(desc);
    if (it != m_views.end())
        return it->second;
    auto view = make_ref<ResourceView>(desc, this);
    m_views.emplace(desc, view);
    return view;
}

ref<ResourceView>
Texture::get_srv(uint32_t mip_level, uint32_t mip_count, uint32_t base_array_layer, uint32_t layer_count) const
{
    return get_view({
        .type = ResourceViewType::shader_resource,
        .format = m_desc.format,
        .subresource_range = {
            .texture_aspect = TextureAspect::color,
            .mip_level = mip_level,
            .mip_count = mip_count,
            .base_array_layer = base_array_layer,
            .layer_count = layer_count,
        },
    });
}

ref<ResourceView> Texture::get_uav(uint32_t mip_level, uint32_t base_array_layer, uint32_t layer_count) const
{
    return get_view({
        .type = ResourceViewType::unordered_access,
        .format = m_desc.format,
        .subresource_range = {
            .texture_aspect = TextureAspect::color,
            .mip_level = mip_level,
            .mip_count = 1,
            .base_array_layer = base_array_layer,
            .layer_count = layer_count,
        },
    });
}

ref<ResourceView> Texture::get_dsv(uint32_t mip_level, uint32_t base_array_layer, uint32_t layer_count) const
{
    return get_view({
        .type = ResourceViewType::depth_stencil,
        .format = m_desc.format,
        .subresource_range = {
            .texture_aspect = TextureAspect::depth_stencil,
            .mip_level = mip_level,
            .mip_count = 1,
            .base_array_layer = base_array_layer,
            .layer_count = layer_count,
        },
    });
}

ref<ResourceView> Texture::get_rtv(uint32_t mip_level, uint32_t base_array_layer, uint32_t layer_count) const
{
    return get_view({
        .type = ResourceViewType::render_target,
        .format = m_desc.format,
        .subresource_range = {
            .texture_aspect = TextureAspect::color,
            .mip_level = mip_level,
            .mip_count = 1,
            .base_array_layer = base_array_layer,
            .layer_count = layer_count,
        },
    });
}

ref<ResourceView> Texture::get_srv() const
{
    return get_srv(0);
}

ref<ResourceView> Texture::get_uav() const
{
    return get_uav(0);
}

DeviceResource::MemoryUsage Texture::memory_usage() const
{
    gfx::Size size = 0, alignment = 0;
    SLANG_CALL(m_device->gfx_device()->getTextureAllocationInfo(*m_gfx_texture->getDesc(), &size, &alignment));
    return {.device = size};
}

ref<Bitmap> Texture::to_bitmap(uint32_t mip_level, uint32_t array_slice) const
{
    KALI_CHECK_LT(mip_level, mip_count());
    KALI_CHECK_LT(array_slice, array_size());

    const FormatInfo& info = get_format_info(m_desc.format);
    if (info.is_compressed)
        KALI_THROW("Cannot convert compressed texture to bitmap.");
    if (info.is_depth_stencil())
        KALI_THROW("Cannot convert depth/stencil texture to bitmap.");
    if (info.is_typeless_format())
        KALI_THROW("Cannot convert typeless texture to bitmap.");
    if (!info.has_equal_channel_bits())
        KALI_THROW("Cannot convert texture with unequal channel bits to bitmap.");
    uint32_t channel_bit_count = info.channel_bit_count[0];
    if (channel_bit_count != 8 && channel_bit_count != 16 && channel_bit_count != 32 && channel_bit_count != 64)
        KALI_THROW("Cannot convert texture with non-8/16/32/64 bit channels to bitmap.");

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
        KALI_THROW("Unsupported channel count.");
    Bitmap::PixelFormat pixel_format = it->second;

    auto it1 = component_type_map.find(info.type);
    if (it1 == component_type_map.end())
        KALI_THROW("Unsupported format type.");
    auto it2 = it1->second.find(channel_bit_count);
    if (it2 == it1->second.end())
        KALI_THROW("Unsupported channel bits.");
    Bitmap::ComponentType component_type = it2->second;

    uint32_t subresource = get_subresource_index(mip_level, array_slice);
    SubresourceLayout layout = get_subresource_layout(subresource);

    uint32_t width = get_mip_width(mip_level);
    uint32_t height = get_mip_height(mip_level);

    ref<Bitmap> bitmap = ref<Bitmap>(new Bitmap(pixel_format, component_type, width, height));

    size_t size = layout.total_size_aligned();
    KALI_ASSERT(size == bitmap->buffer_size());
    size_t row_pitch, pixel_size;
    m_device->read_texture(this, bitmap->buffer_size(), bitmap->data(), &row_pitch, &pixel_size);
    KALI_ASSERT(pixel_size == bitmap->bytes_per_pixel());
    KALI_ASSERT(row_pitch == bitmap->width() * bitmap->bytes_per_pixel());

    return bitmap;
}

std::string Texture::to_string() const
{
    return fmt::format(
        "Texture(\n"
        "  device={},\n"
        "  type={},\n"
        "  width={},\n"
        "  height={},\n"
        "  depth={},\n"
        "  mip_count={},\n"
        "  array_size={},\n"
        "  sample_count={},\n"
        "  format={},\n"
        "  usage={},\n"
        "  memory_type={},\n"
        "  size={}\n"
        ")",
        m_device,
        m_desc.type,
        m_desc.width,
        m_desc.height,
        m_desc.depth,
        m_desc.mip_count,
        m_desc.array_size,
        m_desc.sample_count,
        m_desc.format,
        m_desc.usage,
        m_desc.memory_type,
        string::format_byte_size(memory_usage().device)
    );
}

} // namespace kali
