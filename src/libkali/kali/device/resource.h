#pragma once

#include "kali/device/fwd.h"
#include "kali/device/formats.h"
#include "kali/device/native_handle.h"

#include "kali/core/macros.h"
#include "kali/core/enum.h"
#include "kali/core/object.h"

#include <slang-gfx.h>

#include <unordered_map>
#include <limits>

namespace kali {

using DeviceAddress = uint64_t;
using DeviceOffset = uint64_t;
using DeviceSize = uint64_t;

enum class ResourceType : uint32_t {
    unknown = gfx::IResource::Type::Unknown,
    buffer = gfx::IResource::Type::Buffer,
    texture_1d = gfx::IResource::Type::Texture1D,
    texture_2d = gfx::IResource::Type::Texture2D,
    texture_3d = gfx::IResource::Type::Texture3D,
    texture_cube = gfx::IResource::Type::TextureCube,
};

KALI_ENUM_INFO(
    ResourceType,
    {
        {ResourceType::unknown, "unknown"},
        {ResourceType::buffer, "buffer"},
        {ResourceType::texture_1d, "texture_1d"},
        {ResourceType::texture_2d, "texture_2d"},
        {ResourceType::texture_3d, "texture_3d"},
        {ResourceType::texture_cube, "texture_cube"},
    }
);
KALI_ENUM_REGISTER(ResourceType);

enum class ResourceState : uint32_t {
    undefined = gfx::ResourceState::Undefined,
    general = gfx::ResourceState::General,
    pre_initialized = gfx::ResourceState::PreInitialized,
    vertex_buffer = gfx::ResourceState::VertexBuffer,
    index_buffer = gfx::ResourceState::IndexBuffer,
    constant_buffer = gfx::ResourceState::ConstantBuffer,
    stream_output = gfx::ResourceState::StreamOutput,
    shader_resource = gfx::ResourceState::ShaderResource,
    unordered_access = gfx::ResourceState::UnorderedAccess,
    render_target = gfx::ResourceState::RenderTarget,
    depth_read = gfx::ResourceState::DepthRead,
    depth_write = gfx::ResourceState::DepthWrite,
    present = gfx::ResourceState::Present,
    indirect_argument = gfx::ResourceState::IndirectArgument,
    copy_source = gfx::ResourceState::CopySource,
    copy_destination = gfx::ResourceState::CopyDestination,
    resolve_source = gfx::ResourceState::ResolveSource,
    resolve_destination = gfx::ResourceState::ResolveDestination,
    acceleration_structure = gfx::ResourceState::AccelerationStructure,
    acceleration_structure_build_output = gfx::ResourceState::AccelerationStructureBuildInput,
    pixel_shader_resource = gfx::ResourceState::PixelShaderResource,
    non_pixel_shader_resource = gfx::ResourceState::NonPixelShaderResource,
};

KALI_ENUM_INFO(
    ResourceState,
    {
        {ResourceState::undefined, "undefined"},
        {ResourceState::general, "general"},
        {ResourceState::pre_initialized, "pre_initialized"},
        {ResourceState::vertex_buffer, "vertex_buffer"},
        {ResourceState::index_buffer, "index_buffer"},
        {ResourceState::constant_buffer, "constant_buffer"},
        {ResourceState::stream_output, "stream_output"},
        {ResourceState::shader_resource, "shader_resource"},
        {ResourceState::unordered_access, "unordered_access"},
        {ResourceState::render_target, "render_target"},
        {ResourceState::depth_read, "depth_read"},
        {ResourceState::depth_write, "depth_write"},
        {ResourceState::present, "present"},
        {ResourceState::indirect_argument, "indirect_argument"},
        {ResourceState::copy_source, "copy_source"},
        {ResourceState::copy_destination, "copy_destination"},
        {ResourceState::resolve_source, "resolve_source"},
        {ResourceState::resolve_destination, "resolve_destination"},
        {ResourceState::acceleration_structure, "acceleration_structure"},
        {ResourceState::acceleration_structure_build_output, "acceleration_structure_build_output"},
        {ResourceState::pixel_shader_resource, "pixel_shader_resource"},
        {ResourceState::non_pixel_shader_resource, "non_pixel_shader_resource"},
    }
);
KALI_ENUM_REGISTER(ResourceState);

enum class ResourceUsage : uint32_t {
    none = 0x0,              ///< The resource will not be bound the pipeline. Use this to create a staging resource
    vertex = 0x1,            ///< The resource will be bound as a vertex-buffer
    index = 0x2,             ///< The resource will be bound as a index-buffer
    constant = 0x4,          ///< The resource will be bound as a constant-buffer
    stream_output = 0x8,     ///< The resource will be bound to the stream-output stage as an output buffer
    shader_resource = 0x10,  ///< The resource will be bound as a shader-resource
    unordered_access = 0x20, ///< The resource will be bound as an UAV
    render_target = 0x40,    ///< The resource will be bound as a render-target
    depth_stencil = 0x80,    ///< The resource will be bound as a depth-stencil buffer
    indirect_arg = 0x100,    ///< The resource will be bound as an indirect argument buffer
    shared = 0x200, ///< The resource will be shared with a different adapter. Mostly useful for sharing resoures with
                    ///< CUDA
    acceleration_structure = 0x80000000, ///< The resource will be bound as an acceleration structure
};

KALI_ENUM_INFO(
    ResourceUsage,
    {
        {ResourceUsage::none, "none"},
        {ResourceUsage::vertex, "vertex"},
        {ResourceUsage::index, "index"},
        {ResourceUsage::constant, "constant"},
        {ResourceUsage::stream_output, "stream_output"},
        {ResourceUsage::shader_resource, "shader_resource"},
        {ResourceUsage::unordered_access, "unordered_access"},
        {ResourceUsage::render_target, "render_target"},
        {ResourceUsage::depth_stencil, "depth_stencil"},
        {ResourceUsage::indirect_arg, "indirect_arg"},
        {ResourceUsage::shared, "shared"},
        {ResourceUsage::acceleration_structure, "acceleration_structure"},
    }
);
KALI_ENUM_REGISTER(ResourceUsage);

KALI_ENUM_CLASS_OPERATORS(ResourceUsage)

enum class MemoryType : uint32_t {
    device_local = gfx::MemoryType::DeviceLocal,
    upload = gfx::MemoryType::Upload,
    read_back = gfx::MemoryType::ReadBack,
};

KALI_ENUM_INFO(
    MemoryType,
    {
        {MemoryType::device_local, "device_local"},
        {MemoryType::upload, "upload"},
        {MemoryType::read_back, "read_back"},
    }
);
KALI_ENUM_REGISTER(MemoryType);

enum class ResourceViewType {
    unknown = gfx::IResourceView::Type::Unknown,
    render_target = gfx::IResourceView::Type::RenderTarget,
    depth_stencil = gfx::IResourceView::Type::DepthStencil,
    shader_resource = gfx::IResourceView::Type::ShaderResource,
    unordered_access = gfx::IResourceView::Type::UnorderedAccess,
    acceleration_structure = gfx::IResourceView::Type::AccelerationStructure,
};

KALI_ENUM_INFO(
    ResourceViewType,
    {
        {ResourceViewType::unknown, "unknown"},
        {ResourceViewType::render_target, "render_target"},
        {ResourceViewType::depth_stencil, "depth_stencil"},
        {ResourceViewType::shader_resource, "shader_resource"},
        {ResourceViewType::unordered_access, "unordered_access"},
        {ResourceViewType::acceleration_structure, "acceleration_structure"},
    }
);
KALI_ENUM_REGISTER(ResourceViewType);

struct BufferRange {
    static constexpr uint64_t ALL = std::numeric_limits<uint64_t>::max();
    uint64_t first_element{0};
    uint64_t element_count{ALL};
};

enum class TextureAspect : uint32_t {
    default_ = gfx::TextureAspect::Default,
    color = gfx::TextureAspect::Color,
    depth = gfx::TextureAspect::Depth,
    stencil = gfx::TextureAspect::Stencil,
    meta_data = gfx::TextureAspect::MetaData,
    plane0 = gfx::TextureAspect::Plane0,
    plane1 = gfx::TextureAspect::Plane1,
    plane2 = gfx::TextureAspect::Plane2,
    depth_stencil = depth | stencil,
};

struct SubresourceRange {
    static constexpr uint32_t ALL = std::numeric_limits<uint32_t>::max();
    /// Texture aspect.
    TextureAspect texture_aspect{TextureAspect::default_};
    /// Most detailed mip level.
    uint32_t mip_level{0};
    /// Number of mip levels.
    uint32_t mip_count{ALL};
    /// First array layer.
    uint32_t base_array_layer{0}; // For Texture3D, this is WSlice.
    /// Number of array layers.
    uint32_t layer_count{ALL}; // For cube maps, this is a multiple of 6.
};

struct ResourceViewDesc {
    static constexpr uint32_t MAX_POSSIBLE = std::numeric_limits<uint32_t>::max();

