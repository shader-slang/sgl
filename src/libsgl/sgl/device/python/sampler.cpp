// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/sampler.h"

namespace sgl {
SGL_DICT_TO_DESC_BEGIN(SamplerDesc)
SGL_DICT_TO_DESC_FIELD(min_filter, TextureFilteringMode)
SGL_DICT_TO_DESC_FIELD(mag_filter, TextureFilteringMode)
SGL_DICT_TO_DESC_FIELD(mip_filter, TextureFilteringMode)
SGL_DICT_TO_DESC_FIELD(reduction_op, TextureReductionOp)
SGL_DICT_TO_DESC_FIELD(address_u, TextureAddressingMode)
SGL_DICT_TO_DESC_FIELD(address_v, TextureAddressingMode)
SGL_DICT_TO_DESC_FIELD(address_w, TextureAddressingMode)
SGL_DICT_TO_DESC_FIELD(mip_lod_bias, float)
SGL_DICT_TO_DESC_FIELD(max_anisotropy, uint32_t)
SGL_DICT_TO_DESC_FIELD(comparison_func, ComparisonFunc)
SGL_DICT_TO_DESC_FIELD(border_color, float4)
SGL_DICT_TO_DESC_FIELD(min_lod, float)
SGL_DICT_TO_DESC_FIELD(max_lod, float)
SGL_DICT_TO_DESC_END()
} // namespace sgl

SGL_PY_EXPORT(device_sampler)
{
    using namespace sgl;

    nb::class_<SamplerDesc>(m, "SamplerDesc")
        .def(nb::init<>())
        .def("__init__", [](SamplerDesc* self, nb::dict dict) { new (self) SamplerDesc(dict_to_SamplerDesc(dict)); })
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
        .def_rw("max_lod", &SamplerDesc::max_lod);
    nb::implicitly_convertible<nb::dict, SamplerDesc>();

    nb::class_<Sampler, DeviceResource>(m, "Sampler", D(Sampler)).def_prop_ro("desc", &Sampler::desc, D(Sampler, desc));
}
