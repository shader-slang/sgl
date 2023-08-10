#include "pipeline.h"

#include "kali/rhi/device.h"
#include "kali/rhi/program.h"
#include "kali/rhi/helpers.h"
#include "kali/rhi/native_handle_traits.h"


namespace kali {

// ----------------------------------------------------------------------------
// PipelineState
// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------
// ComputePipelineState
// ----------------------------------------------------------------------------

ComputePipelineState::ComputePipelineState(ComputePipelineStateDesc desc, ref<Device> device)
    : PipelineState(std::move(device))
    , m_desc(std::move(desc))
{
    gfx::ComputePipelineStateDesc gfx_desc{.program = m_desc.program_version->get_gfx_shader_program()};
    SLANG_CALL(m_device->get_gfx_device()->createComputePipelineState(gfx_desc, m_gfx_pipeline_state.writeRef()));
}

// ----------------------------------------------------------------------------
// ComputePipelineCache
// ----------------------------------------------------------------------------

ComputePipelineCache::ComputePipelineCache(ref<Device> device)
    : m_device(std::move(device))
{
}

void ComputePipelineCache::clear()
{
    m_pipelines.clear();
}

ref<ComputePipelineState> ComputePipelineCache::get_pipeline_state(ComputePipelineStateDesc desc)
{
    auto it = m_pipelines.find(desc);
    if (it != m_pipelines.end())
        return it->second;

    ref<ComputePipelineState> pipeline = make_ref<ComputePipelineState>(std::move(desc), m_device);
    m_pipelines.emplace(pipeline->get_desc(), pipeline);
    return pipeline;
}

void ComputePipelineCache::break_strong_reference_to_device()
{
    m_device.break_strong_reference();
}

// ----------------------------------------------------------------------------
// GraphicsPipelineState
// ----------------------------------------------------------------------------

GraphicsPipelineState::GraphicsPipelineState(GraphicsPipelineStateDesc desc, ref<Device> device)
    : PipelineState(std::move(device))
    , m_desc(std::move(desc))
    , m_program(m_desc.program)
{
}

} // namespace kali
