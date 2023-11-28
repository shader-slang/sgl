#pragma once

#include "kali/core/enum.h"

#include "kali/device/device_resource.h"
#include "kali/device/formats.h"

#include <slang-gfx.h>

#include <string>

namespace kali {

enum class InputSlotClass : uint8_t {
    per_vertex = static_cast<uint8_t>(gfx::InputSlotClass::PerVertex),
    per_instance = static_cast<uint8_t>(gfx::InputSlotClass::PerInstance),
};
KALI_ENUM_INFO(
    InputSlotClass,
    {
        {InputSlotClass::per_vertex, "per_vertex"},
        {InputSlotClass::per_instance, "per_instance"},
    }
);
KALI_ENUM_REGISTER(InputSlotClass);

struct InputElementDesc {
    /// The name of the corresponding parameter in shader code.
    std::string semantic_name;
    /// The index of the corresponding parameter in shader code.
    /// Only needed if multiple parameters share a semantic name.
    uint32_t semantic_index;
    /// The format of the data being fetched for this element.
    Format format;
    /// The offset in bytes of this element from the start of the corresponding chunk of vertex stream data.
    size_t offset;
    /// The index of the vertex stream to fetch this element's data from.
    uint32_t buffer_slot_index;
};

struct VertexStreamDesc {
    /// The stride in bytes for this vertex stream.
    size_t stride;
    /// Whether the stream contains per-vertex or per-instance data.
    InputSlotClass slot_class;
    /// How many instances to draw per chunk of data.
    uint32_t instance_data_step_rate;
};

struct InputLayoutDesc {
    std::vector<InputElementDesc> input_elements;
    std::vector<VertexStreamDesc> vertex_streams;
};

class InputLayout : public DeviceResource {
    KALI_OBJECT(InputLayout)
public:
    InputLayout(ref<Device> device, InputLayoutDesc desc);

    const InputLayoutDesc& desc() const { return m_desc; }

    gfx::IInputLayout* gfx_input_layout() const { return m_gfx_input_layout.get(); }

    std::string to_string() const override;

private:
    InputLayoutDesc m_desc;
    Slang::ComPtr<gfx::IInputLayout> m_gfx_input_layout;
};

} // namespace kali
