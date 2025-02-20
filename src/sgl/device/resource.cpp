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

inline gfx::ResourceStateSet gfx_resource_usage_set_from_usage(ResourceUsage usage)
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

inline ResourceUsage get_required_resource_usage(ResourceViewType type)
{
    switch (type) {
    case ResourceViewType::shader_resource:
        return ResourceUsage::shader_resource;
    case ResourceViewType::unordered_access:
        return ResourceUsage::unordered_access;
    case ResourceViewType::render_target:
        return ResourceUsage::render_target;
    case ResourceViewType::depth_stencil:
        return ResourceUsage::depth_stencil;
    default:
        return ResourceUsage::none;
    }
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
    invalidate_views();
}

void Resource::invalidate_views()
{
    for (auto& [desc, view] : m_views)
        view->invalidate(m_deferred_release);
    m_views.clear();
}

const Buffer* Resource::as_buffer() const
{
    SGL_ASSERT(m_type == ResourceType::buffer);
    return static_cast<const Buffer*>(this);
}

Buffer* Resource::as_buffer()
{
    SGL_ASSERT(m_type == ResourceType::buffer);
    return static_cast<Buffer*>(this);
}

const Texture* Resource::as_texture() const
{
    SGL_ASSERT(m_type != ResourceType::buffer);
    return static_cast<const Texture*>(this);
}

