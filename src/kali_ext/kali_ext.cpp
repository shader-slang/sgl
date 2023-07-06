#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace kali {
void register_kali(nb::module_& m);
void register_kali_math(nb::module_& m);
void register_kali_rhi(nb::module_& m);
void register_kali_imageio(nb::module_& m);
} // namespace kali

NB_MODULE(kali_ext, m)
{
    kali::register_kali(m);
    kali::register_kali_math(m);
    kali::register_kali_rhi(m);
    kali::register_kali_imageio(m);
}