    ResourceViewType type{ResourceViewType::unknown};
    Format format{Format::unknown};

    // For buffer views.
    BufferRange buffer_range;
    uint64_t buffer_element_size;

    // For texture views.
    SubresourceRange subresource_range;

    bool operator==(const ResourceViewDesc& other) const
    {
        return (type == other.type) && (format == other.format)
            && (buffer_range.first_element == other.buffer_range.first_element)
            && (buffer_range.element_count == other.buffer_range.element_count)
            && (buffer_element_size == other.buffer_element_size);
    }
};

} // namespace kali

namespace std {
template<>
struct hash<::kali::ResourceViewDesc> {
    size_t operator()(const ::kali::ResourceViewDesc& desc) const noexcept
    {
        KALI_UNUSED(desc);
        return 0;
        // return hash<uint32_t>()(uint32_t(resource_type));
    }
};
} // namespace std

namespace kali {

class KALI_API ResourceView : public Object {
    KALI_OBJECT(ResourceView)
public:
    ResourceView(const ResourceViewDesc& desc, const Buffer* buffer);
    ResourceView(const ResourceViewDesc& desc, const Texture* texture);

    const ResourceViewDesc& desc() const { return m_desc; }

    ResourceViewType type() const { return m_desc.type; }

    const Resource* resource() const { return m_resource; }

