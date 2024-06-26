// SPDX-License-Identifier: Apache-2.0

#include "pipeline.h"

#include "sgl/device/device.h"
#include "sgl/device/shader.h"
#include "sgl/device/input_layout.h"
#include "sgl/device/framebuffer.h"
#include "sgl/device/helpers.h"
#include "sgl/device/native_handle_traits.h"

#include "sgl/core/config.h"
#include "sgl/core/short_vector.h"
#include "sgl/core/type_utils.h"

#include "sgl/math/vector.h"


namespace sgl {

// ----------------------------------------------------------------------------
// Global record of created pipelines used by hot-reload
// ----------------------------------------------------------------------------

std::set<Pipeline*> Pipeline::m_existing_pipelines;

// ----------------------------------------------------------------------------
// Pipeline
// ----------------------------------------------------------------------------

Pipeline::Pipeline(ref<Device> device, const ShaderProgram* program)
    : DeviceResource(std::move(device))
    , m_program(ref<const ShaderProgram>(program))
{
    SGL_CHECK_NOT_NULL(program);
    m_existing_pipelines.insert(this);
}

Pipeline::~Pipeline()
{
    m_device->deferred_release(m_gfx_pipeline_state);
    m_existing_pipelines.erase(this);
}


void Pipeline::notify_program_reloaded(const ShaderProgram* program)
{
    for (Pipeline* pipeline : m_existing_pipelines) {
        if (pipeline->m_program == program) {
            pipeline->m_device->deferred_release(pipeline->m_gfx_pipeline_state);
            pipeline->recreate();
        }
    }
}


NativeHandle Pipeline::get_native_handle() const
{
    gfx::InteropHandle handle = {};
    SLANG_CALL(m_gfx_pipeline_state->getNativeHandle(&handle));
#if SGL_HAS_D3D12
    if (m_device->type() == DeviceType::d3d12)
        return NativeHandle(reinterpret_cast<ID3D12PipelineState*>(handle.handleValue));
#endif
#if SGL_HAS_VULKAN
    if (m_device->type() == DeviceType::vulkan)
        return NativeHandle(reinterpret_cast<VkPipeline>(handle.handleValue));
#endif
    return {};
}

// ----------------------------------------------------------------------------
// ComputePipeline
// ----------------------------------------------------------------------------

ComputePipeline::ComputePipeline(ref<Device> device, ComputePipelineDesc desc)
    : Pipeline(std::move(device), desc.program.get())
    , m_desc(std::move(desc))
{
    recreate();
}

void ComputePipeline::recreate()
{
    if (m_gfx_pipeline_state) {
        m_device->deferred_release(m_gfx_pipeline_state);
        m_gfx_pipeline_state = nullptr;
    }

    gfx::ComputePipelineStateDesc gfx_desc{.program = m_program->gfx_shader_program()};
    SLANG_CALL(m_device->gfx_device()->createComputePipelineState(gfx_desc, m_gfx_pipeline_state.writeRef()));
    m_thread_group_size = m_program->layout()->get_entry_point_by_index(0)->compute_thread_group_size();
}

std::string ComputePipeline::to_string() const
{
    return fmt::format(
        "ComputePipeline(\n"
        "  device = {},\n"
        "  program = {},\n"
        "  thread_group_size = {}\n"
        ")",
        m_device,
        m_program,
        m_thread_group_size
    );
}

// ----------------------------------------------------------------------------
// GraphicsPipeline
// ----------------------------------------------------------------------------

GraphicsPipeline::GraphicsPipeline(ref<Device> device, GraphicsPipelineDesc desc)
    : Pipeline(std::move(device), desc.program.get())
    , m_desc(std::move(desc))
{
    recreate();
}

void GraphicsPipeline::recreate()
{
    if (m_gfx_pipeline_state) {
        m_device->deferred_release(m_gfx_pipeline_state);
        m_gfx_pipeline_state = nullptr;
    }

    const GraphicsPipelineDesc& desc = m_desc;

    SGL_CHECK_NOT_NULL(desc.framebuffer_layout);

    gfx::GraphicsPipelineStateDesc gfx_desc{
        .program = m_program->gfx_shader_program(),
        .inputLayout = desc.input_layout ? desc.input_layout->gfx_input_layout() : nullptr,
        .framebufferLayout = desc.framebuffer_layout->gfx_framebuffer_layout(),
        .primitiveType = static_cast<gfx::PrimitiveType>(desc.primitive_type),
        .depthStencil = {
            .depthTestEnable = desc.depth_stencil.depth_test_enable,
            .depthWriteEnable = desc.depth_stencil.depth_write_enable,
            .depthFunc = static_cast<gfx::ComparisonFunc>(desc.depth_stencil.depth_func),
            .stencilEnable = desc.depth_stencil.stencil_enable,
            .stencilReadMask = desc.depth_stencil.stencil_read_mask,
            .stencilWriteMask = desc.depth_stencil.stencil_write_mask,
            .frontFace = {
                .stencilFailOp = static_cast<gfx::StencilOp>(desc.depth_stencil.front_face.stencil_fail_op),
                .stencilDepthFailOp = static_cast<gfx::StencilOp>(desc.depth_stencil.front_face.stencil_depth_fail_op),
                .stencilPassOp = static_cast<gfx::StencilOp>(desc.depth_stencil.front_face.stencil_pass_op),
                .stencilFunc = static_cast<gfx::ComparisonFunc>(desc.depth_stencil.front_face.stencil_func),
            },
            .backFace = {
                .stencilFailOp = static_cast<gfx::StencilOp>(desc.depth_stencil.back_face.stencil_fail_op),
                .stencilDepthFailOp = static_cast<gfx::StencilOp>(desc.depth_stencil.back_face.stencil_depth_fail_op),
                .stencilPassOp = static_cast<gfx::StencilOp>(desc.depth_stencil.back_face.stencil_pass_op),
                .stencilFunc = static_cast<gfx::ComparisonFunc>(desc.depth_stencil.back_face.stencil_func),
            },
            .stencilRef = desc.depth_stencil.stencil_ref,
        },
        .rasterizer = {
            .fillMode = static_cast<gfx::FillMode>(desc.rasterizer.fill_mode),
            .cullMode = static_cast<gfx::CullMode>(desc.rasterizer.cull_mode),
            .frontFace = static_cast<gfx::FrontFaceMode>(desc.rasterizer.front_face),
            .depthBias = desc.rasterizer.depth_bias,
            .depthBiasClamp = desc.rasterizer.depth_bias_clamp,
            .slopeScaledDepthBias = desc.rasterizer.slope_scaled_depth_bias,
            .depthClipEnable = desc.rasterizer.depth_clip_enable,
            .scissorEnable = desc.rasterizer.scissor_enable,
            .multisampleEnable = desc.rasterizer.multisample_enable,
            .antialiasedLineEnable = desc.rasterizer.antialiased_line_enable,
            .enableConservativeRasterization = desc.rasterizer.enable_conservative_rasterization,
            .forcedSampleCount = desc.rasterizer.forced_sample_count,
        },
        .blend = {
            .targetCount = narrow_cast<gfx::GfxCount>(desc.blend.targets.size()),
            .alphaToCoverageEnable = desc.blend.alpha_to_coverage_enable,
        }
    };

    SGL_CHECK(desc.blend.targets.size() <= 8, "Too many blend targets");

    gfx::TargetBlendDesc* gfx_target = gfx_desc.blend.targets;
    for (const auto& target : desc.blend.targets) {
        *gfx_target++ = {
            .color{
                .srcFactor = static_cast<gfx::BlendFactor>(target.color.src_factor),
                .dstFactor = static_cast<gfx::BlendFactor>(target.color.dst_factor),
                .op = static_cast<gfx::BlendOp>(target.color.op),
            },
            .alpha{
                .srcFactor = static_cast<gfx::BlendFactor>(target.alpha.src_factor),
                .dstFactor = static_cast<gfx::BlendFactor>(target.alpha.dst_factor),
                .op = static_cast<gfx::BlendOp>(target.alpha.op),
            },
            .enableBlend = target.enable_blend,
            .logicOp = static_cast<gfx::LogicOp>(target.logic_op),
            .writeMask = static_cast<gfx::RenderTargetWriteMaskT>(target.write_mask),
        };
    }

    SLANG_CALL(m_device->gfx_device()->createGraphicsPipelineState(gfx_desc, m_gfx_pipeline_state.writeRef()));
}

std::string GraphicsPipeline::to_string() const
{
    return fmt::format(
        "GraphicsPipeline(\n"
        "  device = {}\n"
        "  program = {}\n"
        ")",
        m_device,
        m_program
    );
}

// ----------------------------------------------------------------------------
// RayTracingPipeline
// ----------------------------------------------------------------------------

RayTracingPipeline::RayTracingPipeline(ref<Device> device, RayTracingPipelineDesc desc)
    : Pipeline(std::move(device), desc.program.get())
    , m_desc(std::move(desc))
{
    recreate();
}

void RayTracingPipeline::recreate()
{
    if (m_gfx_pipeline_state) {
        m_device->deferred_release(m_gfx_pipeline_state);
        m_gfx_pipeline_state = nullptr;
    }

    const RayTracingPipelineDesc& desc = m_desc;

    short_vector<gfx::HitGroupDesc, 16> gfx_hit_groups;
    gfx_hit_groups.reserve(desc.hit_groups.size());
    for (const auto& hit_group : desc.hit_groups) {
        gfx_hit_groups.push_back({
            .hitGroupName = hit_group.hit_group_name.c_str(),
            .closestHitEntryPoint = hit_group.closest_hit_entry_point.c_str(),
            .anyHitEntryPoint = hit_group.any_hit_entry_point.c_str(),
            .intersectionEntryPoint = hit_group.intersection_entry_point.c_str(),
        });
    }

    gfx::RayTracingPipelineStateDesc gfx_desc{
        .program = m_program->gfx_shader_program(),
        .hitGroupCount = narrow_cast<gfx::GfxCount>(gfx_hit_groups.size()),
        .hitGroups = gfx_hit_groups.data(),
        .maxRecursion = narrow_cast<int>(desc.max_recursion),
        .maxRayPayloadSize = desc.max_ray_payload_size,
        .maxAttributeSizeInBytes = desc.max_attribute_size,
        .flags = static_cast<gfx::RayTracingPipelineFlags::Enum>(desc.flags),
    };
    SLANG_CALL(m_device->gfx_device()->createRayTracingPipelineState(gfx_desc, m_gfx_pipeline_state.writeRef()));
}

std::string RayTracingPipeline::to_string() const
{
    return fmt::format(
        "RayTracingPipeline(\n"
        "  device = {}\n"
        "  program = {}\n"
        ")",
        m_device,
        m_program
    );
}

} // namespace sgl
