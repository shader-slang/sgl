// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/device_resource.h"
#include "sgl/device/shared_handle.h"
#include "sgl/device/native_handle.h"

#include "sgl/core/macros.h"
#include "sgl/core/object.h"

#include <slang-gfx.h>

namespace sgl {

/// Fence descriptor.
struct FenceDesc {
    /// Initial fence value.
    uint64_t initial_value{0};
    /// Create a shared fence.
    bool shared{false};
};

/// Fence.
class SGL_API Fence : public DeviceResource {
    SGL_OBJECT(Fence)
public:
    static constexpr uint64_t AUTO = uint64_t(-1);
    static constexpr uint64_t TIMEOUT_INFINITE = uint64_t(-1);

    /// Constructor.
    /// Do not use directly, instead use \c Device::create_fence.
    Fence(ref<Device> device, FenceDesc desc);

    const FenceDesc& desc() const { return m_desc; }

    /**
     * Signal the fence.
     * This signals the fence from the host.
     * \param value The value to signal. If \c AUTO, the signaled value will be auto-incremented.
     * \return The signaled value.
     */
    uint64_t signal(uint64_t value = AUTO);

    /**
     * Wait for the fence to be signaled on the host.
     * Blocks the host until the fence reaches or exceeds the specified value.
     * \param value The value to wait for. If \c AUTO, wait for the last signaled value.
     * \param timeout_ns The timeout in nanoseconds. If \c TIMEOUT_INFINITE, the function will block indefinitely.
     */
    void wait(uint64_t value = AUTO, uint64_t timeout_ns = TIMEOUT_INFINITE);

    /// Returns the currently signaled value on the device.
    uint64_t current_value() const;

    /// Returns the last signaled value on the device.
    uint64_t signaled_value() const { return m_signaled_value; }

    /**
     * Updates or increments the signaled value.
     * This is used before signaling a fence (from the host, on the device or
     * from an external source), to update the internal state.
     * The passed value is stored, or if \c AUTO, the last signaled
     * value is auto-incremented by one. The returned value is what the caller
     * should signal to the fence.
     * \param value The value to signal. If \c AUTO, the signaled value will be auto-incremented.
     * \return The value to signal to the fence.
     */
    uint64_t update_signaled_value(uint64_t value = AUTO);

    gfx::IFence* gfx_fence() const { return m_gfx_fence; }

    /// Get the shared fence handle.
    /// Throws if the fence was not created with the \c FenceDesc::shared flag.
    SharedFenceHandle get_shared_handle() const;

    /// Returns the native API handle:
    /// - D3D12: ID3D12Fence*
    /// - Vulkan: currently not supported
    NativeHandle get_native_handle() const;

    std::string to_string() const override;

public:
    FenceDesc m_desc;
    Slang::ComPtr<gfx::IFence> m_gfx_fence;
    uint64_t m_signaled_value;
};

} // namespace sgl
