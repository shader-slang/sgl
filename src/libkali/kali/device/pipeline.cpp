#include "pipeline.h"

#include "kali/device/device.h"
#include "kali/device/shader.h"
#include "kali/device/helpers.h"
#include "kali/device/native_handle_traits.h"

#include "kali/core/config.h"
#include "kali/core/short_vector.h"
#include "kali/core/type_utils.h"

#include "kali/math/vector.h"


namespace kali {

// ----------------------------------------------------------------------------
// Pipeline
// ----------------------------------------------------------------------------

Pipeline::Pipeline(ref<Device> device)
    : DeviceResource(std::move(device))
{
}

NativeHandle Pipeline::get_native_handle() const
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
// ComputePipeline
// ----------------------------------------------------------------------------

ComputePipeline::ComputePipeline(ref<Device> device, ComputePipelineDesc desc)
    : Pipeline(std::move(device))
    , m_desc(std::move(desc))
{
    KALI_CHECK_NOT_NULL(m_desc.program);

    gfx::ComputePipelineStateDesc gfx_desc{.program = m_desc.program->gfx_shader_program()};
    SLANG_CALL(m_device->gfx_device()->createComputePipelineState(gfx_desc, m_gfx_pipeline_state.writeRef()));
    m_thread_group_size = desc.program->entry_point_layout(0)->compute_thread_group_size();
}

std::string ComputePipeline::to_string() const
{
    return fmt::format(
        "ComputePipeline(\n"
        "  device={},\n"
        "  thread_group_size={}\n"
        ")",
        m_device,
        m_thread_group_size
    );
}

// ----------------------------------------------------------------------------
// GraphicsPipeline
// ----------------------------------------------------------------------------

GraphicsPipeline::GraphicsPipeline(ref<Device> device, GraphicsPipelineDesc desc)
    : Pipeline(std::move(device))
    , m_desc(std::move(desc))
{
}

// ----------------------------------------------------------------------------
// RayTracingPipeline
// ----------------------------------------------------------------------------

RayTracingPipeline::RayTracingPipeline(ref<Device> device, RayTracingPipelineDesc desc)
    : Pipeline(std::move(device))
    , m_desc(std::move(desc))
{
    ShortVector<gfx::HitGroupDesc, 16> gfx_hit_groups;
    gfx_hit_groups.reserve(m_desc.hit_groups.size());
    for (const auto& hit_group : m_desc.hit_groups) {
        gfx_hit_groups.push_back({
            .hitGroupName = hit_group.hit_group_name.c_str(),
            .closestHitEntryPoint = hit_group.closest_hit_entry_point.c_str(),
            .anyHitEntryPoint = hit_group.any_hit_entry_point.c_str(),
            .intersectionEntryPoint = hit_group.intersection_entry_point.c_str(),
        });
    }

    gfx::RayTracingPipelineStateDesc gfx_desc{
        .program = m_desc.program->gfx_shader_program(),
        .hitGroupCount = narrow_cast<gfx::GfxCount>(gfx_hit_groups.size()),
        .hitGroups = gfx_hit_groups.data(),
        .maxRecursion = narrow_cast<int>(m_desc.max_recursion),
        .maxRayPayloadSize = m_desc.max_ray_payload_size,
        .maxAttributeSizeInBytes = m_desc.max_attribute_size,
        .flags = static_cast<gfx::RayTracingPipelineFlags::Enum>(m_desc.flags),
    };
    SLANG_CALL(m_device->gfx_device()->createRayTracingPipelineState(gfx_desc, m_gfx_pipeline_state.writeRef()));
}

} // namespace kali
