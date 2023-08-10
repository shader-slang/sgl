#pragma once

#include "kali/rhi/fwd.h"
#include "kali/rhi/types.h"
#include "kali/rhi/native_handle.h"

#include "kali/core/macros.h"
#include "kali/core/enum.h"
#include "kali/core/object.h"

#include <slang-gfx.h>

#include <map>

namespace kali {

class KALI_API PipelineState : public Object {
    KALI_OBJECT(PipelineState)
public:
    PipelineState(ref<Device> device);

    gfx::IPipelineState* get_gfx_pipeline_state() const { return m_gfx_pipeline_state; }

    /// Returns the native API handle:
    /// - D3D12: ID3D12PipelineState*
    /// - Vulkan: VkPipeline
    NativeHandle get_native_handle() const;

protected:
    ref<Device> m_device;
    Slang::ComPtr<gfx::IPipelineState> m_gfx_pipeline_state;
};

struct ComputePipelineStateDesc {
    // TODO we should introduce a weak_ref and use that here
    ref<const ProgramVersion> program_version;
    auto operator<=>(const ComputePipelineStateDesc&) const = default;
};

class KALI_API ComputePipelineState : public PipelineState {
public:
    ComputePipelineState(ComputePipelineStateDesc desc, ref<Device> device);

    const ComputePipelineStateDesc& get_desc() const { return m_desc; }

private:
    ComputePipelineStateDesc m_desc;
};

class KALI_API ComputePipelineCache : public Object {
    KALI_OBJECT(ComputePipelineCache)
public:
    ComputePipelineCache(ref<Device> device);

    void clear();

    ref<ComputePipelineState> get_pipeline_state(ComputePipelineStateDesc desc);

private:
    ref<Device> m_device;
    std::map<ComputePipelineStateDesc, ref<ComputePipelineState>> m_pipelines;
};


struct GraphicsPipelineStateDesc {
    ref<Program> program;

    // ref<InputLayout> input_layout;
    // ref<FramebufferLayout> framebuffer_layout;
    PrimitiveType primitive_type{PrimitiveType::triangle};
    DepthStencilDesc depth_stencil;
    RasterizerDesc rasterizer;
    BlendDesc blend;
};

class KALI_API GraphicsPipelineState : public PipelineState {
public:
    GraphicsPipelineState(GraphicsPipelineStateDesc desc, ref<Device> device);

    const GraphicsPipelineStateDesc& get_desc() const { return m_desc; }

private:
    GraphicsPipelineStateDesc m_desc;
    ref<Program> m_program;
};


} // namespace kali
