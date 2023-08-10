#pragma once

#include "kali/rhi/fwd.h"
#include "kali/rhi/formats.h"
#include "kali/rhi/native_handle.h"

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
    unknown,
    buffer,
    texture_1d,
    texture_2d,
    texture_3d,
    texture_cube,
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
    undefined,
    general,
    pre_initialized,
    vertex_buffer,
    index_buffer,
    constant_buffer,
    stream_output,
    shader_resource,
    unordered_access,
    render_target,
    depth_read,
    depth_write,
    present,
    indirect_argument,
    copy_source,
    copy_destination,
    resolve_source,
    resolve_destination,
    acceleration_structure,
    acceleration_structure_build_output,
    pixel_shader_resource,
    non_pixel_shader_resource,
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
    device_local,
    upload,
    read_back,
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

struct ResourceViewRange {
    static constexpr uint32_t MAX_POSSIBLE = 0xffffffff;

    // Textures
    uint32_t most_detailed_mip = 0;
    uint32_t mip_count = MAX_POSSIBLE;
    uint32_t first_array_slice = 0;
    uint32_t array_size = MAX_POSSIBLE;

    // Buffers
    uint32_t first_element = 0;
    uint32_t element_count = MAX_POSSIBLE;

    ResourceViewRange() = default;
    ResourceViewRange(uint32_t most_detailed_mip, uint32_t mip_count, uint32_t first_array_slice, uint32_t array_size)
        : most_detailed_mip(most_detailed_mip)
        , mip_count(mip_count)
        , first_array_slice(first_array_slice)
        , array_size(array_size)
    {
    }

    ResourceViewRange(uint32_t first_element, uint32_t element_count)
        : first_element(first_element)
        , element_count(element_count)
    {
    }

    bool operator==(const ResourceViewRange& other) const
    {
        return (first_array_slice == other.first_array_slice) && (array_size == other.array_size)
            && (mip_count == other.mip_count) && (most_detailed_mip == other.most_detailed_mip)
            && (first_element == other.first_element) && (element_count == other.element_count);
    }
};

enum class ResourceViewType {
    unknown,
    render_target,
    depth_stencil,
    shader_resource,
    unordered_access,
    acceleration_structure,
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
    static constexpr uint64_t WHOLE = std::numeric_limits<uint64_t>::max();
    uint64_t first_element{0};
    uint64_t element_count{WHOLE};
};

struct SubresourceRange {
    // TextureAspect aspectMask;
    uint32_t mip_level;
    uint32_t mip_level_count;
    uint32_t base_array_layer; // For Texture3D, this is WSlice.
    uint32_t layer_count;      // For cube maps, this is a multiple of 6.
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

    ResourceViewType get_type() const { return m_desc.type; }

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

using MipLevel = uint32_t;
using ArraySlice = uint32_t;

class KALI_API Resource : public Object {
    KALI_OBJECT(Resource)
public:
    virtual ~Resource();

    ResourceType get_type() const { return m_type; };

    Format get_format() const { return Format::unknown; }

    /// Return true if the whole resource has the same resource state.
    bool has_global_state() const { return true; }

    ResourceState get_global_state() const { return ResourceState::undefined; }

    void set_debug_name(const char* name);
    const char* get_debug_name() const;

    // virtual ref<ResourceView> get_rtv() const;
    // virtual ref<ResourceView> get_dsv() const;

    /// Get a shader resource view for the entire resource.
    virtual ref<ResourceView> get_srv() const = 0;

    /// Get a unordered access view for the entire resource.
    virtual ref<ResourceView> get_uav() const = 0;

    virtual DeviceAddress get_device_address() const = 0;
    virtual gfx::IResource* get_gfx_resource() const = 0;

    /// Returns the native API handle:
    /// - D3D12: ID3D12Resource*
    /// - Vulkan: VkBuffer or VkImage
    NativeHandle get_native_handle() const;

protected:
    Resource() = delete;
    Resource(ResourceType type, ref<Device> device);

    void set_global_state(ResourceState global_state) const { KALI_UNUSED(global_state); }

    ResourceType m_type;
    ref<Device> m_device;

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

#if 0
    static BufferDesc create() { return {}; }
    static BufferDesc create_raw(size_t size) { return {.size = size}; }
    static BufferDesc create_typed(Format format, size_t element_count) { return {.size = size, .format = format}; };
    static BufferDesc create_structured(size_t struct_size, size_t struct_count)
    {
        return {.size = size, .struct_size = struct_size};
    }
    template<typename T>
    // static BufferDesc create_structured()
#endif

