// SPDX-License-Identifier: Apache-2.0

#include "resource.h"

#include "sgl/device/device.h"
#include "sgl/device/helpers.h"
#include "sgl/device/native_handle_traits.h"

#include "sgl/core/config.h"
#include "sgl/core/error.h"
#include "sgl/core/string.h"
#include "sgl/core/maths.h"
#include "sgl/core/bitmap.h"

#include "sgl/stl/bit.h" // Replace with <bit> when available on all platforms.

namespace sgl {

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

Resource::~Resource()
{
    for (auto& [desc, view] : m_views)
        view->invalidate(m_deferred_release);
}

SharedResourceHandle Resource::get_shared_handle() const
{
    gfx::InteropHandle handle = {};
    SLANG_CALL(gfx_resource()->getSharedHandle(&handle));
    return static_cast<SharedResourceHandle>(handle.handleValue);
}

NativeHandle Resource::get_native_handle() const
{
    gfx::InteropHandle handle = {};
    SLANG_CALL(gfx_resource()->getNativeResourceHandle(&handle));
#if SGL_HAS_D3D12
    if (m_device->type() == DeviceType::d3d12)
        return NativeHandle(reinterpret_cast<ID3D12Resource*>(handle.handleValue));
#endif
#if SGL_HAS_VULKAN
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
    SGL_ASSERT(buffer);

    SGL_ASSERT(m_desc.type == ResourceViewType::shader_resource || m_desc.type == ResourceViewType::unordered_access);

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
    SGL_ASSERT(texture);

    SGL_ASSERT(
        m_desc.type == ResourceViewType::shader_resource || m_desc.type == ResourceViewType::unordered_access
        || m_desc.type == ResourceViewType::render_target || m_desc.type == ResourceViewType::depth_stencil
    );

    gfx::IResourceView::Desc gfx_desc{
        .type = static_cast<gfx::IResourceView::Type>(m_desc.type),
        .format = static_cast<gfx::Format>(m_desc.format),
        .renderTarget{
            .shape = m_desc.type == ResourceViewType::render_target ? static_cast<gfx::IResource::Type>(texture->type())
                                                                    : gfx::IResource::Type::Unknown,
        },
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

ResourceView::~ResourceView()
{
    if (m_resource)
        m_resource->device()->deferred_release(m_gfx_resource_view);
}

void ResourceView::invalidate(bool deferred_release)
{
    if (m_resource) {
        if (deferred_release)
            m_resource->device()->deferred_release(m_gfx_resource_view);
        m_resource = nullptr;
    }
}

NativeHandle ResourceView::get_native_handle() const
{
    if (!m_resource)
        return {};
    gfx::InteropHandle handle = {};
    SLANG_CALL(m_gfx_resource_view->getNativeHandle(&handle));
#if SGL_HAS_D3D12
    if (m_resource->m_device->type() == DeviceType::d3d12)
        return NativeHandle(D3D12_CPU_DESCRIPTOR_HANDLE{handle.handleValue});
#endif
#if SGL_HAS_VULKAN
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

std::string ResourceView::to_string() const
{
    return fmt::format(
        "ResourceView(\n"
        "  type = {},\n"
        "  format = {},\n"
        "  buffer_range = (first_element={}, element_count={}),\n"
        "  buffer_element_size = {},\n"
        // "  subresource_range = {},\n"
        "  resource = {}\n"
        ")",
        m_desc.type,
        m_desc.format,
        m_desc.buffer_range.first_element,
        m_desc.buffer_range.element_count,
        m_desc.buffer_element_size,
        // m_desc.subresource_range,
        string::indent(m_resource ? m_resource->to_string() : "null")
    );
}

// ----------------------------------------------------------------------------
// Buffer
// ----------------------------------------------------------------------------

Buffer::Buffer(ref<Device> device, BufferDesc desc)
    : Resource(std::move(device), ResourceType::buffer)
    , m_desc(std::move(desc))
{
    // Derive buffer size from initial data.
    if (m_desc.size == 0 && m_desc.data && m_desc.data_size > 0)
        m_desc.size = m_desc.data_size;

    // TODO check init_data size
    SGL_ASSERT(m_desc.size > 0);
    SGL_ASSERT(m_desc.struct_size == 0 || m_desc.format == Format::unknown);
    SGL_ASSERT(m_desc.struct_size == 0 || m_desc.size % m_desc.struct_size == 0);

    SGL_CHECK(
        (m_desc.data == nullptr && m_desc.data_size == 0) || m_desc.data_size == m_desc.size,
        "Invalid data size (got {} bytes, expected {} bytes)",
        m_desc.data_size,
        m_desc.size
    );

    // Override initial state if not specified.
    if (m_desc.initial_state == ResourceState::undefined) {
        if (is_set(m_desc.usage, ResourceUsage::acceleration_structure)) {
            m_desc.initial_state = ResourceState::acceleration_structure;
            m_desc.usage |= ResourceUsage::unordered_access | ResourceUsage::shader_resource;
        }
    }

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

    SLANG_CALL(m_device->gfx_device()->createBufferResource(gfx_desc, m_desc.data, m_gfx_buffer.writeRef()));

    if (!m_desc.debug_name.empty())
        m_gfx_buffer->setDebugName(m_desc.debug_name.c_str());

    // Clear initial data fields in desc.
    m_desc.data = nullptr;
    m_desc.data_size = 0;
}

inline BufferDesc to_buffer_desc(RawBufferDesc desc)
{
    return {
        .size = desc.size,
        .initial_state = desc.initial_state,
        .usage = desc.usage,
        .memory_type = desc.memory_type,
        .debug_name = desc.debug_name,
        .data = desc.data,
        .data_size = desc.data_size,
    };
}

Buffer::Buffer(ref<Device> device, RawBufferDesc desc)
    : Buffer(std::move(device), to_buffer_desc(std::move(desc)))
{
}

inline BufferDesc to_buffer_desc(StructuredBufferDesc desc)
{
    SGL_CHECK(desc.struct_size > 0 || desc.struct_type, "Either 'struct_size' or 'struct_type' must be set.");

    size_t struct_size = desc.struct_size;
    if (desc.struct_type) {
        const TypeLayoutReflection* type = desc.struct_type->unwrap_array()->element_type_layout();
        if (type)
            struct_size = type->stride();
    }

    return {
        .size = desc.element_count * struct_size,
        .struct_size = struct_size,
        .initial_state = desc.initial_state,
        .usage = desc.usage,
        .memory_type = desc.memory_type,
        .debug_name = desc.debug_name,
        .data = desc.data,
        .data_size = desc.data_size,
    };
}

Buffer::Buffer(ref<Device> device, StructuredBufferDesc desc)
    : Buffer(std::move(device), to_buffer_desc(std::move(desc)))
{
}

inline BufferDesc to_buffer_desc(TypedBufferDesc desc)
{
    SGL_CHECK(desc.format != Format::unknown, "Invalid format.");

    return {
        .size = desc.element_count * get_format_info(desc.format).bytes_per_block,
        .format = desc.format,
        .initial_state = desc.initial_state,
        .usage = desc.usage,
        .memory_type = desc.memory_type,
        .debug_name = desc.debug_name,
        .data = desc.data,
        .data_size = desc.data_size,
    };
}

Buffer::Buffer(ref<Device> device, TypedBufferDesc desc)
    : Buffer(std::move(device), to_buffer_desc(std::move(desc)))
{
}

Buffer::~Buffer()
{
    m_device->deferred_release(m_gfx_buffer);
}

size_t Buffer::element_size() const
{
    if (is_structured())
        return m_desc.struct_size;
    else if (is_typed())
        return get_format_info(m_desc.format).bytes_per_block;
    else
        return 1;
}

size_t Buffer::element_count() const
{
    return size() / element_size();
}

void* Buffer::map() const
{
    SGL_ASSERT(m_desc.memory_type != MemoryType::device_local);
    SGL_ASSERT(m_mapped_ptr == nullptr);
    SLANG_CALL(m_gfx_buffer->map(nullptr, &m_mapped_ptr));
    return m_mapped_ptr;
}

void* Buffer::map(DeviceAddress offset, DeviceSize size_in_bytes) const
{
    SGL_ASSERT(m_desc.memory_type != MemoryType::device_local);
    SGL_ASSERT(m_mapped_ptr == nullptr);
    gfx::MemoryRange gfx_read_range{
        .offset = offset,
        .size = size_in_bytes,
    };
    SLANG_CALL(m_gfx_buffer->map(&gfx_read_range, &m_mapped_ptr));
    return m_mapped_ptr;
}

void Buffer::unmap() const
{
    SGL_ASSERT(m_desc.memory_type != MemoryType::device_local);
    SGL_ASSERT(m_mapped_ptr != nullptr);
    SLANG_CALL(m_gfx_buffer->unmap(nullptr));
    m_mapped_ptr = nullptr;
}

ref<ResourceView> Buffer::get_view(ResourceViewDesc desc) const
{
    size_t element_count = this->element_count();
    SGL_CHECK(desc.buffer_range.first_element < element_count, "'first_element' out of range");
    SGL_CHECK(
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
        "  device = {},\n"
        "  size = {},\n"
        "  struct_size = {},\n"
        "  format = {},\n"
        "  usage = {},\n"
        "  memory_type = {},\n"
        "  memory_usage = {},\n"
        "  debug_name = {}\n"
        ")",
        m_device,
        m_desc.size,
        m_desc.struct_size,
        m_desc.format,
        m_desc.usage,
        m_desc.memory_type,
        string::format_byte_size(memory_usage().device),
        m_desc.debug_name
    );
}

// ----------------------------------------------------------------------------
// Texture
// ----------------------------------------------------------------------------

inline void process_texture_desc(TextureDesc& desc)
{
    SGL_CHECK(desc.type != ResourceType::buffer, "Invalid resource type.");
    SGL_CHECK(desc.format != Format::unknown, "Invalid texture format.");

    // Try to infer texture type from dimensions.
    if (desc.type == ResourceType::unknown) {
        if (desc.width > 0 && desc.height == 0 && desc.depth == 0) {
            desc.type = ResourceType::texture_1d;
        } else if (desc.width > 0 && desc.height > 0 && desc.depth == 0) {
            desc.type = ResourceType::texture_2d;
        } else if (desc.width > 0 && desc.height > 0 && desc.depth > 0) {
            desc.type = ResourceType::texture_3d;
        } else {
            SGL_THROW("Failed to infer texture type from dimensions.");
        }
    } else {
        switch (desc.type) {
        case ResourceType::unknown:
            SGL_THROW("Invalid texture type.");
            break;
        case ResourceType::texture_1d:
            SGL_CHECK(desc.width > 0 && desc.height == 0 && desc.depth == 0, "Invalid dimensions for 1D texture.");
            break;
        case ResourceType::texture_2d:
            SGL_CHECK(desc.width > 0 && desc.height > 0 && desc.depth == 0, "Invalid dimensions for 2D texture.");
            break;
        case ResourceType::texture_3d:
            SGL_CHECK(desc.width > 0 && desc.height > 0 && desc.depth > 0, "Invalid dimensions for 3D texture.");
            break;
        case ResourceType::texture_cube:
            SGL_CHECK(desc.width > 0 && desc.height > 0 && desc.depth == 0, "Invalid dimensions for cube texture.");
            break;
        }
    }

    desc.width = std::max(desc.width, 1u);
    desc.height = std::max(desc.height, 1u);
    desc.depth = std::max(desc.depth, 1u);

    if (desc.mip_count == 0) {
        desc.mip_count = stdx::bit_width(std::max({desc.width, desc.height, desc.depth}));
    }
    SGL_CHECK(desc.array_size >= 1, "Invalid array size.");
    SGL_CHECK(desc.sample_count >= 1, "Invalid sample count.");
}

Texture::Texture(ref<Device> device, TextureDesc desc)
    : Resource(std::move(device), desc.type)
    , m_desc(std::move(desc))
{
    process_texture_desc(m_desc);
    m_type = m_desc.type;

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
    gfx::ITextureResource::SubresourceData* gfx_init_data{nullptr};

    SLANG_CALL(m_device->gfx_device()->createTextureResource(gfx_desc, gfx_init_data, m_gfx_texture.writeRef()));

    if (!m_desc.debug_name.empty())
        m_gfx_texture->setDebugName(m_desc.debug_name.c_str());

    // Clear initial data fields in desc.
    m_desc.data = nullptr;
    m_desc.data_size = 0;
}

Texture::Texture(ref<Device> device, TextureDesc desc, gfx::ITextureResource* resource, bool deferred_release)
    : Resource(std::move(device), ResourceType(desc.type))
    , m_desc(std::move(desc))
{
    process_texture_desc(m_desc);

    m_gfx_texture = resource;
    m_deferred_release = deferred_release;
}

Texture::~Texture()
{
    if (m_deferred_release)
        m_device->deferred_release(m_gfx_texture);
}

SubresourceLayout Texture::get_subresource_layout(uint32_t subresource) const
{
    SGL_CHECK_LT(subresource, subresource_count());

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
    SGL_CHECK(desc.subresource_range.mip_level < mip_count, "'mip_level' out of range");
    SGL_CHECK(
        desc.subresource_range.mip_count == SubresourceRange::ALL
            || desc.subresource_range.mip_level + desc.subresource_range.mip_count <= mip_count,
        "'mip_count' out of range"
    );
    if (desc.subresource_range.mip_count == SubresourceRange::ALL) {
        desc.subresource_range.mip_count = mip_count - desc.subresource_range.mip_level;
    }
    uint32_t array_size = this->array_size();
    SGL_CHECK(desc.subresource_range.base_array_layer < array_size, "'base_array_layer' out of range");
    SGL_CHECK(
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
    SGL_CHECK_LT(mip_level, mip_count());
    SGL_CHECK_LT(array_slice, array_size());

    const FormatInfo& info = get_format_info(m_desc.format);
    if (info.is_compressed)
        SGL_THROW("Cannot convert compressed texture to bitmap.");
    if (info.is_depth_stencil())
        SGL_THROW("Cannot convert depth/stencil texture to bitmap.");
    if (info.is_typeless_format())
        SGL_THROW("Cannot convert typeless texture to bitmap.");
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

    uint32_t subresource = get_subresource_index(mip_level, array_slice);
    SubresourceLayout layout = get_subresource_layout(subresource);

    uint32_t width = get_mip_width(mip_level);
    uint32_t height = get_mip_height(mip_level);

    ref<Bitmap> bitmap = ref<Bitmap>(new Bitmap(pixel_format, component_type, width, height));

    size_t row_pitch, pixel_size;
    if (bitmap->buffer_size() == layout.total_size_aligned()) {
        // Fast path: read directly into bitmap buffer
        m_device->read_texture(this, bitmap->buffer_size(), bitmap->data(), &row_pitch, &pixel_size);
        SGL_ASSERT(row_pitch == layout.row_size_aligned);
        SGL_ASSERT(pixel_size == bitmap->bytes_per_pixel());
    } else {
        // Slow path: read into temporary buffer and copy row by row
        std::unique_ptr<uint8_t[]> data(new uint8_t[layout.total_size_aligned()]);
        m_device->read_texture(this, layout.total_size_aligned(), data.get(), &row_pitch, &pixel_size);
        SGL_ASSERT(row_pitch == layout.row_size_aligned);
        SGL_ASSERT(pixel_size == bitmap->bytes_per_pixel());
        size_t row_size = layout.row_size;
        size_t row_size_aligned = layout.row_size_aligned;
        size_t row_count = layout.row_count;
        for (size_t i = 0; i < row_count; ++i)
            std::memcpy(bitmap->uint8_data() + i * row_size, data.get() + i * row_size_aligned, row_size);
    }

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
        "  array_size = {},\n"
        "  sample_count = {},\n"
        "  format = {},\n"
        "  usage = {},\n"
        "  memory_type = {},\n"
        "  size = {},\n"
        "  debug_name = {}\n"
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
        string::format_byte_size(memory_usage().device),
        m_desc.debug_name
    );
}

} // namespace sgl