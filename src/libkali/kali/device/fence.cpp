#include "fence.h"

#include "kali/core/macros.h"
#include "kali/core/string.h"

#include "kali/device/device.h"
#include "kali/device/command_queue.h"
#include "kali/device/native_handle_traits.h"
#include "kali/device/helpers.h"

namespace kali {

Fence::Fence(ref<Device> device, FenceDesc desc)
    : m_device(std::move(device))
    , m_desc(std::move(desc))
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
    uint64_t cur_value = current_value();
    if (cur_value < wait_value) {
        gfx::IFence* fences[] = {m_gfx_fence};
        uint64_t wait_values[] = {wait_value};
        SLANG_CALL(m_device->get_gfx_device()->waitForFences(1, fences, wait_values, true, timeout_ns));
    }
}

uint64_t Fence::current_value() const
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
    KALI_CHECK(m_desc.shared, "Fence must be created with shared flag.");
    gfx::InteropHandle handle;
    SLANG_CALL(m_gfx_fence->getSharedHandle(&handle));
    return reinterpret_cast<SharedFenceHandle>(handle.handleValue);
}

NativeHandle Fence::get_native_handle() const
{
    gfx::InteropHandle handle = {};
    SLANG_CALL(m_gfx_fence->getNativeHandle(&handle));
#if KALI_HAS_D3D12
    if (m_device->type() == DeviceType::d3d12)
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

std::string Fence::to_string() const
{
    return fmt::format(
        "Fence(\n"
        "   device={},\n"
        "   desc={},\n"
        "   current_value={},\n"
        "   signaled_value={}\n"
        ")",
        m_device,
        string::indent(m_desc.to_string()),
        current_value(),
        m_signaled_value
    );
}

} // namespace kali
