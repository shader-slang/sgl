#include "kernel.h"

#include "kali/device/device.h"
#include "kali/device/shader.h"
#include "kali/device/pipeline.h"
#include "kali/device/command.h"
#include "kali/device/shader_cursor.h"

#include "kali/core/maths.h"

namespace kali {

// ----------------------------------------------------------------------------
// Kernel
// ----------------------------------------------------------------------------

Kernel::Kernel(Device* device, ref<ShaderProgram> program)
    : m_device(device)
    , m_program(std::move(program))
{
}

// ----------------------------------------------------------------------------
// ComputeKernel
// ----------------------------------------------------------------------------

ComputeKernel::ComputeKernel(Device* device, ref<ShaderProgram> program)
    : Kernel(device, program)
{
    m_thread_group_size = m_program->entry_point_layout(0)->compute_thread_group_size();
}

ComputePipeline* ComputeKernel::pipeline() const
{
    if (!m_pipeline)
        m_pipeline = m_device->create_compute_pipeline({.program = m_program});
    return m_pipeline;
}

void ComputeKernel::dispatch(uint3 thread_count, BindVarsCallback bind_vars, ComputePassEncoder& compute_pass)
{
    ref<ShaderObject> shader_object = compute_pass.bind_pipeline(pipeline());
    if (bind_vars)
        bind_vars(ShaderCursor(shader_object));

    uint3 thread_group_count{
        div_round_up(thread_count.x, m_thread_group_size.x),
        div_round_up(thread_count.y, m_thread_group_size.y),
        div_round_up(thread_count.z, m_thread_group_size.z)};
    compute_pass.dispatch_thread_groups(thread_group_count);
}

void ComputeKernel::dispatch(uint3 thread_count, BindVarsCallback bind_vars, CommandStream* stream)
{
    if (stream == nullptr)
        stream = m_device->command_stream();

    auto compute_pass = stream->begin_compute_pass();
    dispatch(thread_count, bind_vars, compute_pass);
}

// ----------------------------------------------------------------------------
// RayTracingKernel
// ----------------------------------------------------------------------------

RayTracingKernel::RayTracingKernel(Device* device, ref<ShaderProgram> program)
    : Kernel(device, program)
{
}

RayTracingPipeline* RayTracingKernel::pipeline() const
{
    if (!m_pipeline)
        m_pipeline = m_device->create_ray_tracing_pipeline({.program = m_program});
    return m_pipeline;
}

// void RayTracingKernel::dispatch(uint3 thread_count, BindVarsCallback bind_vars, RayTracingCommandEncoder* encoder)
// {
//     KALI_CHECK_NOT_NULL(encoder);

//     ref<ShaderObject> shader_object = encoder->bind_pipeline(pipeline_state());
//     encoder->dispatch_rays()
// }

// void RayTracingKernel::dispatch(uint3 thread_count, BindVarsCallback bind_vars, CommandStream* stream) { }

} // namespace kali