Texture* Resource::as_texture()
{
    SGL_ASSERT(m_type != ResourceType::buffer);
    return static_cast<Texture*>(this);
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

ResourceView::ResourceView(const ResourceViewDesc& desc, Buffer* buffer)
    : m_desc(desc)
    , m_resource(buffer)
{
    SGL_ASSERT(buffer);

    SGL_ASSERT(m_desc.type == ResourceViewType::shader_resource || m_desc.type == ResourceViewType::unordered_access);

    gfx::IResourceView::Desc gfx_desc{
        .type = static_cast<gfx::IResourceView::Type>(m_desc.type),
        .format = static_cast<gfx::Format>(m_desc.format),
        .bufferRange{
            .offset = m_desc.buffer_range.offset,
            .size = m_desc.buffer_range.size,
        },
    };
    // TODO handle uav counter
    SLANG_CALL(buffer->m_device->gfx_device()
                   ->createBufferView(buffer->gfx_buffer_resource(), nullptr, gfx_desc, m_gfx_resource_view.writeRef())
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

    gfx::IResourceView::Desc gfx_desc{
        .type = static_cast<gfx::IResourceView::Type>(m_desc.type),
        .format = static_cast<gfx::Format>(m_desc.format),
        .renderTarget{
            .shape = (m_desc.type == ResourceViewType::render_target || m_desc.type == ResourceViewType::depth_stencil)
                ? static_cast<gfx::IResource::Type>(texture->type())
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

// ----------------------------------------------------------------------------
// Buffer
// ----------------------------------------------------------------------------

Buffer::Buffer(ref<Device> device, BufferDesc desc)
    : Resource(std::move(device), ResourceType::buffer)
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

    // Override initial state if not specified.
    if (m_desc.initial_state == ResourceState::undefined) {
        if (is_set(m_desc.usage, ResourceUsage::acceleration_structure)) {
            m_desc.initial_state = ResourceState::acceleration_structure;
            m_desc.usage |= ResourceUsage::unordered_access | ResourceUsage::shader_resource;
        }
    }

    m_state_tracker.set_global_state(m_desc.initial_state);

    gfx::IBufferResource::Desc gfx_desc{};
    gfx_desc.type = gfx::IResource::Type::Buffer;
    gfx_desc.defaultState = static_cast<gfx::ResourceState>(m_desc.initial_state);
    gfx_desc.allowedStates = gfx_resource_usage_set_from_usage(m_desc.usage);
    gfx_desc.memoryType = static_cast<gfx::MemoryType>(m_desc.memory_type);
    // TODO(@skallweit): add support for existing handles
    // gfx_desc.existingHandle =
    gfx_desc.isShared = is_set(m_desc.usage, ResourceUsage::shared);
    gfx_desc.sizeInBytes = static_cast<gfx::Size>(m_desc.size);
    gfx_desc.elementSize = static_cast<gfx::Size>(m_desc.struct_size);
    gfx_desc.format = static_cast<gfx::Format>(m_desc.format);

    SLANG_CALL(m_device->gfx_device()->createBufferResource(gfx_desc, nullptr, m_gfx_buffer.writeRef()));

    if (!m_desc.debug_name.empty())
        m_gfx_buffer->setDebugName(m_desc.debug_name.c_str());

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
    m_device->deferred_release(m_gfx_buffer);
}

void* Buffer::map() const
{
    SGL_ASSERT(m_desc.memory_type != MemoryType::device_local);
    SGL_ASSERT(m_mapped_ptr == nullptr);
    SLANG_CALL(m_gfx_buffer->map(nullptr, &m_mapped_ptr));
    return m_mapped_ptr;
}

void* Buffer::map(DeviceAddress offset, DeviceSize size) const
{
    SGL_ASSERT(m_desc.memory_type != MemoryType::device_local);
    SGL_ASSERT(m_mapped_ptr == nullptr);
    gfx::MemoryRange gfx_read_range{
        .offset = offset,
        .size = size,
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
        m_device->upload_buffer_data(this, data, size, offset);
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

ref<ResourceView> Buffer::get_view(ResourceViewDesc desc)
{
    SGL_CHECK(
        is_set(m_desc.usage, get_required_resource_usage(desc.type)),
        "Buffer does not support view type {}",
        desc.type
    );
    size_t size = this->size();
    SGL_CHECK(desc.buffer_range.offset < size, "'offset' out of range");
    SGL_CHECK(
        (desc.buffer_range.size == BufferRange::ALL) || (desc.buffer_range.offset + desc.buffer_range.size <= size),
        "'size' out of range"
    );

    if (desc.buffer_range.size == BufferRange::ALL) {
        desc.buffer_range.size = size - desc.buffer_range.offset;
    }

    auto it = m_views.find(desc);
    if (it != m_views.end())
        return it->second;
    auto view = make_ref<ResourceView>(desc, this);
    m_views.emplace(desc, view);
    return view;
}

ref<ResourceView> Buffer::get_srv(BufferRange range)
{
    return get_view({
        .type = ResourceViewType::shader_resource,
        .format = m_desc.format,
        .buffer_range = range,
        .buffer_element_size = m_desc.struct_size,
    });
}

ref<ResourceView> Buffer::get_uav(BufferRange range)
{
    return get_view({
        .type = ResourceViewType::unordered_access,
        .format = m_desc.format,
        .buffer_range = range,
        .buffer_element_size = m_desc.struct_size,
    });
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
        case ResourceType::buffer:
            SGL_THROW("Invalid texture type.");
            break;
        case ResourceType::texture_1d:
            SGL_CHECK(
                desc.width > 0 && desc.height <= 1 && desc.depth <= 1,
                "Invalid dimensions (width={}, height={}, depth={}) for 1D texture.",
                desc.width,
                desc.height,
                desc.depth
            );
            break;
        case ResourceType::texture_2d:
            SGL_CHECK(
                desc.width > 0 && desc.height > 0 && desc.depth <= 1,
                "Invalid dimensions (width={}, height={}, depth={}) for 2D texture.",
                desc.width,
                desc.height,
                desc.depth
            );
            break;
        case ResourceType::texture_3d:
            SGL_CHECK(
                desc.width > 0 && desc.height > 0 && desc.depth > 0,
                "Invalid dimensions (width={}, height={}, depth={}) for 3D texture.",
                desc.width,
                desc.height,
                desc.depth
            );
            break;
        case ResourceType::texture_cube:
            SGL_CHECK(
                desc.width > 0 && desc.height > 0 && desc.depth <= 1,
                "Invalid dimensions (width={}, height={}, depth={}) for cube texture.",
                desc.width,
                desc.height,
                desc.depth
            );
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

    m_state_tracker.set_global_state(m_desc.initial_state);

    // Check if format supports requested resource states.
    gfx::ResourceStateSet gfx_allowed_states = gfx_resource_usage_set_from_usage(m_desc.usage);
    gfx::ResourceStateSet gfx_supported_states;
    SLANG_CALL(m_device->gfx_device()
                   ->getFormatSupportedResourceStates(static_cast<gfx::Format>(m_desc.format), &gfx_supported_states));
    // TODO remove this workaround when D3D12 reports CopySource and CopyDestination states for formats
    gfx_supported_states.add(gfx::ResourceState::CopySource);
    gfx_supported_states.add(gfx::ResourceState::CopyDestination);
    static_vector<ResourceState, size_t(gfx::ResourceState::_Count)> unsupported_states;
    for (uint32_t i = 0; i < uint32_t(gfx::ResourceState::_Count); ++i)
        if (gfx_allowed_states.contains(gfx::ResourceState(i)) && !gfx_supported_states.contains(gfx::ResourceState(i)))
            unsupported_states.push_back(ResourceState(i));
    if (!unsupported_states.empty()) {
        SGL_THROW(
            "Format {} does not support requested resource states: {}",
            m_desc.format,
            fmt::join(unsupported_states, ", ")
        );
    }
    gfx::ITextureResource::Desc gfx_desc{};
    gfx_desc.type = static_cast<gfx::IResource::Type>(m_desc.type);
    gfx_desc.defaultState = static_cast<gfx::ResourceState>(m_desc.initial_state);
    gfx_desc.allowedStates = gfx_allowed_states;
    gfx_desc.memoryType = static_cast<gfx::MemoryType>(m_desc.memory_type);
    // TODO(@skallweit): add support for existing handles
    // gfx_desc.existingHandle =
    gfx_desc.isShared = is_set(m_desc.usage, ResourceUsage::shared);
    gfx_desc.size.width = static_cast<gfx::Size>(m_desc.width);
    gfx_desc.size.height = static_cast<gfx::Size>(m_desc.height);
    gfx_desc.size.depth = static_cast<gfx::Size>(m_desc.depth);
    gfx_desc.numMipLevels = static_cast<gfx::Size>(m_desc.mip_count);
    // slang-gfx uses 0 for non-array texture
    gfx_desc.arraySize = static_cast<gfx::Size>((m_desc.array_size == 1) ? 0 : m_desc.array_size);
    gfx_desc.format = static_cast<gfx::Format>(m_desc.format);
    gfx_desc.sampleDesc.numSamples = static_cast<gfx::GfxCount>(m_desc.sample_count);
    gfx_desc.sampleDesc.quality = m_desc.quality;

    SLANG_CALL(m_device->gfx_device()->createTextureResource(gfx_desc, nullptr, m_gfx_texture.writeRef()));

    if (!m_desc.debug_name.empty())
        m_gfx_texture->setDebugName(m_desc.debug_name.c_str());

    // Upload init data.
    if (m_desc.data) {
        uint32_t subresource = 0;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(m_desc.data);
        size_t data_size = m_desc.data_size;
        while (true) {
            SubresourceLayout subresource_layout = get_subresource_layout(subresource);
            size_t subresource_size = subresource_layout.total_size();
            if (data_size < subresource_size)
                break;
            SubresourceData subresource_data{
                .data = data,
                .size = subresource_size,
                .row_pitch = subresource_layout.row_pitch,
                .slice_pitch = subresource_layout.row_count * subresource_layout.row_pitch,
            };
            set_subresource_data(subresource, subresource_data);
            data += subresource_size;
            data_size -= subresource_size;
        }
        if (m_desc.mip_count > 1) {
            // TODO generate mip maps
        }
    }

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
    layout.row_pitch
        = div_round_up(mip_dimensions.x, uint32_t(gfx_format_info.blockWidth)) * gfx_format_info.blockSizeInBytes;
    layout.row_pitch_aligned = align_to(alignment, layout.row_pitch);
    layout.row_count = div_round_up(mip_dimensions.y, uint32_t(gfx_format_info.blockHeight));
    layout.depth = mip_dimensions.z;
    return layout;
}

void Texture::set_subresource_data(uint32_t subresource, SubresourceData subresource_data)
{
    SGL_CHECK_LT(subresource, subresource_count());

    m_device->upload_texture_data(this, subresource, subresource_data);
}

OwnedSubresourceData Texture::get_subresource_data(uint32_t subresource) const
{
    SGL_CHECK_LT(subresource, subresource_count());

    return m_device->read_texture_data(this, subresource);
}

ref<ResourceView> Texture::get_view(ResourceViewDesc desc)
{
    SGL_CHECK(
        is_set(m_desc.usage, get_required_resource_usage(desc.type)),
        "Texture does not support view type {}",
        desc.type
    );
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

ref<ResourceView> Texture::get_srv(SubresourceRange range)
{
    return get_view({
        .type = ResourceViewType::shader_resource,
        .format = m_desc.format,
        .subresource_range = range,
    });
}

ref<ResourceView> Texture::get_uav(SubresourceRange range)
{
    return get_view({
        .type = ResourceViewType::unordered_access,
        .format = m_desc.format,
        .subresource_range = range,
    });
}

ref<ResourceView> Texture::get_dsv(SubresourceRange range)
{
    return get_view({
        .type = ResourceViewType::depth_stencil,
        .format = m_desc.format,
        .subresource_range = range,
    });
}

ref<ResourceView> Texture::get_rtv(SubresourceRange range)
{
    return get_view({
        .type = ResourceViewType::render_target,
        .format = m_desc.format,
        .subresource_range = range,
    });
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
    SGL_CHECK(m_desc.type == ResourceType::texture_2d, "Cannot convert non-2D texture to bitmap.");

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
    OwnedSubresourceData subresource_data = get_subresource_data(subresource);

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
