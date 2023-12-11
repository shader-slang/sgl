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
    using BindVarsCallback = std::function<void(ShaderCursor)>;

    virtual ~Kernel() = default;

    ShaderProgram* program() const { return m_program; }
    ReflectionCursor reflection() const { return ReflectionCursor(m_program); }

protected:
    Kernel(Device* device, ref<ShaderProgram> program);

    Device* m_device;
    ref<ShaderProgram> m_program;
};

class KALI_API ComputeKernel : public Kernel {
    KALI_OBJECT(ComputeKernel)
public:
    ComputeKernel(Device* device, ref<ShaderProgram> program);
    virtual ~ComputeKernel() = default;

    ComputePipeline* pipeline() const;

    void dispatch(uint3 thread_count, BindVarsCallback bind_vars, ComputePassEncoder& compute_pass);

    void dispatch(uint3 thread_count, BindVarsCallback bind_vars, CommandStream* stream = nullptr);

private:
    uint3 m_thread_group_size;
    mutable ref<ComputePipeline> m_pipeline;
};

class KALI_API RayTracingKernel : public Kernel {
    KALI_OBJECT(RayTracingKernel)
public:
    RayTracingKernel(Device* device, ref<ShaderProgram> program);
    virtual ~RayTracingKernel() = default;

    RayTracingPipeline* pipeline() const;

    void dispatch(uint3 thread_count, BindVarsCallback bind_vars, RayTracingPassEncoder* pass_encoder);

    void dispatch(uint3 thread_count, BindVarsCallback bind_vars, CommandStream* stream = nullptr);

private:
    mutable ref<RayTracingPipeline> m_pipeline;
};

} // namespace kali