    gfx::IResourceView* get_gfx_resource_view() const { return m_gfx_resource_view; }

    /// Returns the native API handle:
    /// - D3D12: D3D12_CPU_DESCRIPTOR_HANDLE
    /// - Vulkan: VkImageView for texture views, VkBufferView for typed buffer views, VkBuffer for untyped buffer views
    NativeHandle get_native_handle() const;

private:
    ResourceViewDesc m_desc;
    const Resource* m_resource{nullptr};
    Slang::ComPtr<gfx::IResourceView> m_gfx_resource_view;
};

class KALI_API Resource : public Object {
    KALI_OBJECT(Resource)
public:
    virtual ~Resource();

    Device* device() const { return m_device; }

    ResourceType type() const { return m_type; };

    Format format() const { return Format::unknown; }

    /// Return true if the whole resource has the same resource state.
    bool has_global_state() const { return true; }

    ResourceState global_state() const { return ResourceState::undefined; }

    const char* debug_name() const;
    void set_debug_name(const char* name);

    // virtual ref<ResourceView> get_rtv() const;
    // virtual ref<ResourceView> get_dsv() const;

    /// Get a shader resource view for the entire resource.
    virtual ref<ResourceView> get_srv() const = 0;

    /// Get a unordered access view for the entire resource.
    virtual ref<ResourceView> get_uav() const = 0;

    virtual gfx::IResource* get_gfx_resource() const = 0;

    /// Returns the native API handle:
    /// - D3D12: ID3D12Resource*
    /// - Vulkan: VkBuffer or VkImage
    NativeHandle get_native_handle() const;

protected:
    Resource() = delete;
    Resource(ref<Device> device, ResourceType type);

