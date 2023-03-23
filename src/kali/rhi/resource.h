#pragma once

#include "fwd.h"
#include "formats.h"

#include "core/platform.h"
#include "core/object.h"

#include <slang-gfx.h>

#include <optional>

namespace kali {

using DeviceAddress = uint64_t;
using DeviceSize = uint64_t;

enum class ResourceType : uint32_t {
    unknown,
    buffer,
    texture_1d,
    texture_2d,
    texture_3d,
    texture_cube,
};

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

KALI_ENUM_CLASS_OPERATORS(ResourceUsage)

enum class CpuAccess : uint32_t {
    none,
    write,
    read,
};

enum class MemoryType : uint32_t {
    device_local,
    upload,
    read_back,
};

struct MemoryRange {
    uint64_t offset;
    uint64_t size;
};

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


using MipLevel = uint32_t;
using ArraySlice = uint32_t;

class KALI_API Resource : public Object {
public:
    virtual ~Resource();

    ResourceType get_type() const { return m_type; };

    Format get_format() const { return Format::unknown; }

    /// Return true if the whole resource has the same resource state.
    bool has_global_state() const { return true; }

    ResourceState get_global_state() const { return ResourceState::undefined; }

    void set_debug_name(const char* name);
    const char* get_debug_name() const;

    virtual DeviceAddress get_device_address() const = 0;
    virtual gfx::IResource* get_gfx_resource() const = 0;

protected:
    Resource() = delete;
    Resource(ResourceType type, ref<Device> device);

    void set_global_state(ResourceState global_state) const { KALI_UNUSED(global_state); }

    ResourceType m_type;
    ref<Device> m_device;

    friend class CommandList;
};

struct BufferDesc {
    size_t size{0};                 ///< Buffer size in bytes.
    size_t struct_size{0};          ///< Struct size in bytes. If > 0, this is a structured buffer.
    Format format{Format::unknown}; ///< Buffer format. If != unknown, this is a typed buffer.

    std::string debug_name; ///< Debug name.

    ResourceState initial_state{ResourceState::undefined};
    ResourceUsage usage{ResourceUsage::none};
    CpuAccess cpu_access{CpuAccess::none};

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
public:
    Buffer(const BufferDesc& desc, const void* init_data, ref<Device> device);

    const BufferDesc& get_desc() const { return m_desc; }

    size_t get_size() const { return m_desc.size; }
    size_t get_struct_size() const { return m_desc.struct_size; }
    Format get_format() const { return m_desc.format; }

    DeviceAddress get_gpu_address_offset() const { return 0; }

    void* map(std::optional<MemoryRange> read_range);

    template<typename T>
    T* map(std::optional<MemoryRange> read_range)
    {
        return reinterpret_cast<T*>(map(read_range));
    }

    void unmap(std::optional<MemoryRange> write_range);

    virtual DeviceAddress get_device_address() const override { return m_gfx_buffer->getDeviceAddress(); }
    virtual gfx::IResource* get_gfx_resource() const override { return m_gfx_buffer; }
    gfx::IBufferResource* get_gfx_buffer_resource() const { return m_gfx_buffer; }

private:
    BufferDesc m_desc;
    Slang::ComPtr<gfx::IBufferResource> m_gfx_buffer;
};

struct TextureDesc { };

class KALI_API Texture : public Resource {
public:
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

    virtual gfx::IResource* get_gfx_resource() const override { return m_gfx_texture; }
    gfx::ITextureResource* get_gfx_texture_resource() const { return m_gfx_texture; }

private:
    TextureDesc m_desc;
    Slang::ComPtr<gfx::ITextureResource> m_gfx_texture;
};

} // namespace kali
