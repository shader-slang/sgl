#pragma once

#include "kali/device/fwd.h"
#include "kali/device/device_resource.h"
#include "kali/device/types.h"
#include "kali/device/native_handle.h"

#include "kali/core/macros.h"
#include "kali/core/enum.h"
#include "kali/core/object.h"

#include <slang-gfx.h>

#include <map>

namespace kali {

class KALI_API PipelineState : public DeviceResource {
    KALI_OBJECT(PipelineState)
public:
    PipelineState(ref<Device> device);

    gfx::IPipelineState* gfx_pipeline_state() const { return m_gfx_pipeline_state; }

    /// Returns the native API handle:
    /// - D3D12: ID3D12PipelineState*
    /// - Vulkan: VkPipeline
    NativeHandle get_native_handle() const;

protected:
    Slang::ComPtr<gfx::IPipelineState> m_gfx_pipeline_state;
};

struct ComputePipelineStateDesc {
    // TODO we should introduce a weak_ref and use that here
    // This would allow the cache to check if the program is still available
    const ShaderProgram* program;
    auto operator<=>(const ComputePipelineStateDesc&) const = default;
};

class KALI_API ComputePipelineState : public PipelineState {
public:
    ComputePipelineState(ref<Device> device, ComputePipelineStateDesc desc);

    const ComputePipelineStateDesc& desc() const { return m_desc; }

private:
    ComputePipelineStateDesc m_desc;
};

struct GraphicsPipelineStateDesc {
    ShaderProgram* program;

    ref<InputLayout> input_layout;
    // ref<FramebufferLayout> framebuffer_layout;
    PrimitiveType primitive_type{PrimitiveType::triangle};
    DepthStencilDesc depth_stencil;
    RasterizerDesc rasterizer;
    BlendDesc blend;
};

class KALI_API GraphicsPipelineState : public PipelineState {
public:
    GraphicsPipelineState(ref<Device> device, GraphicsPipelineStateDesc desc);

    const GraphicsPipelineStateDesc& desc() const { return m_desc; }

private:
    GraphicsPipelineStateDesc m_desc;
};

struct HitGroupDesc {
    std::string hit_group_name;
    std::string closest_hit_entry_point;
    std::string any_hit_entry_point;
    std::string intersection_entry_point;
};

struct RayTracingPipelineStateDesc {
    ShaderProgram* program;
    std::vector<HitGroupDesc> hit_groups;
    uint32_t max_recursion{0};
    uint32_t max_ray_payload_size{0};
    uint32_t max_attribute_size{8};
    RayTracingPipelineFlags flags{RayTracingPipelineFlags::none};
};

class RayTracingPipelineState : public PipelineState {
public:
    RayTracingPipelineState(ref<Device> device, RayTracingPipelineStateDesc desc);

    const RayTracingPipelineStateDesc& desc() const { return m_desc; }

private:
    RayTracingPipelineStateDesc m_desc;
};

} // namespace kali
