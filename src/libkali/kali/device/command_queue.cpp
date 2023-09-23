#include "command_queue.h"

#include "kali/device/device.h"
#include "kali/device/native_handle_traits.h"
#include "kali/device/helpers.h"

#include "kali/core/error.h"

namespace kali {

inline gfx::ICommandQueue::QueueType get_gfx_command_queue_type(CommandQueueType type)
{
    static_assert(uint32_t(CommandQueueType::graphics) == uint32_t(gfx::ICommandQueue::QueueType::Graphics));
    KALI_ASSERT(uint32_t(type) <= uint32_t(CommandQueueType::graphics));
    return gfx::ICommandQueue::QueueType(type);
}

CommandQueue::CommandQueue(CommandQueueDesc desc, ref<Device> device)
    : m_desc(std::move(desc))
    , m_device(std::move(device))
{
    gfx::ICommandQueue::Desc gfx_desc{
        .type = get_gfx_command_queue_type(m_desc.type),
    };

    SLANG_CALL(m_device->get_gfx_device()->createCommandQueue(gfx_desc, m_gfx_command_queue.writeRef()));
}

NativeHandle CommandQueue::get_native_handle() const
{
    gfx::InteropHandle handle = {};
    SLANG_CALL(m_gfx_command_queue->getNativeHandle(&handle));
#if KALI_HAS_D3D12
    if (m_device->get_type() == DeviceType::d3d12)
        return NativeHandle(reinterpret_cast<ID3D12CommandQueue*>(handle.handleValue));
#endif
#if KALI_HAS_VULKAN
    if (m_device->get_type() == DeviceType::vulkan)
        return NativeHandle(reinterpret_cast<VkQueue>(handle.handleValue));
#endif
    return {};
}

void CommandQueue::break_strong_reference_to_device()
{
    m_device.break_strong_reference();
}

} // namespace kali