    void set_global_state(ResourceState global_state) const { KALI_UNUSED(global_state); }

    ref<Device> m_device;
    ResourceType m_type;

    mutable std::unordered_map<ResourceViewDesc, ref<ResourceView>> m_views;

    friend class ResourceView;
    friend class CommandStream;
};


struct BufferDesc {
    /// Buffer size in bytes.
    size_t size{0};
    /// Struct size in bytes. If > 0, this is a structured buffer.
    size_t struct_size{0};
    /// Buffer format. If != unknown, this is a typed buffer.
    Format format{Format::unknown};

    ResourceState initial_state{ResourceState::undefined};
    ResourceUsage usage{ResourceUsage::none};
    MemoryType memory_type{MemoryType::device_local};

    std::string debug_name;
};

class KALI_API Buffer : public Resource {
    KALI_OBJECT(Buffer)
public:
    Buffer(ref<Device> device, BufferDesc desc, const void* init_data);

    const BufferDesc& desc() const { return m_desc; }

    size_t size() const { return m_desc.size; }
    size_t struct_size() const { return m_desc.struct_size; }
    Format format() const { return m_desc.format; }

    bool is_structured() const { return m_desc.struct_size > 0; }
    bool is_typed() const { return m_desc.format != Format::unknown; }

    size_t element_size() const;
    size_t element_count() const;

    /// Map the whole buffer.
    /// Only available for buffers created with @c MemoryType::upload or @c MemoryType::read_back.
    void* map() const;

    template<typename T>
    T* map() const
    {
        return reinterpret_cast<T*>(map());
    }

    /// Map a range of the buffer.
    /// Only available for buffers created with @c MemoryType::upload or @c MemoryType::read_back.
    void* map(DeviceOffset offset, DeviceSize size_in_bytes) const;

    template<typename T>
    T* map(size_t offset, size_t count) const
    {
        return reinterpret_cast<T*>(map(offset * sizeof(T), count * sizeof(T)));
    }

    /// Unmap the buffer.
    void unmap() const;

    /// Returns true if buffer is currently mapped.
    bool is_mapped() const { return m_mapped_ptr != nullptr; }

    DeviceAddress device_address() const { return m_gfx_buffer->getDeviceAddress(); }

    /// Get a resource view. Views are cached and reused.
    ref<ResourceView> get_view(ResourceViewDesc desc) const;

    /// Get a shader resource view for a range of the buffer.
    ref<ResourceView> get_srv(uint64_t first_element, uint64_t element_count = BufferRange::ALL) const;

    /// Get a unordered access view for a range of the buffer.
    ref<ResourceView> get_uav(uint64_t first_element, uint64_t element_count = BufferRange::ALL) const;

    /// Get a shader resource view for the entire buffer.
    virtual ref<ResourceView> get_srv() const override;

    /// Get a unordered access view for the entire buffer.
    virtual ref<ResourceView> get_uav() const override;

    virtual gfx::IResource* get_gfx_resource() const override { return m_gfx_buffer; }
    gfx::IBufferResource* get_gfx_buffer_resource() const { return m_gfx_buffer; }

private:
    BufferDesc m_desc;
    Slang::ComPtr<gfx::IBufferResource> m_gfx_buffer;

    mutable void* m_mapped_ptr{nullptr};
};

enum class TextureType : uint32_t {
    unknown = uint32_t(ResourceType::unknown),
    texture_1d = uint32_t(ResourceType::texture_1d),
    texture_2d = uint32_t(ResourceType::texture_2d),
    texture_3d = uint32_t(ResourceType::texture_3d),
    texture_cube = uint32_t(ResourceType::texture_cube),
};

KALI_ENUM_INFO(
    TextureType,
    {
        {TextureType::unknown, "unknown"},
        {TextureType::texture_1d, "texture_1d"},
        {TextureType::texture_2d, "texture_2d"},
        {TextureType::texture_3d, "texture_3d"},
        {TextureType::texture_cube, "texture_cube"},
    }
);
KALI_ENUM_REGISTER(TextureType);

struct TextureDesc {
    /// Texture type.
    TextureType type{TextureType::unknown};
    /// Texture format.
    Format format{Format::unknown};
    /// Width in pixels.
    uint32_t width{0};
    /// Height in pixels.
    uint32_t height{0};
    /// Depth in pixels.
    uint32_t depth{0};
    /// Number of array slices (1 for non-array textures).
    uint32_t array_size{1};
    /// Number of mip levels (0 for auto-generated mips).
    uint32_t mip_count{0};
    /// Number of samples per pixel (1 for non-multisampled textures).
    uint32_t sample_count{1};
    /// Quality level for multisampled textures.
    uint32_t quality{0};

