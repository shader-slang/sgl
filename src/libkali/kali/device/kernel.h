#pragma once

#include "kali/device/fwd.h"
#include "kali/device/shader_cursor.h"

#include "kali/core/macros.h"
#include "kali/core/object.h"

#include <functional>

namespace kali {

class KALI_API Kernel : public Object {
    KALI_OBJECT(Kernel)
public:
    virtual ~Kernel() = default;
};

class KALI_API ComputeKernel : public Kernel {
public:
    ComputeKernel(Device* device, ref<ShaderProgram> program);
    virtual ~ComputeKernel() = default;

    using BindVarsCallback = std::function<void(ShaderCursor)>;

    ComputePipelineState* pipeline_state() const;

    void dispatch(uint3 thread_count, BindVarsCallback bind_vars, CommandStream* stream = nullptr);

private:
    Device* m_device;
    ref<ShaderProgram> m_program;
    uint3 m_thread_group_size;
    mutable ref<ComputePipelineState> m_pipeline_state;
};

} // namespace kali
