// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "kali/device/fwd.h"
#include "kali/device/device_resource.h"
#include "kali/device/types.h"
#include "kali/device/native_handle.h"

#include "kali/core/macros.h"
#include "kali/core/enum.h"
#include "kali/core/object.h"

#include "kali/math/vector_types.h"

#include <slang-gfx.h>

#include <map>

namespace kali {

/// Pipeline base class.
class KALI_API Pipeline : public DeviceResource {
    KALI_OBJECT(Pipeline)
public:
    Pipeline(ref<Device> device);

    virtual ~Pipeline();

    gfx::IPipelineState* gfx_pipeline_state() const { return m_gfx_pipeline_state; }

    /// Returns the native API handle:
    /// - D3D12: ID3D12PipelineState*
    /// - Vulkan: VkPipeline
    NativeHandle get_native_handle() const;

protected:
    Slang::ComPtr<gfx::IPipelineState> m_gfx_pipeline_state;
};

struct ComputePipelineDesc {
    ref<ShaderProgram> program;
};

/// Compute pipeline.
class KALI_API ComputePipeline : public Pipeline {
public:
    ComputePipeline(ref<Device> device, ComputePipelineDesc desc);

    /// Thread group size.
    /// Used to determine the number of thread groups to dispatch.
    uint3 thread_group_size() const { return m_thread_group_size; }

    std::string to_string() const override;

private:
    /// Shared reference to shader program to keep reflection data alive.
    ref<ShaderProgram> m_program;
    uint3 m_thread_group_size;
};

struct GraphicsPipelineDesc {
    ref<ShaderProgram> program;
    const InputLayout* input_layout;
    const Framebuffer* framebuffer;
    PrimitiveType primitive_type{PrimitiveType::triangle};
    DepthStencilDesc depth_stencil;
    RasterizerDesc rasterizer;
    BlendDesc blend;
};

/// Graphics pipeline.
class KALI_API GraphicsPipeline : public Pipeline {
public:
    GraphicsPipeline(ref<Device> device, GraphicsPipelineDesc desc);

    std::string to_string() const override;

private:
    /// Shared reference to shader program to keep reflection data alive.
    ref<ShaderProgram> m_program;
};

struct HitGroupDesc {
    std::string hit_group_name;
    std::string closest_hit_entry_point;
    std::string any_hit_entry_point;
    std::string intersection_entry_point;
};

struct RayTracingPipelineDesc {
    ref<ShaderProgram> program;
    std::vector<HitGroupDesc> hit_groups;
    uint32_t max_recursion{0};
    uint32_t max_ray_payload_size{0};
    uint32_t max_attribute_size{8};
    RayTracingPipelineFlags flags{RayTracingPipelineFlags::none};
};

/// Ray tracing pipeline.
class KALI_API RayTracingPipeline : public Pipeline {
public:
    RayTracingPipeline(ref<Device> device, RayTracingPipelineDesc desc);

    std::string to_string() const override;

private:
    /// Shared reference to shader program to keep reflection data alive.
    ref<ShaderProgram> m_program;
};

} // namespace kali
