#include "device.h"
#include "swapchain.h"
#include "resource.h"
#include "program.h"
#include "helpers.h"

#include "core/error.h"
#include "core/window.h"

namespace kali {

inline gfx::DeviceType get_gfx_device_type(DeviceType device_type)
{
    switch (device_type) {
    case DeviceType::automatic:
        return gfx::DeviceType::Default;
    case DeviceType::d3d12:
        return gfx::DeviceType::DirectX12;
    case DeviceType::vulkan:
        return gfx::DeviceType::Vulkan;
    case DeviceType::cpu:
        return gfx::DeviceType::CPU;
    case DeviceType::cuda:
        return gfx::DeviceType::CUDA;
    }
    KALI_THROW(Exception("Invalid DeviceType"));
}

Device::Device(const DeviceDesc& desc)
{
    gfx::IDevice::Desc gfx_desc{
        .deviceType = get_gfx_device_type(desc.type),
    };
    if (SLANG_FAILED(gfx::gfxCreateDevice(&gfx_desc, m_gfx_device.writeRef())))
        KALI_THROW(Exception("Failed to create device!"));

    gfx::ICommandQueue::Desc queue_desc{
        .type = gfx::ICommandQueue::QueueType::Graphics,
    };

    SLANG_CALL(m_gfx_device->createCommandQueue(queue_desc, m_gfx_queue.writeRef()));
    SLANG_CALL(slang::createGlobalSession(m_slang_session.writeRef()));

    m_program_manager = new ProgramManager(this, m_slang_session);
}

Device::~Device()
{
    m_gfx_queue.setNull();
    m_gfx_device.setNull();
}

ref<Swapchain> Device::create_swapchain(const SwapchainDesc& desc, ref<Window> window)
{
    return new Swapchain(desc, window, this);
}

ref<Buffer> Device::create_buffer(const BufferDesc& desc, const void* init_data)
{
    return new Buffer(desc, init_data, this);
}

ref<Buffer> Device::create_raw_buffer(size_t size, ResourceUsage usage, CpuAccess cpu_access, const void* init_data)
{
    BufferDesc desc{
        .size = size,
        .usage = usage,
        .cpu_access = cpu_access,
    };
    return create_buffer(desc, init_data);
}

ref<Buffer> Device::create_typed_buffer(
    Format format,
    size_t element_count,
    ResourceUsage usage,
    CpuAccess cpu_access,
    const void* init_data
)
{
    BufferDesc desc{
        .size = element_count * 1,
        .format = format,
        .usage = usage,
        .cpu_access = cpu_access,
    };
    return create_buffer(desc, init_data);
}

ref<Buffer> Device::create_structured_buffer(
    size_t struct_size,
    size_t element_count,
    ResourceUsage usage,
    CpuAccess cpu_access,
    const void* init_data
)
{
    BufferDesc desc{
        .size = element_count * struct_size,
        .struct_size = struct_size,
        .usage = usage,
        .cpu_access = cpu_access,
    };
    return create_buffer(desc, init_data);
}

ref<Program> Device::create_program(const ProgramDesc& desc)
{
    return m_program_manager->create_program(desc);
}

ref<Program> Device::create_program(std::filesystem::path path, std::string entrypoint)
{
    return create_program(ProgramDesc::create().add_file(path).add_entrypoint(entrypoint));
}

ProgramManager& Device::get_program_manager()
{
    return *m_program_manager;
}

} // namespace kali
