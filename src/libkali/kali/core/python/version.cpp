#include "nanobind.h"

#include "kali/core/version.h"

KALI_PY_EXPORT(core_version)
{
    using namespace kali;

    nb::class_<Version>(m, "Version")
        .def_ro("minor", &Version::minor)
        .def_ro("major", &Version::major)
        .def_ro("git_commit", &Version::git_commit)
        .def_ro("git_branch", &Version::git_branch)
        .def_ro("git_dirty", &Version::git_dirty)
        .def_ro("short_tag", &Version::short_tag)
        .def_ro("long_tag", &Version::long_tag)
        .def("__str__", [](const Version& self) { return self.long_tag; });
    m.attr("version") = get_version();
}
