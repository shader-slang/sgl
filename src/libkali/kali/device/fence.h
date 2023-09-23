#pragma once

#include "kali/device/fwd.h"
#include "kali/device/native_handle.h"
#include "kali/device/shared_handle.h"

#include "kali/core/object.h"

#include <slang-gfx.h>

namespace kali {

struct FenceDesc {
    uint64_t initial_value{0};
    bool shared{false};
};

class Fence : public Object {
    KALI_OBJECT(Fence)
public:
    static constexpr uint64_t AUTO = uint64_t(-1);
    static constexpr uint64_t TIMEOUT_INFINITE = uint64_t(-1);

    /// Constructor.
    /// Do not use directly, instead use @see Device::create_fence.
    Fence(FenceDesc desc, ref<Device> device);

    /// Signal the fence on the device.
    /// @param queue The command queue to signal the fence on.
    /// @param value The value to signal. If @see AUTO, the signaled value will be auto-incremented.
    ///
    void signal(CommandQueue* queue, uint64_t value = AUTO);

    /// Wait for the fence to be signaled on the device.
    /// @param queue The command queue to wait on.
    /// @param value The value to wait for. If @see AUTO, wait for the last signaled value.
    ///
    void wait_device(CommandQueue* queue, uint64_t value = AUTO);

    /// Wait for the fence to be signaled on the host.
    /// @param value The value to wait for. If @see AUTO, wait for the last signaled value.
    /// @param timeout_ns The timeout in nanoseconds. If @see TIMEOUT_INFINITE, the function will block indefinitely.
    ///
    void wait_host(uint64_t value = AUTO, uint64_t timeout_ns = TIMEOUT_INFINITE);

    /// Returns the currently signaled value on the device.
    uint64_t get_current_value();

    /// Set the currently signaled value on the device.
    void set_current_value(uint64_t value);

    /// Returns the last signaled value on the device.
    uint64_t get_signaled_value();

    gfx::IFence* get_gfx_fence() const { return m_gfx_fence; }

    /// Get the shared fence handle.
    /// Throws if the fence was not created with the @see FenceDesc::shared flag.
    SharedFenceHandle get_shared_handle() const;

    /// Returns the native API handle:
    /// - D3D12: ID3D12Fence*
    /// - Vulkan: currently not supported
    NativeHandle get_native_handle() const;

    void break_strong_reference_to_device();

public:
    FenceDesc m_desc;
    breakable_ref<Device> m_device;
    Slang::ComPtr<gfx::IFence> m_gfx_fence;
    uint64_t m_signaled_value;
};

} // namespace kali
