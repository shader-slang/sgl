#include "nanobind.h"

#include "kali/device/sampler.h"

KALI_PY_EXPORT(device_sampler)
{
    using namespace kali;

    nb::class_<SamplerDesc>(m, "SamplerDesc")
        .def(nb::init<>())
        .def_rw("min_filter", &SamplerDesc::min_filter)
        .def_rw("mag_filter", &SamplerDesc::mag_filter)
        .def_rw("mip_filter", &SamplerDesc::mip_filter)
        .def_rw("reduction_op", &SamplerDesc::reduction_op)
        .def_rw("address_u", &SamplerDesc::address_u)
        .def_rw("address_v", &SamplerDesc::address_v)
        .def_rw("address_w", &SamplerDesc::address_w)
        .def_rw("mip_lod_bias", &SamplerDesc::mip_lod_bias)
        .def_rw("max_anisotropy", &SamplerDesc::max_anisotropy)
        .def_rw("comparison_func", &SamplerDesc::comparison_func)
        .def_rw("border_color", &SamplerDesc::border_color)
        .def_rw("min_lod", &SamplerDesc::min_lod)
        .def_rw("max_lod", &SamplerDesc::max_lod)
        .def("__repr__", &SamplerDesc::to_string);

    nb::class_<Sampler, DeviceResource>(m, "Sampler").def_prop_ro("desc", &Sampler::desc);
}
