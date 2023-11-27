#include "fence.h"

#include "kali/device/device.h"
#include "kali/device/native_handle_traits.h"
#include "kali/device/helpers.h"

#include "kali/core/config.h"
#include "kali/core/macros.h"
#include "kali/core/string.h"

namespace kali {

Fence::Fence(ref<Device> device, FenceDesc desc)
    : DeviceResource(std::move(device))
    , m_desc(std::move(desc))
{
    KALI_ASSERT(m_device);

    gfx::IFence::Desc gfx_desc{
        .initialValue = m_desc.initial_value,
        .isShared = m_desc.shared,
    };

    SLANG_CALL(m_device->gfx_device()->createFence(gfx_desc, m_gfx_fence.writeRef()));

    m_signaled_value = m_desc.initial_value;
}

uint64_t Fence::signal(uint64_t value)
{
    uint64_t signal_value = update_signaled_value(value);
    SLANG_CALL(m_gfx_fence->setCurrentValue(signal_value));
    return signal_value;
}

void Fence::wait(uint64_t value, uint64_t timeout_ns)
{
    uint64_t wait_value = value == AUTO ? m_signaled_value : value;
    uint64_t cur_value = current_value();
    if (cur_value < wait_value) {
        gfx::IFence* fences[] = {m_gfx_fence};
        uint64_t wait_values[] = {wait_value};
        SLANG_CALL(m_device->gfx_device()->waitForFences(1, fences, wait_values, true, timeout_ns));
    }
}

uint64_t Fence::current_value() const
{
    uint64_t value;
    SLANG_CALL(m_gfx_fence->getCurrentValue(&value));
    return value;
}

uint64_t Fence::update_signaled_value(uint64_t value)
{
    m_signaled_value = value == AUTO ? m_signaled_value + 1 : value;
    return m_signaled_value;
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

std::string Fence::to_string() const
{
    return fmt::format(
        "Fence(\n"
        "  device={},\n"
        "  initial_value={}\n"
        "  shared={}\n"
        "  current_value={},\n"
        "  signaled_value={}\n"
        ")",
        m_device,
        m_desc.initial_value,
        m_desc.shared,
        current_value(),
        m_signaled_value
    );
}

} // namespace kali
