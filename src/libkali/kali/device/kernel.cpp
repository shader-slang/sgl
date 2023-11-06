#include "kernel.h"

#include "kali/device/device.h"
#include "kali/device/shader.h"
#include "kali/device/pipeline.h"
#include "kali/device/command.h"
#include "kali/device/shader_cursor.h"

#include "kali/core/maths.h"

namespace kali {

ComputeKernel::ComputeKernel(Device* device, ref<ShaderProgram> program)
    : m_device(device)
    , m_program(std::move(program))
{
    m_thread_group_size = m_program->entry_point_layout(0)->compute_thread_group_size();
}

ComputePipelineState* ComputeKernel::pipeline_state() const
{
    if (!m_pipeline_state)
        m_pipeline_state = m_device->create_compute_pipeline_state({.program = m_program});
    return m_pipeline_state;
}

void ComputeKernel::dispatch(uint3 thread_count, SetShaderVariablesCallback set_vars, CommandStream* stream)
{
    if (stream == nullptr)
        stream = m_program->device()->command_stream();

    ref<ShaderObject> shader_object = stream->bind_compute_pipeline(pipeline_state());
    if (set_vars)
        set_vars(ShaderCursor(shader_object));

    uint3 thread_group_count{
        div_round_up(thread_count.x, m_thread_group_size.x),
        div_round_up(thread_count.y, m_thread_group_size.y),
        div_round_up(thread_count.z, m_thread_group_size.z)};
    stream->dispatch_compute(thread_group_count);
}

} // namespace kali
