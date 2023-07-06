#pragma once

#include "kali/rhi/fwd.h"
#include "kali/rhi/types.h"

#include "kali/macros.h"
#include "kali/enum.h"
#include "kali/object.h"

#include <slang-gfx.h>

namespace kali {

class KALI_API PipelineState : public Object {
    KALI_OBJECT(PipelineState)
public:
    PipelineState(ref<Device> device);

    gfx::IPipelineState* get_gfx_pipeline_state() const { return m_gfx_pipeline_state; }

protected:
    ref<Device> m_device;
    Slang::ComPtr<gfx::IPipelineState> m_gfx_pipeline_state;
};

struct ComputePipelineStateDesc {
    ref<Program> program;
};

class KALI_API ComputePipelineState : public PipelineState {
public:
    ComputePipelineState(const ComputePipelineStateDesc& desc, ref<Device> device);

    const ComputePipelineStateDesc& get_desc() const { return m_desc; }

private:
    ComputePipelineStateDesc m_desc;
    ref<Program> m_program;
};

struct GraphicsPipelineStateDesc {
    ref<Program> program;

    // ref<InputLayout> input_layout;
    // ref<FramebufferLayout> framebuffer_layout;
    PrimitiveType primitive_type {PrimitiveType::triangle};
    DepthStencilDesc depth_stencil;
    RasterizerDesc rasterizer;
    BlendDesc blend;
};

class KALI_API GraphicsPipelineState : public PipelineState {
public:
    GraphicsPipelineState(const GraphicsPipelineStateDesc& desc, ref<Device> device);

    const GraphicsPipelineStateDesc& get_desc() const { return m_desc; }

private:
    GraphicsPipelineStateDesc m_desc;
    ref<Program> m_program;
};


} // namespace kali