    // clang-format off
    BufferDesc& set_desc(size_t size_) { size = size_; return *this; }
    BufferDesc& set_struct_size(size_t struct_size_) { struct_size = struct_size_; return *this; }
    BufferDesc& set_format(Format format_) { format = format_; return *this; }
    // clang-format on

    // bool shared
    // existing handle
};

class KALI_API Buffer : public Resource {
    KALI_OBJECT(Buffer)
public:
    Buffer(BufferDesc desc, const void* init_data, ref<Device> device);

    const BufferDesc& get_desc() const { return m_desc; }

    size_t get_size() const { return m_desc.size; }
    size_t get_struct_size() const { return m_desc.struct_size; }
    Format get_format() const { return m_desc.format; }

    /// Map the whole buffer.
    /// Only available for buffers created with MemoryType::upload or MemoryType::read_back.
    void* map() const;

    template<typename T>
    T* map() const
    {
        return reinterpret_cast<T*>(map());
    }

    /// Map a range of the buffer.
    /// Only available for buffers created with MemoryType::upload or MemoryType::read_back.
    void* map(DeviceOffset offset, DeviceSize size_in_bytes) const;

    /// Unmap the buffer.
    void unmap() const;

    /// Returns true if buffer is currently mapped.
    bool is_mapped() const { return m_mapped_ptr != nullptr; }

    /// Get a resource view. Views are cached and reused.
    ref<ResourceView> get_view(const ResourceViewDesc& desc) const;

    /// Get a shader resource view for a range of the buffer.
    ref<ResourceView> get_srv(uint64_t first_element, uint64_t element_count = BufferRange::WHOLE) const;

    /// Get a unordered access view for a range of the buffer.
    ref<ResourceView> get_uav(uint64_t first_element, uint64_t element_count = BufferRange::WHOLE) const;

    /// Get a shader resource view for the entire buffer.
    virtual ref<ResourceView> get_srv() const override;

    /// Get a unordered access view for the entire buffer.
    virtual ref<ResourceView> get_uav() const override;

    virtual DeviceAddress get_device_address() const override { return m_gfx_buffer->getDeviceAddress(); }
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
    /// Height in pixels (1 for 1D textures).
    uint32_t height{1};
    /// Depth in pixels (1for 1D/2D textures).
    uint32_t depth{1};
    /// Number of array slices (0 for non-array textures).
    uint32_t array_size{0};
    /// Number of mip levels (0 for auto-generated mips).
    uint32_t mip_count{0};
    /// Number of samples per pixel (0 for non-multisampled textures).
    uint32_t sample_count{0};
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
    Texture(TextureDesc desc, const void* init_data, ref<Device> device);

    const TextureDesc& get_desc() const { return m_desc; }

    uint32_t get_width(MipLevel mip_level = 0) const
    {
        KALI_UNUSED(mip_level);
        return 0;
    }
    uint32_t get_height(MipLevel mip_level = 0) const
    {
        KALI_UNUSED(mip_level);
        return 0;
    }
    uint32_t get_depth(MipLevel mip_level = 0) const
    {
        KALI_UNUSED(mip_level);
        return 0;
    }

    uint32_t get_array_size() const { return 1; }
    uint32_t get_mip_count() const { return 1; }
    uint32_t get_subresource_count() const
    {
        return get_array_size() * get_mip_count() * (get_type() == ResourceType::texture_cube ? 6 : 1);
    }

    /// Get a resource view. Views are cached and reused.
    ref<ResourceView> get_view(const ResourceViewDesc& desc) const;

    /// Get a shader resource view for the entire texture.
    virtual ref<ResourceView> get_srv() const override;

    /// Get a unordered access view for the entire texture.
    virtual ref<ResourceView> get_uav() const override;

    virtual DeviceAddress get_device_address() const override { return 0; }
    virtual gfx::IResource* get_gfx_resource() const override { return m_gfx_texture; }
    gfx::ITextureResource* get_gfx_texture_resource() const { return m_gfx_texture; }

private:
    TextureDesc m_desc;
    Slang::ComPtr<gfx::ITextureResource> m_gfx_texture;
};

} // namespace kali
