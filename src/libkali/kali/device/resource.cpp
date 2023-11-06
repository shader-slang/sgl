#include "resource.h"

#include "kali/device/device.h"
#include "kali/device/helpers.h"
#include "kali/device/native_handle_traits.h"

#include "kali/core/error.h"

namespace kali {

inline gfx::ResourceStateSet get_gfx_allowed_states(ResourceUsage usage)
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

inline gfx::ResourceState get_gfx_initial_state(ResourceUsage usage)
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
    : m_device(device)
    , m_type(type)
{
}

Resource::~Resource() { }

const char* Resource::debug_name() const
{
    return get_gfx_resource()->getDebugName();
}

void Resource::set_debug_name(const char* name)
{
    get_gfx_resource()->setDebugName(name);
}

NativeHandle Resource::get_native_handle() const
{
    gfx::InteropHandle handle = {};
    SLANG_CALL(get_gfx_resource()->getNativeResourceHandle(&handle));
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
        .format = get_gfx_format(m_desc.format),
        .bufferRange{
            .firstElement = m_desc.buffer_range.first_element,
            .elementCount = m_desc.buffer_range.element_count,
        },
        .bufferElementSize = m_desc.buffer_element_size,
    };
    // TODO handle uav counter
    SLANG_CALL(
        buffer->m_device->get_gfx_device()
            ->createBufferView(buffer->get_gfx_buffer_resource(), nullptr, gfx_desc, m_gfx_resource_view.writeRef())
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
        .format = get_gfx_format(m_desc.format),
    };
    SLANG_CALL(texture->m_device->get_gfx_device()
                   ->createTextureView(texture->get_gfx_texture_resource(), gfx_desc, m_gfx_resource_view.writeRef()));
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

Buffer::Buffer(ref<Device> device, BufferDesc desc, const void* init_data)
    : Resource(std::move(device), ResourceType::buffer)
    , m_desc(std::move(desc))
{
    KALI_ASSERT(m_desc.size > 0);
    KALI_ASSERT(m_desc.struct_size == 0 || m_desc.format == Format::unknown);
    KALI_ASSERT(m_desc.struct_size == 0 || m_desc.size % m_desc.struct_size == 0);

    gfx::IBufferResource::Desc gfx_desc{};
    gfx_desc.type = gfx::IResource::Type::Buffer;
    gfx_desc.defaultState = static_cast<gfx::ResourceState>(m_desc.initial_state);
    gfx_desc.allowedStates = get_gfx_allowed_states(m_desc.usage);
    gfx_desc.memoryType = static_cast<gfx::MemoryType>(m_desc.memory_type);
    // TODO(@skallweit): add support for existing handles
    // gfx_desc.existingHandle =
    gfx_desc.isShared = is_set(m_desc.usage, ResourceUsage::shared);
    gfx_desc.sizeInBytes = gfx::Size(m_desc.size);
    gfx_desc.elementSize = gfx::Size(m_desc.struct_size);
    gfx_desc.format = get_gfx_format(m_desc.format);

    SLANG_CALL(m_device->get_gfx_device()->createBufferResource(gfx_desc, init_data, m_gfx_buffer.writeRef()));
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
        desc.buffer_range.element_count == BufferRange::WHOLE
            || desc.buffer_range.first_element + desc.buffer_range.element_count <= element_count,
        "'element_count' out of range"
    );

    if (desc.buffer_range.element_count == BufferRange::WHOLE) {
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
    ResourceViewDesc desc{
        .type = ResourceViewType::shader_resource,
        .format = m_desc.format,
        .buffer_range{.first_element = first_element, .element_count = element_count},
    };
    return get_view(desc);
}

ref<ResourceView> Buffer::get_uav(uint64_t first_element, uint64_t element_count) const
{
    ResourceViewDesc desc{
        .type = ResourceViewType::unordered_access,
        .format = m_desc.format,
        .buffer_range{.first_element = first_element, .element_count = element_count},
    };
    return get_view(desc);
}

ref<ResourceView> Buffer::get_srv() const
{
    return get_srv(0);
}

ref<ResourceView> Buffer::get_uav() const
{
    return get_uav(0);
}

// ----------------------------------------------------------------------------
// Texture
// ----------------------------------------------------------------------------

static void check_desc(const TextureDesc& desc)
{
    KALI_ASSERT(desc.width > 0);
    KALI_ASSERT(desc.height > 0);
    KALI_ASSERT(desc.depth > 0);
    KALI_ASSERT(desc.mip_count > 0);
    // KALI_ASSERT(desc.array_size > 0);
    KALI_ASSERT(desc.format != Format::unknown);
}

Texture::Texture(ref<Device> device, TextureDesc desc, const void* init_data)
    : Resource(std::move(device), ResourceType(desc.type))
    , m_desc(std::move(desc))
{
    check_desc(m_desc);

    gfx::ITextureResource::Desc gfx_desc{};
    gfx_desc.type = static_cast<gfx::IResource::Type>(m_desc.type);
    gfx_desc.defaultState = static_cast<gfx::ResourceState>(m_desc.initial_state);
    gfx_desc.allowedStates = get_gfx_allowed_states(m_desc.usage);
    gfx_desc.memoryType = static_cast<gfx::MemoryType>(m_desc.memory_type);
    // TODO(@skallweit): add support for existing handles
    // gfx_desc.existingHandle =
    gfx_desc.isShared = is_set(m_desc.usage, ResourceUsage::shared);
    gfx_desc.size.width = gfx::Size(m_desc.width);
    gfx_desc.size.height = gfx::Size(m_desc.height);
    gfx_desc.size.depth = gfx::Size(m_desc.depth);
    gfx_desc.numMipLevels = gfx::Size(m_desc.mip_count);
    gfx_desc.arraySize = gfx::Size(m_desc.array_size);
    gfx_desc.format = get_gfx_format(m_desc.format);
    gfx_desc.sampleDesc.numSamples = gfx::GfxCount(m_desc.sample_count);
    gfx_desc.sampleDesc.quality = m_desc.quality;

    // TODO(@skallweit): add support for init data
    KALI_UNUSED(init_data);
    gfx::ITextureResource::SubresourceData* gfx_init_data{nullptr};

    SLANG_CALL(m_device->get_gfx_device()->createTextureResource(gfx_desc, gfx_init_data, m_gfx_texture.writeRef()));
}

Texture::Texture(ref<Device> device, TextureDesc desc, gfx::ITextureResource* resource)
    : Resource(std::move(device), ResourceType(desc.type))
    , m_desc(std::move(desc))
{
    check_desc(m_desc);

    m_gfx_texture = resource;
}

ref<ResourceView> Texture::get_view(const ResourceViewDesc& desc) const
{
    auto it = m_views.find(desc);
    if (it != m_views.end())
        return it->second;
    auto view = make_ref<ResourceView>(desc, this);
    m_views.emplace(desc, view);
    return view;
}

ref<ResourceView> Texture::get_srv() const
{
    return {};
}

ref<ResourceView> Texture::get_uav() const
{
    return {};
}

} // namespace kali
