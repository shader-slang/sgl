#include "nanobind.h"

#include "kali/utils/tev.h"
#include "kali/core/bitmap.h"
#include "kali/device/resource.h"

KALI_PY_EXPORT(utils_tev)
{
    using namespace kali;

    nb::module_ utils = m.attr("utils");

    utils.def(
        "show_in_tev",
        nb::overload_cast<const Bitmap*, std::optional<std::string>, const std::string&, uint16_t, uint32_t>(
            &utils::show_in_tev
        ),
        "bitmap"_a,
        "name"_a = std::optional<std::string>{},
        "host"_a = "127.0.0.1",
        "port"_a = 14158,
        "max_retries"_a = 3
    );
    utils.def(
        "show_in_tev",
        nb::overload_cast<const Texture*, std::optional<std::string>, const std::string&, uint16_t, uint32_t>(
            &utils::show_in_tev
        ),
        "texture"_a,
        "name"_a = std::optional<std::string>{},
        "host"_a = "127.0.0.1",
        "port"_a = 14158,
        "max_retries"_a = 3
    );
    utils.def(
        "show_in_tev_async",
        nb::overload_cast<const Bitmap*, std::optional<std::string>, const std::string&, uint16_t, uint32_t>(
            &utils::show_in_tev_async
        ),
        "bitmap"_a,
        "name"_a = std::optional<std::string>{},
        "host"_a = "127.0.0.1",
        "port"_a = 14158,
        "max_retries"_a = 3
    );
    utils.def(
        "show_in_tev_async",
        nb::overload_cast<const Texture*, std::optional<std::string>, const std::string&, uint16_t, uint32_t>(
            &utils::show_in_tev_async
        ),
        "texture"_a,
        "name"_a = std::optional<std::string>{},
        "host"_a = "127.0.0.1",
        "port"_a = 14158,
        "max_retries"_a = 3
    );
}
