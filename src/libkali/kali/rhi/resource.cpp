#include "resource.h"

#include "kali/rhi/device.h"
#include "kali/rhi/helpers.h"

#include "kali/error.h"

namespace kali {

inline gfx::IResource::Type get_gfx_resource_type(ResourceType resource_type)
{
    static_assert(uint32_t(ResourceType::unknown) == uint32_t(gfx::IResource::Type::Unknown));
    static_assert(uint32_t(ResourceType::buffer) == uint32_t(gfx::IResource::Type::Buffer));
    static_assert(uint32_t(ResourceType::texture_1d) == uint32_t(gfx::IResource::Type::Texture1D));
    static_assert(uint32_t(ResourceType::texture_2d) == uint32_t(gfx::IResource::Type::Texture2D));
    static_assert(uint32_t(ResourceType::texture_3d) == uint32_t(gfx::IResource::Type::Texture3D));
    static_assert(uint32_t(ResourceType::texture_cube) == uint32_t(gfx::IResource::Type::TextureCube));
    KALI_ASSERT(uint32_t(resource_type) <= uint32_t(ResourceType::texture_cube));
    return gfx::IResource::Type(resource_type);
}

gfx::ResourceState get_gfx_resource_state(ResourceState resource_state)
{
    // clang-format off
    static_assert(uint32_t(ResourceState::undefined) == uint32_t(gfx::ResourceState::Undefined));
    static_assert(uint32_t(ResourceState::general) == uint32_t(gfx::ResourceState::General));
    static_assert(uint32_t(ResourceState::pre_initialized) == uint32_t(gfx::ResourceState::PreInitialized));
    static_assert(uint32_t(ResourceState::vertex_buffer) == uint32_t(gfx::ResourceState::VertexBuffer));
    static_assert(uint32_t(ResourceState::index_buffer) == uint32_t(gfx::ResourceState::IndexBuffer));
    static_assert(uint32_t(ResourceState::constant_buffer) == uint32_t(gfx::ResourceState::ConstantBuffer));
    static_assert(uint32_t(ResourceState::stream_output) == uint32_t(gfx::ResourceState::StreamOutput));
    static_assert(uint32_t(ResourceState::shader_resource) == uint32_t(gfx::ResourceState::ShaderResource));
    static_assert(uint32_t(ResourceState::unordered_access) == uint32_t(gfx::ResourceState::UnorderedAccess));
    static_assert(uint32_t(ResourceState::render_target) == uint32_t(gfx::ResourceState::RenderTarget));
    static_assert(uint32_t(ResourceState::depth_read) == uint32_t(gfx::ResourceState::DepthRead));
    static_assert(uint32_t(ResourceState::depth_write) == uint32_t(gfx::ResourceState::DepthWrite));
    static_assert(uint32_t(ResourceState::present) == uint32_t(gfx::ResourceState::Present));
    static_assert(uint32_t(ResourceState::indirect_argument) == uint32_t(gfx::ResourceState::IndirectArgument));
    static_assert(uint32_t(ResourceState::copy_source) == uint32_t(gfx::ResourceState::CopySource));
    static_assert(uint32_t(ResourceState::copy_destination) == uint32_t(gfx::ResourceState::CopyDestination));
    static_assert(uint32_t(ResourceState::resolve_source) == uint32_t(gfx::ResourceState::ResolveSource));
    static_assert(uint32_t(ResourceState::resolve_destination) == uint32_t(gfx::ResourceState::ResolveDestination));
    static_assert(uint32_t(ResourceState::acceleration_structure) == uint32_t(gfx::ResourceState::AccelerationStructure));
    static_assert(uint32_t(ResourceState::acceleration_structure_build_output) == uint32_t(gfx::ResourceState::AccelerationStructureBuildInput));
    static_assert(uint32_t(ResourceState::pixel_shader_resource) == uint32_t(gfx::ResourceState::PixelShaderResource));
    static_assert(uint32_t(ResourceState::non_pixel_shader_resource) == uint32_t(gfx::ResourceState::NonPixelShaderResource));
    // clang-format on
    KALI_ASSERT(uint32_t(resource_state) <= uint32_t(ResourceState::non_pixel_shader_resource));
    return gfx::ResourceState(resource_state);
}

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


inline gfx::MemoryType get_gfx_memory_type(MemoryType memory_type)
{
    static_assert(uint32_t(MemoryType::device_local) == uint32_t(gfx::MemoryType::DeviceLocal));
    static_assert(uint32_t(MemoryType::upload) == uint32_t(gfx::MemoryType::Upload));
    static_assert(uint32_t(MemoryType::read_back) == uint32_t(gfx::MemoryType::ReadBack));
    KALI_ASSERT(uint32_t(memory_type) <= uint32_t(MemoryType::read_back));
    return gfx::MemoryType(memory_type);
}

// ----------------------------------------------------------------------------
// Resource
// ----------------------------------------------------------------------------

Resource::Resource(ResourceType type, ref<Device> device)
    : m_type(type)
    , m_device(device)
{
}

Resource::~Resource() { }

void Resource::set_debug_name(const char* name)
{
    get_gfx_resource()->setDebugName(name);
}

const char* Resource::get_debug_name() const
{
    return get_gfx_resource()->getDebugName();
}

// ----------------------------------------------------------------------------
// Buffer
// ----------------------------------------------------------------------------

Buffer::Buffer(const BufferDesc& desc, const void* init_data, ref<Device> device)
    : Resource(ResourceType::buffer, device)
    , m_desc(desc)
{
    KALI_ASSERT(m_desc.size > 0);
    KALI_ASSERT(m_desc.struct_size == 0 || m_desc.format == Format::unknown);
    KALI_ASSERT(m_desc.struct_size == 0 || m_desc.size % m_desc.struct_size == 0);

    gfx::IBufferResource::Desc gfx_desc{};
    gfx_desc.type = gfx::IResource::Type::Buffer;
    gfx_desc.defaultState = get_gfx_resource_state(m_desc.initial_state);
    gfx_desc.allowedStates = get_gfx_allowed_states(m_desc.usage);
    gfx_desc.memoryType = get_gfx_memory_type(m_desc.memory_type);
    // TODO(@skallweit): add support for existing handles
    // gfx_desc.existingHandle =
    gfx_desc.isShared = is_set(m_desc.usage, ResourceUsage::shared);
    gfx_desc.sizeInBytes = gfx::Size(m_desc.size);
    gfx_desc.elementSize = gfx::Size(m_desc.struct_size);
    gfx_desc.format = get_gfx_format(m_desc.format);

    SLANG_CALL(m_device->get_gfx_device()->createBufferResource(gfx_desc, init_data, m_gfx_buffer.writeRef()));
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

// ----------------------------------------------------------------------------
// Texture
// ----------------------------------------------------------------------------

Texture::Texture(const TextureDesc& desc, const void* init_data, ref<Device> device)
    : Resource(ResourceType(desc.type), device)
    , m_desc(desc)
{
    KALI_ASSERT(m_desc.width > 0);
    KALI_ASSERT(m_desc.height > 0);
    KALI_ASSERT(m_desc.depth > 0);
    KALI_ASSERT(m_desc.mip_count > 0);
    KALI_ASSERT(m_desc.array_size > 0);
    KALI_ASSERT(m_desc.format != Format::unknown);

    gfx::ITextureResource::Desc gfx_desc{};
    gfx_desc.type = get_gfx_resource_type(ResourceType(m_desc.type));
    gfx_desc.defaultState = get_gfx_resource_state(m_desc.initial_state);
    gfx_desc.allowedStates = get_gfx_allowed_states(m_desc.usage);
    gfx_desc.memoryType = get_gfx_memory_type(m_desc.memory_type);
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

} // namespace kali
