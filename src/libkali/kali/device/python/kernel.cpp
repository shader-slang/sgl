// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "kali/device/kernel.h"
#include "kali/device/command.h"
#include "kali/device/resource.h"
#include "kali/device/sampler.h"
#include "kali/device/pipeline.h"
#include "kali/device/shader.h"

namespace kali {

inline void bind_python_var(ShaderCursor cursor, nb::handle var)
{
#define HANDLE_VECTOR_TYPE(type)                                                                                       \
    if (nb::isinstance<type>(var)) {                                                                                   \
        cursor = nb::cast<type>(var);                                                                                  \
        return;                                                                                                        \
    }

    HANDLE_VECTOR_TYPE(uint2);
    HANDLE_VECTOR_TYPE(uint3);
    HANDLE_VECTOR_TYPE(uint4);
    HANDLE_VECTOR_TYPE(bool2);
    HANDLE_VECTOR_TYPE(bool3);
    HANDLE_VECTOR_TYPE(bool4);
    HANDLE_VECTOR_TYPE(int2);
    HANDLE_VECTOR_TYPE(int3);
    HANDLE_VECTOR_TYPE(int4);
    HANDLE_VECTOR_TYPE(float2);
    HANDLE_VECTOR_TYPE(float3);
    HANDLE_VECTOR_TYPE(float4);

#undef HANDLE_VECTOR_TYPE

    if (nb::isinstance<nb::int_>(var)) {
        cursor = nb::cast<int>(var);
    } else if (nb::isinstance<nb::float_>(var)) {
        cursor = nb::cast<float>(var);
    } else if (nb::isinstance<ResourceView>(var)) {
        cursor = ref<ResourceView>(nb::cast<ResourceView*>(var));
    } else if (nb::isinstance<Buffer>(var)) {
        cursor = ref<Buffer>(nb::cast<Buffer*>(var));
    } else if (nb::isinstance<Texture>(var)) {
        cursor = ref<Texture>(nb::cast<Texture*>(var));
    } else if (nb::isinstance<Sampler>(var)) {
        cursor = ref<Sampler>(nb::cast<Sampler*>(var));
    } else if (nb::isinstance<AccelerationStructure>(var)) {
        cursor = ref<AccelerationStructure>(nb::cast<AccelerationStructure*>(var));
    } else if (nb::isinstance<MutableShaderObject>(var)) {
        cursor = ref<MutableShaderObject>(nb::cast<MutableShaderObject*>(var));
    } else if (nb::isinstance<nb::list>(var)) {
        uint32_t index = 0;
        for (const auto& value : nb::cast<nb::list>(var))
            bind_python_var(cursor[index++], value);
    } else if (nb::isinstance<nb::dict>(var)) {
        for (const auto& [key, value] : nb::cast<nb::dict>(var))
            bind_python_var(cursor[nb::cast<std::string_view>(key)], value);
#if KALI_HAS_CUDA
    } else if (nb::isinstance<nb::ndarray<nb::device::cuda>>(var)) {
        cursor.set_cuda_tensor_view(ndarray_to_cuda_tensor_view(nb::cast<nb::ndarray<nb::device::cuda>>(var)));
#endif
    } else {
        KALI_THROW("Unsupported variable type!");
    }
}

} // namespace kali

KALI_PY_EXPORT(device_kernel)
{
    using namespace kali;

    nb::class_<Kernel, DeviceResource>(m, "Kernel") //
        .def_prop_ro("program", &Kernel::program)
        .def_prop_ro("reflection", &Kernel::reflection);

    nb::class_<ComputeKernel, Kernel>(m, "ComputeKernel")
        .def_prop_ro("pipeline", &ComputeKernel::pipeline)
        .def(
            "dispatch",
            [](ComputeKernel* self, uint3 thread_count, nb::dict vars, CommandBuffer* command_buffer, nb::kwargs kwargs)
            {
                auto bind_vars = [&](ShaderCursor cursor)
                {
                    // bind locals
                    if (kwargs.size() > 0)
                        bind_python_var(cursor.find_entry_point(0), kwargs);
                    // bind globals
                    bind_python_var(cursor, vars);
                };
                self->dispatch(thread_count, bind_vars, command_buffer);
            },
            "thread_count"_a,
            "vars"_a = nb::dict(),
            "command_buffer"_a = nullptr,
            "kwargs"_a
        );
}
