#include "fence.h"

#include "kali/core/macros.h"

#include "kali/rhi/device.h"
#include "kali/rhi/command_queue.h"
#include "kali/rhi/native_handle_traits.h"
#include "kali/rhi/helpers.h"

namespace kali {

Fence::Fence(FenceDesc desc, ref<Device> device)
    : m_desc(std::move(desc))
    , m_device(std::move(device))
{
    KALI_ASSERT(m_device);

    gfx::IFence::Desc gfx_desc{
        .initialValue = m_desc.initial_value,
        .isShared = m_desc.shared,
    };

    SLANG_CALL(m_device->get_gfx_device()->createFence(gfx_desc, m_gfx_fence.writeRef()));

    m_signaled_value = m_desc.initial_value;
}

void Fence::signal(CommandQueue* queue, uint64_t value)
{
    KALI_ASSERT(queue);
    uint64_t signal_value = value == AUTO ? m_signaled_value + 1 : value;
    queue->get_gfx_command_queue()->executeCommandBuffer(nullptr, m_gfx_fence, signal_value);
    m_signaled_value = signal_value;
}

void Fence::wait_device(CommandQueue* queue, uint64_t value)
{
    KALI_ASSERT(queue);
    uint64_t wait_value = value == AUTO ? m_signaled_value : value;
    gfx::IFence* fences[] = {m_gfx_fence};
    uint64_t wait_values[] = {wait_value};
    SLANG_CALL(queue->get_gfx_command_queue()->waitForFenceValuesOnDevice(1, fences, wait_values));
}

void Fence::wait_host(uint64_t value, uint64_t timeout_ns)
{
    uint64_t wait_value = value == AUTO ? m_signaled_value : value;
    uint64_t current_value = get_current_value();
    if (current_value < wait_value) {
        gfx::IFence* fences[] = {m_gfx_fence};
        uint64_t wait_values[] = {wait_value};
        SLANG_CALL(m_device->get_gfx_device()->waitForFences(1, fences, wait_values, true, timeout_ns));
    }
}

uint64_t Fence::get_current_value()
{
    uint64_t value;
    SLANG_CALL(m_gfx_fence->getCurrentValue(&value));
    return value;
}

void Fence::set_current_value(uint64_t value)
{
    SLANG_CALL(m_gfx_fence->setCurrentValue(value));
    m_signaled_value = value;
}

SharedFenceHandle Fence::get_shared_handle() const
{
    gfx::InteropHandle handle;
    SLANG_CALL(m_gfx_fence->getSharedHandle(&handle));
    return reinterpret_cast<SharedFenceHandle>(handle.handleValue);
}

NativeHandle Fence::get_native_handle() const
{
    gfx::InteropHandle handle = {};
    SLANG_CALL(m_gfx_fence->getNativeHandle(&handle));
#if KALI_HAS_D3D12
    if (m_device->get_type() == DeviceType::d3d12)
        return NativeHandle(reinterpret_cast<ID3D12Fence*>(handle.handleValue));
#endif
#if KALI_HAS_VULKAN
        // currently not supported
#endif
    return {};
}

void Fence::break_strong_reference_to_device()
{
    m_device.break_strong_reference();
}

} // namespace kali
