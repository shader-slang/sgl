// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/types.h"
#include "sgl/device/device_resource.h"
#include "sgl/device/formats.h"
#include "sgl/device/native_handle.h"

#include "sgl/core/fwd.h"
#include "sgl/core/macros.h"
#include "sgl/core/enum.h"
#include "sgl/core/object.h"

#include <slang-rhi.h>

#include <map>
#include <set>
#include <limits>

namespace sgl {

enum class ResourceState : uint32_t {
    undefined = static_cast<uint32_t>(rhi::ResourceState::Undefined),
    general = static_cast<uint32_t>(rhi::ResourceState::General),
    vertex_buffer = static_cast<uint32_t>(rhi::ResourceState::VertexBuffer),
    index_buffer = static_cast<uint32_t>(rhi::ResourceState::IndexBuffer),
    constant_buffer = static_cast<uint32_t>(rhi::ResourceState::ConstantBuffer),
    stream_output = static_cast<uint32_t>(rhi::ResourceState::StreamOutput),
    shader_resource = static_cast<uint32_t>(rhi::ResourceState::ShaderResource),
    unordered_access = static_cast<uint32_t>(rhi::ResourceState::UnorderedAccess),
    render_target = static_cast<uint32_t>(rhi::ResourceState::RenderTarget),
    depth_read = static_cast<uint32_t>(rhi::ResourceState::DepthRead),
    depth_write = static_cast<uint32_t>(rhi::ResourceState::DepthWrite),
    present = static_cast<uint32_t>(rhi::ResourceState::Present),
    indirect_argument = static_cast<uint32_t>(rhi::ResourceState::IndirectArgument),
    copy_source = static_cast<uint32_t>(rhi::ResourceState::CopySource),
    copy_destination = static_cast<uint32_t>(rhi::ResourceState::CopyDestination),
    resolve_source = static_cast<uint32_t>(rhi::ResourceState::ResolveSource),
    resolve_destination = static_cast<uint32_t>(rhi::ResourceState::ResolveDestination),
    acceleration_structure = static_cast<uint32_t>(rhi::ResourceState::AccelerationStructure),
    acceleration_structure_build_output = static_cast<uint32_t>(rhi::ResourceState::AccelerationStructureBuildInput),
};

SGL_ENUM_INFO(
    ResourceState,
    {
        {ResourceState::undefined, "undefined"},
        {ResourceState::general, "general"},
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
    }
);
SGL_ENUM_REGISTER(ResourceState);

enum class BufferUsage : uint32_t {
    none = static_cast<uint32_t>(rhi::BufferUsage::None),
    vertex_buffer = static_cast<uint32_t>(rhi::BufferUsage::VertexBuffer),
    index_buffer = static_cast<uint32_t>(rhi::BufferUsage::IndexBuffer),
    constant_buffer = static_cast<uint32_t>(rhi::BufferUsage::ConstantBuffer),
    shader_resource = static_cast<uint32_t>(rhi::BufferUsage::ShaderResource),
    unordered_access = static_cast<uint32_t>(rhi::BufferUsage::UnorderedAccess),
    indirect_argument = static_cast<uint32_t>(rhi::BufferUsage::IndirectArgument),
    copy_source = static_cast<uint32_t>(rhi::BufferUsage::CopySource),
    copy_destination = static_cast<uint32_t>(rhi::BufferUsage::CopyDestination),
    acceleration_structure = static_cast<uint32_t>(rhi::BufferUsage::AccelerationStructure),
    acceleration_structure_build_input = static_cast<uint32_t>(rhi::BufferUsage::AccelerationStructureBuildInput),
    shader_table = static_cast<uint32_t>(rhi::BufferUsage::ShaderTable),
    shared = static_cast<uint32_t>(rhi::BufferUsage::Shared),
};
SGL_ENUM_CLASS_OPERATORS(BufferUsage);

SGL_ENUM_INFO(
    BufferUsage,
    {
        {BufferUsage::none, "none"},
        {BufferUsage::vertex_buffer, "vertex_buffer"},
        {BufferUsage::index_buffer, "index_buffer"},
        {BufferUsage::constant_buffer, "constant_buffer"},
        {BufferUsage::shader_resource, "shader_resource"},
        {BufferUsage::unordered_access, "unordered_access"},
        {BufferUsage::indirect_argument, "indirect_argument"},
        {BufferUsage::copy_source, "copy_source"},
        {BufferUsage::copy_destination, "copy_destination"},
        {BufferUsage::acceleration_structure, "acceleration_structure"},
        {BufferUsage::acceleration_structure_build_input, "acceleration_structure_build_input"},
        {BufferUsage::shader_table, "shader_table"},
        {BufferUsage::shared, "shared"},
    }
);
SGL_ENUM_REGISTER(BufferUsage);

enum class TextureUsage : uint32_t {
    none = static_cast<uint32_t>(rhi::TextureUsage::None),
    shader_resource = static_cast<uint32_t>(rhi::TextureUsage::ShaderResource),
    unordered_access = static_cast<uint32_t>(rhi::TextureUsage::UnorderedAccess),
    render_target = static_cast<uint32_t>(rhi::TextureUsage::RenderTarget),
    depth_read = static_cast<uint32_t>(rhi::TextureUsage::DepthRead),
    depth_write = static_cast<uint32_t>(rhi::TextureUsage::DepthWrite),
    present = static_cast<uint32_t>(rhi::TextureUsage::Present),
    copy_source = static_cast<uint32_t>(rhi::TextureUsage::CopySource),
    copy_destination = static_cast<uint32_t>(rhi::TextureUsage::CopyDestination),
    resolve_source = static_cast<uint32_t>(rhi::TextureUsage::ResolveSource),
    resolve_destination = static_cast<uint32_t>(rhi::TextureUsage::ResolveDestination),
    shared = static_cast<uint32_t>(rhi::TextureUsage::Shared),
};
SGL_ENUM_CLASS_OPERATORS(TextureUsage);

SGL_ENUM_INFO(
    TextureUsage,
    {
        {TextureUsage::none, "none"},
        {TextureUsage::shader_resource, "shader_resource"},
        {TextureUsage::unordered_access, "unordered_access"},
        {TextureUsage::render_target, "render_target"},
        {TextureUsage::depth_read, "depth_read"},
        {TextureUsage::depth_write, "depth_write"},
        {TextureUsage::present, "present"},
        {TextureUsage::copy_source, "copy_source"},
        {TextureUsage::copy_destination, "copy_destination"},
        {TextureUsage::resolve_source, "resolve_source"},
        {TextureUsage::resolve_destination, "resolve_destination"},
        {TextureUsage::shared, "shared"},
    }
);
SGL_ENUM_REGISTER(TextureUsage);

enum class TextureType : uint32_t {
    texture_1d = static_cast<uint32_t>(rhi::TextureType::Texture1D),
    texture_2d = static_cast<uint32_t>(rhi::TextureType::Texture2D),
    texture_3d = static_cast<uint32_t>(rhi::TextureType::Texture3D),
    texture_cube = static_cast<uint32_t>(rhi::TextureType::TextureCube),
};

SGL_ENUM_INFO(
    TextureType,
    {
        {TextureType::texture_1d, "texture_1d"},
        {TextureType::texture_2d, "texture_2d"},
        {TextureType::texture_3d, "texture_3d"},
        {TextureType::texture_cube, "texture_cube"},
    }
);
SGL_ENUM_REGISTER(TextureType);

enum class MemoryType : uint32_t {
    device_local = static_cast<uint32_t>(rhi::MemoryType::DeviceLocal),
    upload = static_cast<uint32_t>(rhi::MemoryType::Upload),
    read_back = static_cast<uint32_t>(rhi::MemoryType::ReadBack),
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

struct BufferRange {
    static constexpr uint64_t ALL = std::numeric_limits<uint64_t>::max();
    uint64_t offset{0};
    uint64_t size{ALL};

