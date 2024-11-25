// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/types.h"
#include "sgl/device/device_resource.h"
#include "sgl/device/formats.h"
#include "sgl/device/shared_handle.h"
#include "sgl/device/native_handle.h"

#include "sgl/core/fwd.h"
#include "sgl/core/macros.h"
#include "sgl/core/enum.h"
#include "sgl/core/object.h"

#include <slang-gfx.h>

#include <map>
#include <set>
#include <limits>

namespace sgl {

enum class ResourceType : uint32_t {
    unknown = static_cast<uint32_t>(gfx::IResource::Type::Unknown),
    buffer = static_cast<uint32_t>(gfx::IResource::Type::Buffer),
    texture_1d = static_cast<uint32_t>(gfx::IResource::Type::Texture1D),
    texture_2d = static_cast<uint32_t>(gfx::IResource::Type::Texture2D),
    texture_3d = static_cast<uint32_t>(gfx::IResource::Type::Texture3D),
    texture_cube = static_cast<uint32_t>(gfx::IResource::Type::TextureCube),
};

SGL_ENUM_INFO(
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
SGL_ENUM_REGISTER(ResourceType);

enum class ResourceState : uint32_t {
    undefined = static_cast<uint32_t>(gfx::ResourceState::Undefined),
    general = static_cast<uint32_t>(gfx::ResourceState::General),
    pre_initialized = static_cast<uint32_t>(gfx::ResourceState::PreInitialized),
    vertex_buffer = static_cast<uint32_t>(gfx::ResourceState::VertexBuffer),
    index_buffer = static_cast<uint32_t>(gfx::ResourceState::IndexBuffer),
    constant_buffer = static_cast<uint32_t>(gfx::ResourceState::ConstantBuffer),
    stream_output = static_cast<uint32_t>(gfx::ResourceState::StreamOutput),
    shader_resource = static_cast<uint32_t>(gfx::ResourceState::ShaderResource),
    unordered_access = static_cast<uint32_t>(gfx::ResourceState::UnorderedAccess),
    render_target = static_cast<uint32_t>(gfx::ResourceState::RenderTarget),
    depth_read = static_cast<uint32_t>(gfx::ResourceState::DepthRead),
    depth_write = static_cast<uint32_t>(gfx::ResourceState::DepthWrite),
    present = static_cast<uint32_t>(gfx::ResourceState::Present),
    indirect_argument = static_cast<uint32_t>(gfx::ResourceState::IndirectArgument),
    copy_source = static_cast<uint32_t>(gfx::ResourceState::CopySource),
    copy_destination = static_cast<uint32_t>(gfx::ResourceState::CopyDestination),
    resolve_source = static_cast<uint32_t>(gfx::ResourceState::ResolveSource),
    resolve_destination = static_cast<uint32_t>(gfx::ResourceState::ResolveDestination),
    acceleration_structure = static_cast<uint32_t>(gfx::ResourceState::AccelerationStructure),
    acceleration_structure_build_output = static_cast<uint32_t>(gfx::ResourceState::AccelerationStructureBuildInput),
    pixel_shader_resource = static_cast<uint32_t>(gfx::ResourceState::PixelShaderResource),
    non_pixel_shader_resource = static_cast<uint32_t>(gfx::ResourceState::NonPixelShaderResource),
};

SGL_ENUM_INFO(
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
SGL_ENUM_REGISTER(ResourceState);

using ResourceStateSet = std::set<ResourceState>;

enum class ResourceUsage : uint32_t {
    /// The resource will not be bound the pipeline. Use this to create a staging resource.
    none = 0x0,
    /// The resource will be bound as a vertex-buffer.
    vertex = 0x1,
    /// The resource will be bound as a index-buffer.
    index = 0x2,
    /// The resource will be bound as a constant-buffer.
    constant = 0x4,
    /// The resource will be bound to the stream-output stage as an output buffer.
    stream_output = 0x8,
    /// The resource will be bound as a shader-resource.
    shader_resource = 0x10,
    /// The resource will be bound as an UAV.
    unordered_access = 0x20,
    /// The resource will be bound as a render-target.
    render_target = 0x40,
    /// The resource will be bound as a depth-stencil buffer.
    depth_stencil = 0x80,
    /// The resource will be bound as an indirect argument buffer.
    indirect_arg = 0x100,
    /// The resource will be shared with a different adapter. Mostly useful for sharing resoures with CUDA.
    shared = 0x200,
    /// The resource will be bound as an acceleration structure
    acceleration_structure = 0x80000000,
};

SGL_ENUM_INFO(
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
SGL_ENUM_REGISTER(ResourceUsage);

SGL_ENUM_CLASS_OPERATORS(ResourceUsage)

enum class MemoryType : uint32_t {
    device_local = static_cast<uint32_t>(gfx::MemoryType::DeviceLocal),
    upload = static_cast<uint32_t>(gfx::MemoryType::Upload),
    read_back = static_cast<uint32_t>(gfx::MemoryType::ReadBack),
};

SGL_ENUM_INFO(
    MemoryType,
    {
        {MemoryType::device_local, "device_local"},
        {MemoryType::upload, "upload"},
        {MemoryType::read_back, "read_back"},
    }
);
SGL_ENUM_REGISTER(MemoryType);

enum class ResourceViewType : uint32_t {
    unknown = static_cast<uint32_t>(gfx::IResourceView::Type::Unknown),
    render_target = static_cast<uint32_t>(gfx::IResourceView::Type::RenderTarget),
    depth_stencil = static_cast<uint32_t>(gfx::IResourceView::Type::DepthStencil),
    shader_resource = static_cast<uint32_t>(gfx::IResourceView::Type::ShaderResource),
    unordered_access = static_cast<uint32_t>(gfx::IResourceView::Type::UnorderedAccess),
    // acceleration_structure = static_cast<uint32_t>(gfx::IResourceView::Type::AccelerationStructure),
};

SGL_ENUM_INFO(
    ResourceViewType,
    {
        {ResourceViewType::unknown, "unknown"},
        {ResourceViewType::render_target, "render_target"},
        {ResourceViewType::depth_stencil, "depth_stencil"},
        {ResourceViewType::shader_resource, "shader_resource"},
        {ResourceViewType::unordered_access, "unordered_access"},
        // {ResourceViewType::acceleration_structure, "acceleration_structure"},
    }
);
SGL_ENUM_REGISTER(ResourceViewType);

struct BufferRange {
    static constexpr uint64_t ALL = std::numeric_limits<uint64_t>::max();
    uint64_t offset{0};
    uint64_t size{ALL};

    auto operator<=>(const BufferRange&) const = default;

    std::string to_string() const { return fmt::format("BufferRange(offset={}, size={}", offset, size); }
};

enum class TextureAspect : uint32_t {
    default_ = static_cast<uint32_t>(gfx::TextureAspect::Default),
    color = static_cast<uint32_t>(gfx::TextureAspect::Color),
    depth = static_cast<uint32_t>(gfx::TextureAspect::Depth),
    stencil = static_cast<uint32_t>(gfx::TextureAspect::Stencil),
    meta_data = static_cast<uint32_t>(gfx::TextureAspect::MetaData),
    plane0 = static_cast<uint32_t>(gfx::TextureAspect::Plane0),
    plane1 = static_cast<uint32_t>(gfx::TextureAspect::Plane1),
    plane2 = static_cast<uint32_t>(gfx::TextureAspect::Plane2),
    depth_stencil = depth | stencil,
};
SGL_ENUM_INFO(
    TextureAspect,
    {
        {TextureAspect::default_, "default_"},
        {TextureAspect::color, "color"},
        {TextureAspect::depth, "depth"},
        {TextureAspect::stencil, "stencil"},
        {TextureAspect::meta_data, "meta_data"},
        {TextureAspect::plane0, "plane0"},
        {TextureAspect::plane1, "plane1"},
        {TextureAspect::plane2, "plane2"},
        {TextureAspect::depth_stencil, "depth_stencil"},

    }
);
SGL_ENUM_REGISTER(TextureAspect);

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

    auto operator<=>(const SubresourceRange&) const = default;

    std::string to_string() const
    {
        return fmt::format(
            "SubresourceRange(texture_aspect={}, mip_level={}, mip_count={}, base_array_layer={}, layer_count={}",
            texture_aspect,
            mip_level,
            mip_count,
            base_array_layer,
            layer_count
        );
    }
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

    auto operator<=>(const ResourceViewDesc&) const = default;
};

class SGL_API ResourceView : public Object {
    SGL_OBJECT(ResourceView)
public:
    ResourceView(const ResourceViewDesc& desc, Buffer* buffer);
    ResourceView(const ResourceViewDesc& desc, Texture* texture);

    ~ResourceView();

    void invalidate(bool deferred_release);

    const ResourceViewDesc& desc() const { return m_desc; }

    ResourceViewType type() const { return m_desc.type; }

    Resource* resource() const { return m_resource; }

    const BufferRange& buffer_range() const { return m_desc.buffer_range; }
    const SubresourceRange& subresource_range() const { return m_desc.subresource_range; }

    /// True if the view covers all subresources.
    bool all_subresources() const { return m_all_subresources; }

    gfx::IResourceView* gfx_resource_view() const { return m_gfx_resource_view; }

    /// Returns the native API handle:
    /// - D3D12: D3D12_CPU_DESCRIPTOR_HANDLE
    /// - Vulkan: VkImageView for texture views, VkBufferView for typed buffer views, VkBuffer for untyped buffer views
    NativeHandle get_native_handle() const;

    std::string to_string() const override;

private:
    ResourceViewDesc m_desc;
    Resource* m_resource{nullptr};
    bool m_all_subresources{false};
    Slang::ComPtr<gfx::IResourceView> m_gfx_resource_view;
};

class ResourceStateTracker {
public:
    ResourceStateTracker() = default;

    bool has_global_state() const { return m_has_global_state; }

    ResourceState global_state() const { return m_global_state; }

    void set_global_state(ResourceState state)
    {
        m_has_global_state = true;
        m_global_state = state;
    }

    ResourceState subresource_state(uint32_t subresource) const
    {
        if (m_has_global_state)
            return m_global_state;
        SGL_ASSERT(m_subresource_states);
        if (subresource >= m_subresource_states->size())
            return m_global_state;
        return m_subresource_states->operator[](subresource);
    }

    void set_subresource_state(uint32_t subresource, ResourceState state)
    {
        if (m_has_global_state && (state == m_global_state))
            return;
        m_has_global_state = false;
        if (!m_subresource_states)
            m_subresource_states = std::make_unique<std::vector<ResourceState>>(subresource + 1, m_global_state);
        if (subresource >= m_subresource_states->size())
            m_subresource_states->resize(subresource + 1, m_global_state);
        m_subresource_states->operator[](subresource) = state;
    }

private:
    bool m_has_global_state{true};
    ResourceState m_global_state{ResourceState::undefined};
    std::unique_ptr<std::vector<ResourceState>> m_subresource_states;
};

class SGL_API Resource : public DeviceResource {
    SGL_OBJECT(Resource)
public:
    virtual ~Resource();

    ResourceType type() const { return m_type; };

    virtual Format format() const = 0;

    ResourceStateTracker& state_tracker() const { return m_state_tracker; }

    void invalidate_views();

    const Buffer* as_buffer() const;
    Buffer* as_buffer();

    const Texture* as_texture() const;
    Texture* as_texture();

    virtual gfx::IResource* gfx_resource() const = 0;

    /// Get the shared resource handle.
    SharedResourceHandle get_shared_handle() const;

    /// Returns the native API handle:
    /// - D3D12: ID3D12Resource*
    /// - Vulkan: VkBuffer or VkImage
    NativeHandle get_native_handle() const;

protected:
    Resource() = delete;
    Resource(ref<Device> device, ResourceType type);

    ResourceType m_type;
    mutable ResourceStateTracker m_state_tracker;

    std::map<ResourceViewDesc, ref<ResourceView>> m_views;

    bool m_deferred_release{true};

    friend class ResourceView;
};


struct BufferDesc {
    /// Buffer size in bytes.
    size_t size{0};

    /// Buffer size in number of struct elements. Can be used instead of \c size.
    size_t element_count{0};
    /// Struct size in bytes.
    size_t struct_size{0};
    /// Struct type. Can be used instead of \c struct_size to specify the size of the struct.
    ref<const TypeLayoutReflection> struct_type;

    /// Buffer format. Used when creating typed buffer views.
    Format format{Format::unknown};

    /// Initial resource state.
    ResourceState initial_state{ResourceState::undefined};
    /// Resource usage flags.
    ResourceUsage usage{ResourceUsage::none};
    /// Memory type.
    MemoryType memory_type{MemoryType::device_local};

    /// Resource debug name.
    std::string debug_name;

    /// Initial data to upload to the buffer.
    const void* data{nullptr};
    /// Size of the initial data in bytes.
    size_t data_size{0};
};

class SGL_API Buffer : public Resource {
    SGL_OBJECT(Buffer)
public:
    Buffer(ref<Device> device, BufferDesc desc);

    ~Buffer();

    const BufferDesc& desc() const { return m_desc; }

    size_t size() const { return m_desc.size; }
    size_t struct_size() const { return m_desc.struct_size; }
    Format format() const override { return m_desc.format; }
    MemoryType memory_type() const { return m_desc.memory_type; }

    /// Map the whole buffer.
    /// Only available for buffers created with \c MemoryType::upload or \c MemoryType::read_back.
    void* map() const;

    template<typename T>
    T* map() const
    {
        return reinterpret_cast<T*>(map());
    }

    /// Map a range of the buffer.
    /// Only available for buffers created with \c MemoryType::upload or \c MemoryType::read_back.
    void* map(DeviceOffset offset, DeviceSize size) const;

    template<typename T>
    T* map(size_t offset, size_t count) const
    {
        return reinterpret_cast<T*>(map(offset * sizeof(T), count * sizeof(T)));
    }

    /// Unmap the buffer.
    void unmap() const;

    /// Returns true if buffer is currently mapped.
    bool is_mapped() const { return m_mapped_ptr != nullptr; }

    /// Returns a pointer to the CUDA memory.
    /// This is only supported if the buffer was created with ResourceUsage::shared
    /// and the device has CUDA interop enabled.
    void* cuda_memory() const;

    /**
     * Set buffer data from host memory.
     *
     * \param data Data to write.
     * \param size Size of the data in bytes.
     * \param offset Offset in the buffer to write to.
     */
    void set_data(const void* data, size_t size, DeviceOffset offset = 0);

    template<typename T>
    void set_element(size_t index, const T& value)
    {
        set_data(&value, sizeof(T), index * sizeof(T));
    }

    template<typename T>
    void set_elements(size_t index, std::span<const T> values)
    {
        set_data(values.data(), values.size() * sizeof(T), index * sizeof(T));
    }

    /**
     * Get buffer data to host memory.
     * \note If the buffer is in device local memory, this will wait until the data is copied back to host memory.
     *
     * \param data Data buffer to read to.
     * \param size Size of the data in bytes.
     * \param offset Offset in the buffer to read from.
     */
    void get_data(void* data, size_t size, DeviceOffset offset = 0);

    template<typename T>
    void get_element(size_t index)
    {
        T value;
        get_data(&value, sizeof(T), index * sizeof(T));
        return value;
    }

    template<typename T>
    std::vector<T> get_elements(size_t index = 0, size_t count = 0)
    {
        if (count == 0)
            count = (m_desc.size / sizeof(T)) - index;
        std::vector<T> values(count);
        get_data(values.data(), values.size() * sizeof(T), index * sizeof(T));
        return values;
    }

    DeviceAddress device_address() const { return m_gfx_buffer->getDeviceAddress(); }

    /// Get a resource view. Views are cached and reused.
    ref<ResourceView> get_view(ResourceViewDesc desc);

    /// Get a shader resource view for a range of the buffer.
    ref<ResourceView> get_srv(BufferRange range = BufferRange());

    /// Get a unordered access view for a range of the buffer.
    ref<ResourceView> get_uav(BufferRange range = BufferRange());

    virtual gfx::IResource* gfx_resource() const override { return m_gfx_buffer; }
    gfx::IBufferResource* gfx_buffer_resource() const { return m_gfx_buffer; }

    MemoryUsage memory_usage() const override;

    std::string to_string() const override;

private:
    BufferDesc m_desc;
    Slang::ComPtr<gfx::IBufferResource> m_gfx_buffer;
    mutable ref<cuda::ExternalMemory> m_cuda_memory;
    mutable void* m_mapped_ptr{nullptr};
};

struct SubresourceData {
    const void* data{nullptr};
    size_t size{0};
    size_t row_pitch{0};
    size_t slice_pitch{0};
};

struct OwnedSubresourceData : SubresourceData {
    std::unique_ptr<uint8_t[]> owned_data;
};

struct TextureDesc {
    /// Resource type (optional). Type is inferred from width, height, depth if not specified.
    ResourceType type{ResourceType::unknown};
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

    const void* data{nullptr};
    size_t data_size{0};
};

struct SubresourceLayout {
    /// Size of a single row in bytes (unaligned).
    size_t row_pitch;
    /// Size of a single row in bytes (aligned to device texture alignment).
    size_t row_pitch_aligned;
    /// Number of rows.
    size_t row_count;
    /// Number of depth slices.
    size_t depth;

    /// Get the total size of the subresource in bytes (unaligned).
    size_t total_size() const { return row_pitch * row_count * depth; }

    /// Get the total size of the subresource in bytes (aligned to device texture alignment).
    size_t total_size_aligned() const { return row_pitch_aligned * row_count * depth; }
};

class SGL_API Texture : public Resource {
    SGL_OBJECT(Texture)
public:
    Texture(ref<Device> device, TextureDesc desc);
    Texture(ref<Device> device, TextureDesc desc, gfx::ITextureResource* resource, bool deferred_release);

    ~Texture();

    const TextureDesc& desc() const { return m_desc; }

    Format format() const override { return m_desc.format; }
    uint32_t width() const { return m_desc.width; }
    uint32_t height() const { return m_desc.height; }
    uint32_t depth() const { return m_desc.depth; }

    uint32_t array_size() const { return m_desc.array_size; }
    uint32_t mip_count() const { return m_desc.mip_count; }
    uint32_t layer_count() const { return array_size() * (type() == ResourceType::texture_cube ? 6 : 1); }

    uint32_t subresource_count() const { return layer_count() * mip_count(); }

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

    uint3 get_mip_dimensions(uint32_t mip_level = 0) const
    {
        return uint3(get_mip_width(mip_level), get_mip_height(mip_level), get_mip_depth(mip_level));
    }

    SubresourceLayout get_subresource_layout(uint32_t subresource) const;

    /**
     * Set subresource data from host memory.
     *
     * \param subresource Subresource index.
     * \param subresource_data Subresource data.
     */
    void set_subresource_data(uint32_t subresource, SubresourceData subresource_data);

    /**
     * Get subresource data to host memory.
     * \note This will wait until the data is copied back to host memory.
     *
     * \param subresource Subresource index.
     * \return Subresource data.
     */
    OwnedSubresourceData get_subresource_data(uint32_t subresource) const;

    /// Get a resource view. Views are cached and reused.
    ref<ResourceView> get_view(ResourceViewDesc desc);

    /// Get a shader resource view for a subresource range of the texture.
    ref<ResourceView> get_srv(SubresourceRange range = SubresourceRange());

    /// Get a unordered access view for a subresource range of the texture.
    /// \note Only a single mip level can be bound.
    ref<ResourceView> get_uav(SubresourceRange range = SubresourceRange());

    /// Get a depth stencil view for a subresource range of the texture.
    /// \note Only a single mip level can be bound.
    ref<ResourceView> get_dsv(SubresourceRange range = SubresourceRange());

    /// Get a render target view for a subresource range of the texture.
    /// \note Only a single mip level can be bound.
    ref<ResourceView> get_rtv(SubresourceRange range = SubresourceRange());

    virtual gfx::IResource* gfx_resource() const override { return m_gfx_texture; }
    gfx::ITextureResource* gfx_texture_resource() const { return m_gfx_texture; }

    MemoryUsage memory_usage() const override;

    ref<Bitmap> to_bitmap(uint32_t mip_level = 0, uint32_t array_slice = 0) const;

    std::string to_string() const override;

private:
    TextureDesc m_desc;
    Slang::ComPtr<gfx::ITextureResource> m_gfx_texture;
};

} // namespace sgl
