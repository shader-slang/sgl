// SPDX-License-Identifier: Apache-2.0

#include "sgl/device/device.h"

#include "sgl/utils/python/slangpybuffer.h"

namespace sgl {
extern void write_shader_cursor(ShaderCursor& cursor, nb::object value);
}

namespace sgl::slangpy {

NativeNDBuffer::NativeNDBuffer(ref<Device> device, NativeNDBufferDesc desc)
    : m_desc(desc)
{

    BufferDesc buffer_desc;
    buffer_desc.element_count = desc.shape.element_count();
    buffer_desc.struct_size = desc.element_stride;
    buffer_desc.usage = desc.usage;
    buffer_desc.memory_type = desc.memory_type;
    m_storage = device->create_buffer(buffer_desc);

    m_signature = fmt::format("[{},{},{}]", desc.dtype->get_type_reflection()->name(), desc.shape.size(), desc.usage);
}

} // namespace sgl::slangpy