    auto operator<=>(const BufferRange&) const = default;

    std::string to_string() const { return fmt::format("BufferRange(offset={}, size={}", offset, size); }
};

enum class TextureAspect : uint32_t {
    all = static_cast<uint32_t>(rhi::TextureAspect::All),
    depth_only = static_cast<uint32_t>(rhi::TextureAspect::DepthOnly),
    stencil_only = static_cast<uint32_t>(rhi::TextureAspect::StencilOnly),
};
SGL_ENUM_INFO(
    TextureAspect,
    {
        {TextureAspect::all, "all"},
        {TextureAspect::depth_only, "depth_only"},
        {TextureAspect::stencil_only, "stencil_only"},
    }
);
SGL_ENUM_REGISTER(TextureAspect);

struct SubresourceRange {
    static constexpr uint32_t ALL = std::numeric_limits<uint32_t>::max();
    /// Texture aspect.
    TextureAspect texture_aspect{TextureAspect::all};
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

class SGL_API Resource : public DeviceResource {
    SGL_OBJECT(Resource)
public:
    virtual ~Resource();

    virtual rhi::IResource* rhi_resource() const = 0;

    /// Returns the native API handle:
    /// - D3D12: ID3D12Resource*
    /// - Vulkan: VkBuffer or VkImage
    NativeHandle get_native_handle() const;

protected:
    Resource() = delete;
    Resource(ref<Device> device);
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

    /// Memory type.
    MemoryType memory_type{MemoryType::device_local};

    /// Resource usage flags.
    BufferUsage usage{BufferUsage::none};
    /// Initial resource state.
    ResourceState default_state{ResourceState::undefined};

    /// Debug label.
    std::string label;

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
    Format format() const { return m_desc.format; }
    MemoryType memory_type() const { return m_desc.memory_type; }

