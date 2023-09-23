#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace kali {
void register_kali_core(nb::module_& m);
void register_kali_math(nb::module_& m);
void register_kali_device(nb::module_& m);
void register_kali_imageio(nb::module_& m);
} // namespace kali

NB_MODULE(kali_ext, m)
{
    kali::register_kali_core(m);
    kali::register_kali_math(m);
    kali::register_kali_device(m);
    kali::register_kali_imageio(m);
}
