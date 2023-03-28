#include "resource.h"
#include "device.h"
#include "helpers.h"

#include "core/assert.h"
#include "core/error.h"

namespace kali {

gfx::IResource::Type get_gfx_resource_type(ResourceType resource_type)
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

inline gfx::MemoryType get_gfx_memory_type(MemoryType memory_type)
{
    static_assert(uint32_t(MemoryType::device_local) == uint32_t(gfx::MemoryType::DeviceLocal));
    static_assert(uint32_t(MemoryType::upload) == uint32_t(gfx::MemoryType::Upload));
    static_assert(uint32_t(MemoryType::read_back) == uint32_t(gfx::MemoryType::ReadBack));
    KALI_ASSERT(uint32_t(memory_type) <= uint32_t(MemoryType::read_back));
    return gfx::MemoryType(memory_type);
}

inline gfx::MemoryType get_gfx_memory_type(CpuAccess cpu_access)
{
    switch (cpu_access) {
    case CpuAccess::none:
        return gfx::MemoryType::DeviceLocal;
    case CpuAccess::write:
        return gfx::MemoryType::Upload;
    case CpuAccess::read:
        return gfx::MemoryType::ReadBack;
    }
    KALI_THROW(Exception("Invalid CpuAccess value"));
}

inline gfx::ResourceStateSet get_gfx_allowed_states(ResourceUsage resource_usage)
{
    KALI_UNUSED(resource_usage);
    return {};
}

inline gfx::MemoryRange get_gfx_memory_range(MemoryRange memory_range)
{
    return gfx::MemoryRange{
        .offset = memory_range.offset,
        .size = memory_range.size,
    };
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
    gfx::IBufferResource::Desc gfx_desc{};
    gfx_desc.type = gfx::IResource::Type::Buffer;
    gfx_desc.defaultState = get_gfx_resource_state(m_desc.initial_state);
    // gfx_desc.allowedStates = ResourceStateSet();
    gfx_desc.memoryType = get_gfx_memory_type(m_desc.cpu_access);
    // gfx_desc.existingHandle =
    // gfx_desc.isShared =
    gfx_desc.sizeInBytes = gfx::Size(m_desc.size);
    gfx_desc.elementSize = gfx::Size(m_desc.struct_size);
    gfx_desc.format = get_gfx_format(m_desc.format);

    SLANG_CALL(m_device->get_gfx_device()->createBufferResource(gfx_desc, init_data, m_gfx_buffer.writeRef()));
}

void* Buffer::map(std::optional<MemoryRange> read_range)
{
    KALI_ASSERT(m_desc.cpu_access != CpuAccess::none);

    gfx::MemoryRange gfx_read_range;
    if (read_range)
        gfx_read_range = get_gfx_memory_range(*read_range);
    void* ptr;
    SLANG_CALL(m_gfx_buffer->map(read_range ? &gfx_read_range : nullptr, &ptr));
    return ptr;
}

void Buffer::unmap(std::optional<MemoryRange> write_range)
{
    KALI_ASSERT(m_desc.cpu_access != CpuAccess::none);

    gfx::MemoryRange gfx_write_range;
    if (write_range)
        gfx_write_range = get_gfx_memory_range(*write_range);
    SLANG_CALL(m_gfx_buffer->unmap(write_range ? &gfx_write_range : nullptr));
}

// ----------------------------------------------------------------------------
// Texture
// ----------------------------------------------------------------------------


} // namespace kali