    /// Map the whole buffer.
    /// Only available for buffers created with \c MemoryType::upload or \c MemoryType::read_back.
    void* map() const;

    template<typename T>
    T* map() const
    {
        return reinterpret_cast<T*>(map());
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

    DeviceAddress device_address() const { return m_rhi_buffer->getDeviceAddress(); }

    ref<BufferView> create_view(BufferViewDesc desc);

    /// Get the shared resource handle.
    NativeHandle get_shared_handle() const;

    virtual rhi::IResource* rhi_resource() const override { return m_rhi_buffer; }
    rhi::IBuffer* rhi_buffer() const { return m_rhi_buffer; }

    MemoryUsage memory_usage() const override;

    std::string to_string() const override;

private:
    BufferDesc m_desc;
    Slang::ComPtr<rhi::IBuffer> m_rhi_buffer;
    mutable ref<cuda::ExternalMemory> m_cuda_memory;
    mutable void* m_mapped_ptr{nullptr};
};

struct BufferViewDesc {
    Format format{Format::unknown};
    BufferRange range;
    std::string label;
};

class BufferView : public DeviceResource {
    SGL_OBJECT(BufferView)
public:
    BufferView(ref<Device> device, ref<Buffer> buffer, BufferViewDesc desc);

    Buffer* buffer() const { return m_buffer; }

    const BufferViewDesc& desc() const { return m_desc; }

    Format format() const { return m_desc.format; }
    const BufferRange& range() const { return m_desc.range; }
    std::string_view label() const { return m_desc.label; }

    NativeHandle get_native_handle() const;

    std::string to_string() const override;

private:
    ref<Buffer> m_buffer;
    BufferViewDesc m_desc;
    // Slang::ComPtr<rhi::IBufferView> m_rhi_buffer_view;
};

struct BufferOffsetPair {
    Buffer* buffer{nullptr};
    DeviceOffset offset{0};

    BufferOffsetPair() = default;
    BufferOffsetPair(Buffer* buffer, DeviceOffset offset = 0)
        : buffer(buffer)
        , offset(offset)
    {
    }
    BufferOffsetPair(const ref<Buffer>& buffer, DeviceOffset offset = 0)
        : buffer(buffer.get())
        , offset(offset)
    {
    }
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
    /// Texture type.
    TextureType type{TextureType::texture_2d};
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
    uint32_t sample_quality{0};

    // TODO(@skallweit): support clear value

    MemoryType memory_type{MemoryType::device_local};

    TextureUsage usage{TextureUsage::none};
    ResourceState default_state{ResourceState::undefined};

    /// Debug label.
    std::string label;

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
    Texture(ref<Device> device, TextureDesc desc, rhi::ITexture* resource);

    ~Texture();

    const TextureDesc& desc() const { return m_desc; }

    TextureType type() const { return m_desc.type; }
    Format format() const { return m_desc.format; }
    uint32_t width() const { return m_desc.width; }
    uint32_t height() const { return m_desc.height; }
    uint32_t depth() const { return m_desc.depth; }

    uint32_t array_size() const { return m_desc.array_size; }
    uint32_t mip_count() const { return m_desc.mip_count; }
    uint32_t layer_count() const { return array_size() * (type() == TextureType::texture_cube ? 6 : 1); }

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

    ref<TextureView> create_view(TextureViewDesc desc);

    /// Get the shared resource handle.
    NativeHandle get_shared_handle() const;

    virtual rhi::IResource* rhi_resource() const override { return m_rhi_texture; }
    rhi::ITexture* rhi_texture() const { return m_rhi_texture; }

    MemoryUsage memory_usage() const override;

    ref<Bitmap> to_bitmap(uint32_t mip_level = 0, uint32_t array_slice = 0) const;

    std::string to_string() const override;

private:
    TextureDesc m_desc;
    Slang::ComPtr<rhi::ITexture> m_rhi_texture;
};

struct TextureViewDesc {
    Format format{Format::unknown};
    TextureAspect aspect{TextureAspect::all};
    SubresourceRange subresource_range;
    std::string label;
};

class SGL_API TextureView : public DeviceResource {
    SGL_OBJECT(TextureView)
public:
    TextureView(ref<Device> device, ref<Texture> texture, TextureViewDesc desc);

    Texture* texture() const { return m_texture.get(); }

    const TextureViewDesc& desc() const { return m_desc; }

    Format format() const { return m_desc.format; }
    TextureAspect aspect() const { return m_desc.aspect; }
    const SubresourceRange& subresource_range() const { return m_desc.subresource_range; }
    std::string_view label() const { return m_desc.label; }

    NativeHandle get_native_handle() const;

    rhi::ITextureView* rhi_texture_view() const { return m_rhi_texture_view.get(); }

    std::string to_string() const override;

private:
    ref<Texture> m_texture;
    TextureViewDesc m_desc;
    Slang::ComPtr<rhi::ITextureView> m_rhi_texture_view;
};

} // namespace sgl
