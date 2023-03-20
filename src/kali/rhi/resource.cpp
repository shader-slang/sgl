#pragma once

#include "resource.h"
#include "device.h"

#include "core/error.h"

namespace kali {

inline gfx::IResource::Type get_gfx_resource_type(ResourceType resource_type)
{
    switch (resource_type) {
    case ResourceType::unknown:
        return gfx::IResource::Type::Unknown;
    case ResourceType::buffer:
        return gfx::IResource::Type::Buffer;
    case ResourceType::texture_1d:
        return gfx::IResource::Type::Texture1D;
    case ResourceType::texture_2d:
        return gfx::IResource::Type::Texture2D;
    case ResourceType::texture_3d:
        return gfx::IResource::Type::Texture3D;
    case ResourceType::texture_cube:
        return gfx::IResource::Type::TextureCube;
    }
    KALI_THROW(Exception("Invalid ResourceType"));
}

inline gfx::MemoryType get_gfx_memory_type(MemoryType memory_type)
{
    switch (memory_type) {
    case MemoryType::device_local:
        return gfx::MemoryType::DeviceLocal;
    case MemoryType::upload:
        return gfx::MemoryType::Upload;
    case MemoryType::read_back:
        return gfx::MemoryType::ReadBack;
    }
    KALI_THROW(Exception("Invalid MemoryType"));
}

inline gfx::MemoryRange get_gfx_memory_range(MemoryRange memory_range)
{
    return gfx::MemoryRange{
        .offset = memory_range.offset,
        .size = memory_range.size,
    };
}

Buffer::Buffer(const BufferDesc& desc, const void* init_data, ref<Device> device)
    : m_desc(desc)
    , m_device(device)
{
    KALI_ASSERT(m_desc.type == ResourceType::buffer);

    gfx::IBufferResource::Desc gfx_desc{};
    gfx_desc.type = get_gfx_resource_type(m_desc.type);
    // gfx_desc.defaultState = ResourceState::Undefined;
    // gfx_desc.allowedStates = ResourceStateSet();
    gfx_desc.memoryType = get_gfx_memory_type(m_desc.memory_type);
    // gfx_desc.existingHandle =
    // gfx_desc.isShared =
    gfx_desc.sizeInBytes = gfx::Size(m_desc.size);
    gfx_desc.elementSize = gfx::Size(m_desc.element_size);
    gfx_desc.format = get_gfx_format(m_desc.format);

    if (SLANG_FAILED(m_device->get_gfx_device()->createBufferResource(gfx_desc, init_data, m_gfx_buffer.writeRef())))
        KALI_THROW(Exception("Failed to create buffer!"));
}

void* Buffer::map(std::optional<MemoryRange> read_range)
{
    gfx::MemoryRange gfx_read_range;
    if (read_range)
        gfx_read_range = get_gfx_memory_range(*read_range);
    void* ptr;
    if (SLANG_FAILED(m_gfx_buffer->map(read_range ? &gfx_read_range : nullptr, &ptr)))
        KALI_THROW(Exception("Failed to map memory!"));
    return ptr;
}

void Buffer::unmap(std::optional<MemoryRange> write_range)
{
    gfx::MemoryRange gfx_write_range;
    if (write_range)
        gfx_write_range = get_gfx_memory_range(*write_range);
    if (SLANG_FAILED(m_gfx_buffer->unmap(write_range ? &gfx_write_range : nullptr)))
        KALI_THROW(Exception("Failed to map memory!"));
}


} // namespace kali
