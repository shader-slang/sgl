// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "kali/device/fwd.h"
#include "kali/device/device_resource.h"
#include "kali/device/shader_cursor.h"

#include "kali/core/macros.h"
#include "kali/core/object.h"

#include <functional>

namespace kali {

class KALI_API Kernel : public DeviceResource {
    KALI_OBJECT(Kernel)
public:
    using BindVarsCallback = std::function<void(ShaderCursor)>;

    virtual ~Kernel() = default;

    ShaderProgram* program() const { return m_program; }
    ReflectionCursor reflection() const { return ReflectionCursor(m_program); }

protected:
    Kernel(ref<Device> device, ref<ShaderProgram> program);

    ref<ShaderProgram> m_program;
};

struct ComputeKernelDesc {
    ref<ShaderProgram> program;
};

class KALI_API ComputeKernel : public Kernel {
    KALI_OBJECT(ComputeKernel)
public:
    ComputeKernel(ref<Device> device, ComputeKernelDesc desc);
    ~ComputeKernel() = default;

    ComputePipeline* pipeline() const;

    void dispatch(uint3 thread_count, BindVarsCallback bind_vars, ComputeCommandEncoder& encoder);

    void dispatch(uint3 thread_count, BindVarsCallback bind_vars, CommandBuffer* command_buffer = nullptr);

private:
    uint3 m_thread_group_size;
    mutable ref<ComputePipeline> m_pipeline;
};

class KALI_API RayTracingKernel : public Kernel {
    KALI_OBJECT(RayTracingKernel)
public:
    RayTracingKernel(ref<Device> device, ref<ShaderProgram> program);
    ~RayTracingKernel() = default;

    RayTracingPipeline* pipeline() const;

    void dispatch(uint3 thread_count, BindVarsCallback bind_vars, RayTracingCommandEncoder& encoder);

    void dispatch(uint3 thread_count, BindVarsCallback bind_vars, CommandBuffer* command_buffer = nullptr);

private:
    mutable ref<RayTracingPipeline> m_pipeline;
};

} // namespace kali
