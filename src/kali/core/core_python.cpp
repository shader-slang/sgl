#include "core/error.h"
#include "core/window.h"
#include "core/version.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

namespace nb = nanobind;
using namespace nb::literals;

void register_rhi(nb::module_& m);

namespace kali {

NB_MODULE(kali, m)
{
    nb::exception<Exception>(m, "Exception", PyExc_RuntimeError);

    nb::class_<Version> version(m, "Version");
    version.def_ro("minor", &Version::minor);
    version.def_ro("major", &Version::major);
    version.def_ro("git_commit", &Version::git_commit);
    version.def_ro("git_branch", &Version::git_branch);
    version.def_ro("git_dirty", &Version::git_dirty);
    version.def_ro("short_tag", &Version::short_tag);
    version.def_ro("long_tag", &Version::long_tag);
    m.def("get_version", []() { return get_version(); });

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
