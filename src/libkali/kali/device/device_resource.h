#pragma once

#include "kali/device/fwd.h"

#include "kali/core/object.h"

namespace kali {

class DeviceResource : public Object {
    KALI_OBJECT(DeviceResource)
public:
    DeviceResource(ref<Device> device)
        : m_device(std::move(device))
    {
    }

    virtual ~DeviceResource() = default;

    Device* device() const { return m_device; }

    void break_strong_reference_to_device() { m_device.break_strong_reference(); }

protected:
    breakable_ref<Device> m_device;
};

} // namespace kali
