// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/core/file_system_watcher.h"

SGL_PY_EXPORT(core_filesystemwatcher)
{
    using namespace sgl;
    nb::sgl_enum<FileSystemWatcherChange>(m, "FileSystemWatcherChange");

    nb::class_<FileSystemWatchDesc>(m, "FileSystemWatchDesc", D_NA())
        .def(nb::init<>())
        .def_rw("directory", &FileSystemWatchDesc::directory, D_NA())
        .def_rw("recursive", &FileSystemWatchDesc::recursive, D_NA());

    
    nb::class_<FileSystemWatchEvent>(m, "FileSystemWatchEvent", D_NA())
        .def(nb::init<>())      
        .def_ro("path", &FileSystemWatchEvent::path, D_NA())
        .def_ro("change", &FileSystemWatchEvent::change, D_NA());

    nb::class_<FileSystemWatcher, Object>(m, "FileSystemWatcher", D_NA())
        .def(nb::init<>())
        .def("add_watch", &FileSystemWatcher::add_watch, "desc"_a, D_NA())
        .def(
            "add_watch",
            [](FileSystemWatcher* self,
               std::filesystem::path directory,
               bool recursive)
            {
                return self->add_watch({.directory = directory, .recursive = recursive});
            },
            "directory"_a,
            "recursive"_a,
            D_NA()
        )
        .def("remove_watch", &FileSystemWatcher::remove_watch, "directory"_a, D_NA())
        .def_prop_ro("watch_count", &FileSystemWatcher::get_watch_count, D_NA())
        .def_prop_rw("on_change", &FileSystemWatcher::get_on_change, &FileSystemWatcher::set_on_change, "callback"_a, D_NA())
        .def("update", &FileSystemWatcher::update, D_NA());
}
