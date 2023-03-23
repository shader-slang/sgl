#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace kali {
void register_core(nb::module_& m);
void register_rhi(nb::module_& m);
} // namespace kali

NB_MODULE(kali_python, m)
{
    kali::register_core(m);
    kali::register_rhi(m);
}
