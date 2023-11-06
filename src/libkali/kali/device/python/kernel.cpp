#include "nanobind.h"

#include "kali/device/kernel.h"
#include "kali/device/command.h"

namespace kali {
inline void assign_vars(ShaderCursor cursor, const nb::dict& vars)
{
    for (const auto& [key, value] : vars) {
        std::string_view key_str = nb::cast<std::string_view>(key);
        if (nb::isinstance<Buffer*>(value)) {
            cursor[key_str] = ref<Buffer>(nb::cast<Buffer*>(value));
        } else if (nb::isinstance<nb::dict>(value)) {
            assign_vars(cursor[key_str], nb::cast<nb::dict>(value));
        }
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
                auto set_vars = [&](ShaderCursor cursor)
                {
                    // assign locals
                    if (kwargs.size() > 0)
                        assign_vars(cursor.find_entry_point(0), kwargs);
                    // assign globals
                    assign_vars(cursor, vars);
                };
                self->dispatch(thread_count, set_vars, stream);
            },
            "thread_count"_a,
            "vars"_a = nb::dict(),
            "stream"_a = nullptr,
            "kwargs"_a = nb::kwargs()
        );
}
