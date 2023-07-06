#include "pipeline.h"

#include "kali/rhi/device.h"
#include "kali/rhi/program.h"


namespace kali {

PipelineState::PipelineState(ref<Device> device)
    : m_device(std::move(device))
{
}

ComputePipelineState::ComputePipelineState(const ComputePipelineStateDesc& desc, ref<Device> device)
    : PipelineState(std::move(device))
    , m_desc(desc)
    , m_program(desc.program)
{
    // gfx::ComputePipelineStateDesc gfx_desc{.program = m_program->get_gfx_program()};
    // gfx::ComputePipelineStateDesc gfx_desc{.program = m_program->get_gfx_program()};

    // m_device->get_gfx_device()->createComputePipelineState(gfx_desc, m_gfx_pipeline.writeRef());
}

} // namespace kali
