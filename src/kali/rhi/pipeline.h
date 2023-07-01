#pragma once

#include "fwd.h"

#include "core/macros.h"
#include "core/object.h"

#include <slang-gfx.h>

namespace kali {

class KALI_API Pipeline : public Object {
    KALI_OBJECT(Pipeline)
public:
    Pipeline(ref<Device> device);

    gfx::IPipelineState* get_gfx_pipeline() const { return m_gfx_pipeline; }

protected:
    ref<Device> m_device;
    Slang::ComPtr<gfx::IPipelineState> m_gfx_pipeline;
};

struct ComputePipelineDesc {
    Program* program;
};

class KALI_API ComputePipeline : public Pipeline {
public:
    ComputePipeline(const ComputePipelineDesc& desc, ref<Device> device);

    const ComputePipelineDesc& get_desc() const { return m_desc; }

private:
    ComputePipelineDesc m_desc;
    ref<Program> m_program;
};

} // namespace kali
