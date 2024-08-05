// SPDX-License-Identifier: Apache-2.0

#include "input_layout.h"

#include "sgl/device/device.h"
#include "sgl/device/helpers.h"

#include "sgl/core/format.h"
#include "sgl/core/type_utils.h"
#include "sgl/core/short_vector.h"
#include "sgl/core/string.h"

namespace sgl {

InputLayout::InputLayout(ref<Device> device, InputLayoutDesc desc)
    : DeviceResource(std::move(device))
    , m_desc(std::move(desc))
{
    short_vector<gfx::InputElementDesc, 16> gfx_input_elements;
    for (const auto& input_element : m_desc.input_elements) {
        gfx_input_elements.push_back({
            .semanticName = input_element.semantic_name.c_str(),
            .semanticIndex = narrow_cast<gfx::GfxIndex>(input_element.semantic_index),
            .format = static_cast<gfx::Format>(input_element.format),
            .offset = input_element.offset,
            .bufferSlotIndex = narrow_cast<gfx::GfxIndex>(input_element.buffer_slot_index),
        });
    }

    short_vector<gfx::VertexStreamDesc, 16> gfx_vertex_streams;
    for (const auto& vertex_stream : m_desc.vertex_streams) {
        gfx_vertex_streams.push_back({
            .stride = vertex_stream.stride,
            .slotClass = static_cast<gfx::InputSlotClass>(vertex_stream.slot_class),
            .instanceDataStepRate = narrow_cast<gfx::GfxCount>(vertex_stream.instance_data_step_rate),
        });
    }

    gfx::IInputLayout::Desc gfx_desc{
        .inputElements = gfx_input_elements.data(),
        .inputElementCount = narrow_cast<gfx::GfxCount>(gfx_input_elements.size()),
        .vertexStreams = gfx_vertex_streams.data(),
        .vertexStreamCount = narrow_cast<gfx::GfxCount>(gfx_vertex_streams.size()),
    };
    SLANG_CALL(m_device->gfx_device()->createInputLayout(gfx_desc, m_gfx_input_layout.writeRef()));
}

inline std::string to_string(const InputElementDesc& desc)
{
    return fmt::format(
        "(semantic_name=\"{}\", semantic_index={}, format={}, offset={}, buffer_slot_index={})",
        desc.semantic_name,
        desc.semantic_index,
        desc.format,
        desc.offset,
        desc.buffer_slot_index
    );
}

inline std::string to_string(const VertexStreamDesc& desc)
{
    return fmt::format(
        "(stride={}, slot_class={}, instance_data_step_rate={})",
        desc.stride,
        desc.slot_class,
        desc.instance_data_step_rate
    );
}

std::string InputLayout::to_string() const
{
    return fmt::format(
        "InputLayout(\n"
        "  device = {},\n"
        "  input_elements = {},\n"
        "  vertex_streams = {}\n"
        ")",
        m_device,
        string::indent(string::list_to_string(m_desc.input_elements)),
        string::indent(string::list_to_string(m_desc.vertex_streams))
    );
}

} // namespace sgl
