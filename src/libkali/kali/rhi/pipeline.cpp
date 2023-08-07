#include "pipeline.h"

#include "kali/rhi/device.h"
#include "kali/rhi/program.h"
#include "kali/rhi/helpers.h"
#include "kali/rhi/native_handle_traits.h"


namespace kali {

PipelineState::PipelineState(ref<Device> device)
    : m_device(std::move(device))
{
}

NativeHandle PipelineState::get_native_handle() const
{
    gfx::InteropHandle handle = {};
    SLANG_CALL(m_gfx_pipeline_state->getNativeHandle(&handle));
#if KALI_HAS_D3D12
    if (m_device->get_type() == DeviceType::d3d12)
        return NativeHandle(reinterpret_cast<ID3D12PipelineState*>(handle.handleValue));
#endif
#if KALI_HAS_VULKAN
    if (m_device->get_type() == DeviceType::vulkan)
        return NativeHandle(reinterpret_cast<VkPipeline>(handle.handleValue));
#endif
    return {};
}

ComputePipelineState::ComputePipelineState(const ComputePipelineStateDesc& desc, ref<Device> device)
    : PipelineState(std::move(device))
    , m_desc(desc)
    , m_program(desc.program)
{
    // gfx::ComputePipelineStateDesc gfx_desc{.program = m_program->get_gfx_program()};
    // gfx::ComputePipelineStateDesc gfx_desc{.program = m_program->get_gfx_program()};

    gfx::ComputePipelineStateDesc gfx_desc;

    // m_device->get_gfx_device()->createComputePipelineState(gfx_desc, m_gfx_pipeline.writeRef());
}

GraphicsPipelineState::GraphicsPipelineState(const GraphicsPipelineStateDesc& desc, ref<Device> device)
    : PipelineState(std::move(device))
    , m_desc(desc)
    , m_program(desc.program)
{
}

} // namespace kali
