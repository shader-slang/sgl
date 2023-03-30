#include "pipeline.h"
#include "device.h"
#include "program.h"


namespace kali {

Pipeline::Pipeline(ref<Device> device)
    : m_device(device)
{
}

ComputePipeline::ComputePipeline(const ComputePipelineDesc& desc, ref<Device> device)
    : Pipeline(device)
    , m_desc(desc)
    , m_program(desc.program)
{
    // gfx::ComputePipelineStateDesc gfx_desc{.program = m_program->get_gfx_program()};

    // m_device->get_gfx_device()->createComputePipelineState(gfx_desc, m_gfx_pipeline.writeRef());
}

} // namespace kali
