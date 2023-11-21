#include "nanobind.h"

#include "kali/utils/tev.h"
#include "kali/core/bitmap.h"

KALI_PY_EXPORT(utils_tev)
{
    using namespace kali;

    nb::module_ utils = m.attr("utils");

    utils.def(
        "show_in_tev",
        &utils::show_in_tev,
        "bitmap"_a,
        "name"_a = std::optional<std::string>{},
        "host"_a = "127.0.0.1",
        "port"_a = 14158
    );
    utils.def(
        "show_in_tev_async",
        &utils::show_in_tev_async,
        "bitmap"_a,
        "name"_a = std::optional<std::string>{},
        "host"_a = "127.0.0.1",
        "port"_a = 14158
    );
}
