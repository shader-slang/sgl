#include <kali/core/window.h>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

namespace nb = nanobind;
using namespace nb::literals;

namespace kali {

NB_MODULE(core, m)
{
    nb::class_<Window> window(m, "Window");
    window.def(
        nb::init<uint32_t, uint32_t, const std::string&>(), "width"_a = 1024, "height"_a = 1024, "title"_a = "kali");
    window.def_prop_ro("width", &Window::get_width);
    window.def_prop_ro("height", &Window::get_height);
    window.def("resize", &Window::resize, "width"_a, "height"_a);
    window.def_prop_rw("title", &Window::get_title, &Window::set_title);
    window.def("main_loop", &Window::main_loop);
}

} // namespace kali
