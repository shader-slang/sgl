#include "nanobind.h"

#include "kali/device/kernel.h"
#include "kali/device/command.h"
#include "kali/device/resource.h"
#include "kali/device/sampler.h"

namespace kali {

inline void bind_python_var(ShaderCursor cursor, nb::handle var)
{
    if (nb::isinstance<ResourceView>(var)) {
        cursor = ref<ResourceView>(nb::cast<ResourceView*>(var));
    } else if (nb::isinstance<Buffer>(var)) {
        cursor = ref<Buffer>(nb::cast<Buffer*>(var));
    } else if (nb::isinstance<Texture>(var)) {
        cursor = ref<Texture>(nb::cast<Texture*>(var));
    } else if (nb::isinstance<Sampler>(var)) {
        cursor = ref<Sampler>(nb::cast<Sampler*>(var));
    } else if (nb::isinstance<nb::list>(var)) {
        uint32_t index = 0;
        for (const auto& value : nb::cast<nb::list>(var))
            bind_python_var(cursor[index++], value);
    } else if (nb::isinstance<nb::dict>(var)) {
        for (const auto& [key, value] : nb::cast<nb::dict>(var))
            bind_python_var(cursor[nb::cast<std::string_view>(key)], value);
    } else {
        KALI_THROW("Unsupported variable type!");
    }
}

} // namespace kali

KALI_PY_EXPORT(device_kernel)
{
    using namespace kali;

    nb::class_<Kernel, Object>(m, "Kernel");

    nb::class_<ComputeKernel, Kernel>(m, "ComputeKernel")
        .def(
            "dispatch",
            [](ComputeKernel* self, uint3 thread_count, nb::dict vars, CommandStream* stream, nb::kwargs kwargs)
            {
                auto bind_vars = [&](ShaderCursor cursor)
                {
                    // bind locals
                    if (kwargs.size() > 0)
                        bind_python_var(cursor.find_entry_point(0), kwargs);
                    // bind globals
                    bind_python_var(cursor, vars);
                };
                self->dispatch(thread_count, bind_vars, stream);
            },
            "thread_count"_a,
            "vars"_a = nb::dict(),
            "stream"_a = nullptr,
            "kwargs"_a = nb::kwargs()
        );
}