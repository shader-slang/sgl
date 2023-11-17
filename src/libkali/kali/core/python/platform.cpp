#include "nanobind.h"

#include "kali/core/platform.h"

KALI_PY_EXPORT(core_platform)
{
    using namespace kali::platform;

    nb::module_ platform = m.attr("platform");

    nb::class_<FileDialogFilter>(platform, "FileDialogFilter")
        .def(nb::init<>())
        .def(nb::init<const std::string&, const std::string&>())
        .def(nb::init_implicit<std::pair<std::string, std::string>>())
        .def_rw("name", &FileDialogFilter::name)
        .def_rw("pattern", &FileDialogFilter::pattern);

    platform.def(
        "open_file_dialog",
        [](FileDialogFilterList filters) { return open_file_dialog(filters); },
        "filters"_a = FileDialogFilterList()
    );
    platform.def(
        "save_file_dialog",
        [](FileDialogFilterList filters) { return save_file_dialog(filters); },
        "filters"_a = FileDialogFilterList()
    );
    platform.def("choose_folder_dialog", &choose_folder_dialog);

    platform.attr("display_scale_factor") = display_scale_factor();

    platform.attr("executable_path") = executable_path();
    platform.attr("executable_directory") = executable_directory();
    platform.attr("executable_name") = executable_name();
    platform.attr("app_data_directory") = app_data_directory();
    platform.attr("home_directory") = home_directory();
    platform.attr("project_directory") = project_directory();
    platform.attr("runtime_directory") = runtime_directory();

    platform.attr("page_size") = page_size();

    nb::class_<MemoryStats>(platform, "MemoryStats")
        .def_ro("rss", &MemoryStats::rss)
        .def_ro("peak_rss", &MemoryStats::peak_rss);

    platform.def("memory_stats", &memory_stats);
}
