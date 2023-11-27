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

// ----------------------------------------------------------------------------
// RayTracingPipelineState
// ----------------------------------------------------------------------------

RayTracingPipelineState::RayTracingPipelineState(ref<Device> device, RayTracingPipelineStateDesc desc)
    : PipelineState(std::move(device))
    , m_desc(std::move(desc))
{
    gfx::RayTracingPipelineStateDesc gfx_desc{
        .program = m_desc.program->get_gfx_shader_program(),
        // .hitGroupCount = m_desc.hit_groups.size(),
        // .hitGroups = m_desc.hit_groups.data(),
        .maxRecursion = narrow_cast<int>(m_desc.max_recursion),
        .maxRayPayloadSize = m_desc.max_ray_payload_size,
        .maxAttributeSizeInBytes = m_desc.max_attribute_size,
        .flags = static_cast<gfx::RayTracingPipelineFlags::Enum>(m_desc.flags),
    };
    SLANG_CALL(m_device->get_gfx_device()->createRayTracingPipelineState(gfx_desc, m_gfx_pipeline_state.writeRef()));
}


} // namespace kali
