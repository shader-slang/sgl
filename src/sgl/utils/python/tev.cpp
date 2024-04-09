// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/utils/tev.h"
#include "sgl/core/bitmap.h"
#include "sgl/device/resource.h"

SGL_PY_EXPORT(utils_tev)
{
    using namespace sgl;

    nb::module_ utils = m.attr("utils");

    utils.def(
        "show_in_tev",
        nb::overload_cast<const Bitmap*, std::string, std::string, uint16_t, uint32_t>(&utils::show_in_tev),
        "bitmap"_a,
        "name"_a = "",
        "host"_a = "127.0.0.1",
        "port"_a = 14158,
        "max_retries"_a = 3,
        D(utils, show_in_tev)
    );
    utils.def(
        "show_in_tev",
        nb::overload_cast<const Texture*, std::string, std::string, uint16_t, uint32_t>(&utils::show_in_tev),
        "texture"_a,
        "name"_a = "",
        "host"_a = "127.0.0.1",
        "port"_a = 14158,
        "max_retries"_a = 3,
        D(utils, show_in_tev_2)
    );
    utils.def(
        "show_in_tev_async",
        nb::overload_cast<const Bitmap*, std::string, std::string, uint16_t, uint32_t>(&utils::show_in_tev_async),
        "bitmap"_a,
        "name"_a = "",
        "host"_a = "127.0.0.1",
        "port"_a = 14158,
        "max_retries"_a = 3,
        D(utils, show_in_tev_async)
    );
    utils.def(
        "show_in_tev_async",
        nb::overload_cast<const Texture*, std::string, std::string, uint16_t, uint32_t>(&utils::show_in_tev_async),
        "texture"_a,
        "name"_a = "",
        "host"_a = "127.0.0.1",
        "port"_a = 14158,
        "max_retries"_a = 3,
        D(utils, show_in_tev_async_2)
    );
}
