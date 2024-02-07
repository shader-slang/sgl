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
    const ShaderProgram* program;
};

class KALI_API ComputePipeline : public Pipeline {
public:
    ComputePipeline(ref<Device> device, ComputePipelineDesc desc);

    uint3 thread_group_size() const { return m_thread_group_size; }

    std::string to_string() const override;

private:
    uint3 m_thread_group_size;
};

struct GraphicsPipelineDesc {
    const ShaderProgram* program;
    const InputLayout* input_layout;
    const Framebuffer* framebuffer;
    PrimitiveType primitive_type{PrimitiveType::triangle};
    DepthStencilDesc depth_stencil;
    RasterizerDesc rasterizer;
    BlendDesc blend;
};

class KALI_API GraphicsPipeline : public Pipeline {
public:
    GraphicsPipeline(ref<Device> device, GraphicsPipelineDesc desc);

    std::string to_string() const override;

private:
};

struct HitGroupDesc {
    std::string hit_group_name;
    std::string closest_hit_entry_point;
    std::string any_hit_entry_point;
    std::string intersection_entry_point;
};

struct RayTracingPipelineDesc {
    const ShaderProgram* program;
    std::vector<HitGroupDesc> hit_groups;
    uint32_t max_recursion{0};
    uint32_t max_ray_payload_size{0};
    uint32_t max_attribute_size{8};
    RayTracingPipelineFlags flags{RayTracingPipelineFlags::none};
};

class KALI_API RayTracingPipeline : public Pipeline {
public:
    RayTracingPipeline(ref<Device> device, RayTracingPipelineDesc desc);

    std::string to_string() const override;

private:
};

} // namespace kali