    // TODO(@skallweit): support clear value

    ResourceState initial_state{ResourceState::undefined};
    ResourceUsage usage{ResourceUsage::none};
    MemoryType memory_type{MemoryType::device_local};

    std::string debug_name;
};

class KALI_API Texture : public Resource {
    KALI_OBJECT(Texture)
public:
    Texture(ref<Device> device, TextureDesc desc, const void* init_data);
    Texture(ref<Device> device, TextureDesc desc, gfx::ITextureResource* resource);

    const TextureDesc& desc() const { return m_desc; }

    Format format() const { return m_desc.format; }
    uint32_t width() const { return m_desc.width; }
    uint32_t height() const { return m_desc.height; }
    uint32_t depth() const { return m_desc.depth; }

    uint32_t array_size() const { return m_desc.array_size; }
    uint32_t mip_count() const { return m_desc.mip_count; }
    uint32_t subresource_count() const
    {
        return array_size() * mip_count() * (type() == ResourceType::texture_cube ? 6 : 1);
    }

    uint32_t get_subresource_index(uint32_t mip_level, uint32_t array_slice) const
    {
        return mip_level + array_slice * mip_count();
    }

    uint32_t get_subresource_array_slice(uint32_t subresource) const { return subresource / mip_count(); }

    uint32_t get_subresource_mip_level(uint32_t subresource) const { return subresource % mip_count(); }

    uint32_t get_mip_width(uint32_t mip_level = 0) const
    {
        return (mip_level == 0) || (mip_level < mip_count()) ? std::max(1U, width() >> mip_level) : 0;
    }

    uint32_t get_mip_height(uint32_t mip_level = 0) const
    {
        return (mip_level == 0) || (mip_level < mip_count()) ? std::max(1U, height() >> mip_level) : 0;
    }

    uint32_t get_mip_depth(uint32_t mip_level = 0) const
    {
        return (mip_level == 0) || (mip_level < mip_count()) ? std::max(1U, depth() >> mip_level) : 0;
    }

    /// Get a resource view. Views are cached and reused.
    ref<ResourceView> get_view(ResourceViewDesc desc) const;

    ref<ResourceView> get_srv(
        uint32_t mip_level,
        uint32_t mip_count = SubresourceRange::ALL,
        uint32_t base_array_layer = 0,
        uint32_t layer_count = SubresourceRange::ALL
    ) const;

    ref<ResourceView>
    get_uav(uint32_t mip_level, uint32_t base_array_layer = 0, uint32_t layer_count = SubresourceRange::ALL) const;

    ref<ResourceView>
    get_dsv(uint32_t mip_level, uint32_t base_array_layer = 0, uint32_t layer_count = SubresourceRange::ALL) const;

    ref<ResourceView>
    get_rtv(uint32_t mip_level, uint32_t base_array_layer = 0, uint32_t layer_count = SubresourceRange::ALL) const;

    /// Get a shader resource view for the entire texture.
    virtual ref<ResourceView> get_srv() const override;

    /// Get a unordered access view for the entire texture.
    virtual ref<ResourceView> get_uav() const override;

    virtual gfx::IResource* get_gfx_resource() const override { return m_gfx_texture; }
    gfx::ITextureResource* get_gfx_texture_resource() const { return m_gfx_texture; }

private:
    TextureDesc m_desc;
    Slang::ComPtr<gfx::ITextureResource> m_gfx_texture;
};

} // namespace kali
