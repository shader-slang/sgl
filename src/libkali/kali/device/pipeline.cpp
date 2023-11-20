#include "pipeline.h"

#include "kali/device/device.h"
#include "kali/device/shader.h"
#include "kali/device/helpers.h"
#include "kali/device/native_handle_traits.h"

#include "kali/core/config.h"


namespace kali {

// ----------------------------------------------------------------------------
// PipelineState
// ----------------------------------------------------------------------------

PipelineState::PipelineState(ref<Device> device)
    : DeviceResource(std::move(device))
{
}

NativeHandle PipelineState::get_native_handle() const
{
    gfx::InteropHandle handle = {};
    SLANG_CALL(m_gfx_pipeline_state->getNativeHandle(&handle));
#if KALI_HAS_D3D12
    if (m_device->type() == DeviceType::d3d12)
        return NativeHandle(reinterpret_cast<ID3D12PipelineState*>(handle.handleValue));
#endif
#if KALI_HAS_VULKAN
    if (m_device->type() == DeviceType::vulkan)
        return NativeHandle(reinterpret_cast<VkPipeline>(handle.handleValue));
#endif
    return {};
}

// ----------------------------------------------------------------------------
// ComputePipelineState
// ----------------------------------------------------------------------------

ComputePipelineState::ComputePipelineState(ref<Device> device, ComputePipelineStateDesc desc)
    : PipelineState(std::move(device))
    , m_desc(std::move(desc))
{
    gfx::ComputePipelineStateDesc gfx_desc{.program = m_desc.program->get_gfx_shader_program()};
    SLANG_CALL(m_device->get_gfx_device()->createComputePipelineState(gfx_desc, m_gfx_pipeline_state.writeRef()));
}

// ----------------------------------------------------------------------------
// GraphicsPipelineState
// ----------------------------------------------------------------------------

GraphicsPipelineState::GraphicsPipelineState(ref<Device> device, GraphicsPipelineStateDesc desc)
    : PipelineState(std::move(device))
    , m_desc(std::move(desc))
{
}

} // namespace kali
