// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/device.h"
#include "sgl/device/surface.h"

namespace sgl {

SGL_DICT_TO_DESC_BEGIN(SurfaceConfig)
SGL_DICT_TO_DESC_FIELD(format, Format)
SGL_DICT_TO_DESC_FIELD(usage, TextureUsage)
SGL_DICT_TO_DESC_FIELD(width, uint32_t)
SGL_DICT_TO_DESC_FIELD(height, uint32_t)
SGL_DICT_TO_DESC_FIELD(desired_image_count, uint32_t)
SGL_DICT_TO_DESC_FIELD(vsync, bool)
SGL_DICT_TO_DESC_END()

} // namespace sgl

SGL_PY_EXPORT(device_surface)
{
    using namespace sgl;

    nb::class_<SurfaceInfo>(m, "SurfaceInfo", D_NA(SurfaceInfo))
        .def_ro("preferred_format", &SurfaceInfo::preferred_format, D_NA(SurfaceInfo, preferred_format))
        .def_ro("supported_usage", &SurfaceInfo::supported_usage, D_NA(SurfaceInfo, supported_usage))
        .def_ro("formats", &SurfaceInfo::formats, D_NA(SurfaceInfo, formats));


    nb::class_<SurfaceConfig>(m, "SurfaceConfig", D_NA(SurfaceConfig))
        .def(nb::init<>())
        .def(
            "__init__",
            [](SurfaceConfig* self, nb::dict dict) { new (self) SurfaceConfig(dict_to_SurfaceConfig(dict)); }
        )
        .def_rw("format", &SurfaceConfig::format, D_NA(SurfaceConfig, format))
        .def_rw("usage", &SurfaceConfig::usage, D_NA(SurfaceConfig, usage))
        .def_rw("width", &SurfaceConfig::width, D_NA(SurfaceConfig, width))
        .def_rw("height", &SurfaceConfig::height, D_NA(SurfaceConfig, height))
        .def_rw("desired_image_count", &SurfaceConfig::desired_image_count, D_NA(SurfaceConfig, desired_image_count))
        .def_rw("vsync", &SurfaceConfig::vsync, D_NA(SurfaceConfig, vsync));
    nb::implicitly_convertible<nb::dict, SurfaceConfig>();

    nb::class_<Surface, Object>(m, "Surface", D_NA(Surface))
        .def_prop_ro("info", &Surface::info, D_NA(Surface, info))
        .def_prop_ro("config", &Surface::config, D_NA(Surface, config))
        .def("configure", &Surface::configure, "config"_a, D_NA(Surface, configure))
        .def("get_current_texture", &Surface::get_current_texture, D_NA(Surface, get_current_texture))
        .def("present", &Surface::present, D_NA(Surface, present));
}
