#include "raytracing.h"

#include "kali/device/device.h"
#include "kali/device/helpers.h"

#include "kali/core/type_utils.h"

namespace kali {

AccelerationStructure::AccelerationStructure(ref<Device> device, Desc desc)
    : DeviceResource(std::move(device))
    , m_desc(std::move(desc))
{
    gfx::IAccelerationStructure::CreateDesc gfx_desc{
        .kind = static_cast<gfx::IAccelerationStructure::Kind>(m_desc.kind),
        .buffer = m_desc.buffer->get_gfx_buffer_resource(),
        .offset = m_desc.offset,
        .size = m_desc.size,
    };
    SLANG_CALL(
        m_device->get_gfx_device()->createAccelerationStructure(gfx_desc, m_gfx_acceleration_structure.writeRef())
    );
}

DeviceAddress AccelerationStructure::device_address() const
{
    return m_gfx_acceleration_structure->getDeviceAddress();
}

std::string AccelerationStructure::to_string() const
{
    return fmt::format(
        "AccelerationStructure(\n"
        "  device={}\n"
        "  kind={}\n"
        "  buffer={}\n"
        "  offset={}\n"
        "  size={}\n"
        ")",
        m_device,
        m_desc.kind,
        m_desc.buffer,
        m_desc.offset,
        m_desc.size
    );
}

} // namespace kali
