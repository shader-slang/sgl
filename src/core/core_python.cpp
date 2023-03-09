#include <kali/core/window.h>

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace kali {

int add(int a, int b)
{
    return a + b;
}

NB_MODULE(core, m)
{
    // nb::class_<Window> window(m, "Window", nb::intrusive_ptr{});

    m.def("add", &add);
}

} // namespace kali
