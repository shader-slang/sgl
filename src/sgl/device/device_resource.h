// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"

#include "sgl/core/object.h"

namespace sgl {

class SGL_API DeviceResource : public Object {
    SGL_OBJECT(DeviceResource)
public:
    DeviceResource(ref<Device> device)
        : m_device(std::move(device))
    {
    }

    virtual ~DeviceResource() = default;

    Device* device() const { return m_device; }

    void break_strong_reference_to_device() { m_device.break_strong_reference(); }

    struct MemoryUsage {
        /// The amount of memory in bytes used on the device.
        size_t device{0};
        /// The amount of memory in bytes used on the host.
        size_t host{0};
    };

    /// The memory usage by this resource.
    virtual MemoryUsage memory_usage() const;

protected:
    breakable_ref<Device> m_device;
};

} // namespace sgl
