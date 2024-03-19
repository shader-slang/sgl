// SPDX-License-Identifier: Apache-2.0

#include "kernel.h"

#include "sgl/device/device.h"
#include "sgl/device/shader.h"
#include "sgl/device/pipeline.h"
#include "sgl/device/command.h"
#include "sgl/device/shader_cursor.h"

#include "sgl/core/maths.h"

namespace sgl {

// ----------------------------------------------------------------------------
// Kernel
// ----------------------------------------------------------------------------

Kernel::Kernel(ref<Device> device, ref<ShaderProgram> program)
    : DeviceResource(std::move(device))
    , m_program(std::move(program))
{
}

// ----------------------------------------------------------------------------
// ComputeKernel
// ----------------------------------------------------------------------------

ComputeKernel::ComputeKernel(ref<Device> device, ComputeKernelDesc desc)
    : Kernel(std::move(device), std::move(desc.program))
{
    m_thread_group_size = m_program->layout()->get_entry_point_by_index(0)->compute_thread_group_size();
}

ComputePipeline* ComputeKernel::pipeline() const
{
    if (!m_pipeline)
        m_pipeline = m_device->create_compute_pipeline({.program = m_program});
    return m_pipeline;
}

void ComputeKernel::dispatch(uint3 thread_count, BindVarsCallback bind_vars, ComputeCommandEncoder& encoder)
{
    ref<ShaderObject> shader_object = encoder.bind_pipeline(pipeline());
    if (bind_vars)
        bind_vars(ShaderCursor(shader_object));

    uint3 thread_group_count{
        div_round_up(thread_count.x, m_thread_group_size.x),
        div_round_up(thread_count.y, m_thread_group_size.y),
        div_round_up(thread_count.z, m_thread_group_size.z)};
    encoder.dispatch_thread_groups(thread_group_count);
}

void ComputeKernel::dispatch(uint3 thread_count, BindVarsCallback bind_vars, CommandBuffer* command_buffer)
{
    ref<CommandBuffer> temp_command_buffer;
    if (command_buffer == nullptr) {
        temp_command_buffer = m_device->create_command_buffer();
        command_buffer = temp_command_buffer;
    }

    {
        auto encoder = command_buffer->encode_compute_commands();
        dispatch(thread_count, bind_vars, encoder);
    }

    if (temp_command_buffer)
        temp_command_buffer->submit();
}

// ----------------------------------------------------------------------------
// RayTracingKernel
// ----------------------------------------------------------------------------

RayTracingKernel::RayTracingKernel(ref<Device> device, ref<ShaderProgram> program)
    : Kernel(std::move(device), std::move(program))
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
//     SGL_CHECK_NOT_NULL(encoder);

//     ref<ShaderObject> shader_object = encoder->bind_pipeline(pipeline_state());
//     encoder->dispatch_rays()
// }

// void RayTracingKernel::dispatch(uint3 thread_count, BindVarsCallback bind_vars, CommandBuffer* command_buffer) { }

} // namespace sgl
