#include "input_layout.h"

#include "kali/device/device.h"
#include "kali/device/helpers.h"

#include "kali/core/format.h"
#include "kali/core/type_utils.h"
#include "kali/core/short_vector.h"
#include "kali/core/string.h"

namespace kali {

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

std::string InputLayout::to_string() const
{
    std::vector<std::string> input_elements;
    for (const auto& input_element : m_desc.input_elements) {
        input_elements.push_back(fmt::format(
            "InputElementDesc(semantic_name=\"{}\", semantic_index={}, format={}, offset={}, buffer_slot_index={})",
            input_element.semantic_name,
            input_element.semantic_index,
            input_element.format,
            input_element.offset,
            input_element.buffer_slot_index
        ));
    }

    std::vector<std::string> vertex_streams;
    for (const auto& vertex_stream : m_desc.vertex_streams) {
        vertex_streams.push_back(fmt::format(
            "VertexStreamDesc(stride={}, slot_class={}, instance_data_step_rate={})",
            vertex_stream.stride,
            vertex_stream.slot_class,
            vertex_stream.instance_data_step_rate
        ));
    }

    return fmt::format(
        "InputLayout(\n"
        "  device={},\n"
        "  input_elements=[{}],\n"
        "  vertex_streams=[{}]\n"
        ")",
        m_device,
        string::indent(string::join(input_elements, ",\n")),
        string::indent(string::join(vertex_streams, ",\n"))
    );
}

} // namespace kali
