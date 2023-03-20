#pragma once

#include "fwd.h"
#include "formats.h"

#include "core/platform.h"
#include "core/object.h"

#include <slang-gfx.h>

#include <optional>

namespace kali {

using DeviceAddress = uint64_t;

enum class ResourceType {
    unknown,
    buffer,
    texture_1d,
    texture_2d,
    texture_3d,
    texture_cube,
};

enum class ResourceState {
    undefined = 0,
    General = (1 << 0),
    PreInitialized = (1 << 1),
    VertexBuffer = (1 << 2),
    IndexBuffer = (1 << 3),
    ConstantBuffer = (1 << 4),
    StreamOutput = (1 << 5),
    ShaderResource = (1 << 6),
    UnorderedAccess = (1 << 7),
    RenderTarget = (1 << 8),
    DepthRead = (1 << 9),
    DepthWrite = (1 << 10),
    Present = (1 << 11),
    IndirectArgument = (1 << 12),
    CopySource = (1 << 13),
    CopyDestination = (1 << 14),
    ResolveSource = (1 << 15),
    ResolveDestination = (1 << 16),
    AccelerationStructure = (1 << 17),
    AccelerationStructureBuildInput = (1 << 18),
    PixelShaderResource = (1 << 19),
    NonPixelShaderResource = (1 << 20),
};

enum class MemoryType {
    device_local,
    upload,
    read_back,
};

struct MemoryRange {
    uint64_t offset;
    uint64_t size;
};

struct ResourceDesc {
    ResourceType type{ResourceType::unknown};
    ResourceState default_state{};
    // ResourceStateSet allowed_states{};
    MemoryType memory_type{MemoryType::device_local};
    // bool shared
    // existing handle
};

class KALI_API Resource : public Object {
public:
    virtual DeviceAddress get_device_address() const = 0;
    virtual gfx::IResource* get_gfx_resource() const = 0;

private:
};

struct BufferDesc : ResourceDesc {
    Format format{Format::unknown};
    size_t size; ///< Total size in bytes.
    size_t element_size; ///< Element size in bytes. If > 0, this is a structured buffer.
};

class KALI_API Buffer : public Resource {
public:
    Buffer(const BufferDesc& desc, const void* init_data, ref<Device> device);

    const BufferDesc& get_desc() const { return m_desc; }

    void* map(std::optional<MemoryRange> read_range);

    template<typename T>
    T* map(std::optional<MemoryRange> read_range)
    {
        return reinterpret_cast<T*>(map(read_range));
    }

    void unmap(std::optional<MemoryRange> write_range);

    virtual DeviceAddress get_device_address() const override { return m_gfx_buffer->getDeviceAddress(); }
    virtual gfx::IResource* get_gfx_resource() const override { return m_gfx_buffer; }

private:
    BufferDesc m_desc;
    ref<Device> m_device;
    Slang::ComPtr<gfx::IBufferResource> m_gfx_buffer;
};

struct TextureDesc : ResourceDesc { };

class KALI_API Texture : public Resource {
public:
    virtual gfx::IResource* get_gfx_resource() const override { return m_gfx_texture; }

private:
    Slang::ComPtr<gfx::ITextureResource> m_gfx_texture;
};

} // namespace kali
