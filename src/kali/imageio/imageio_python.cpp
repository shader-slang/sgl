#include "imageio/imageio.h"

#include "core/object_python.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/filesystem.h>

namespace nb = nanobind;
using namespace nb::literals;

namespace kali {

void register_imageio(nb::module_& m)
{
    nb::enum_<ComponentType>(m, "ComponentType")
        .value("unknown", ComponentType::unknown)
        .value("u8", ComponentType::u8)
        .value("u16", ComponentType::u16)
        .value("u32", ComponentType::u32)
        .value("f16", ComponentType::f16)
        .value("f32", ComponentType::f32);

    nb::class_<ImageSpec>(m, "ImageSpec")
        .def_rw("width", &ImageSpec::width)
        .def_rw("height", &ImageSpec::height)
        .def_rw("component_count", &ImageSpec::component_count)
        .def_rw("component_type", &ImageSpec::component_type);

    nb::class_<ImageInput, Object>(m, "ImageInput")
        .def_static("open", &ImageInput::open, "path"_a)
        .def_prop_ro("spec", &ImageInput::get_spec);

    nb::class_<ImageOutput, Object>(m, "ImageOutput").def_static("open", &ImageOutput::open, "path"_a, "spec"_a);
}

} // namespace kali
