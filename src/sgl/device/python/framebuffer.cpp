// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/framebuffer.h"
#include "sgl/device/resource.h"

namespace sgl {
SGL_DICT_TO_DESC_BEGIN(FramebufferLayoutTargetDesc)
SGL_DICT_TO_DESC_FIELD(format, Format)
SGL_DICT_TO_DESC_FIELD(sample_count, uint32_t)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(FramebufferLayoutDesc)
SGL_DICT_TO_DESC_FIELD_LIST(render_targets, FramebufferLayoutTargetDesc)
SGL_DICT_TO_DESC_FIELD(depth_stencil, std::optional<FramebufferLayoutTargetDesc>)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(FramebufferDesc)
SGL_DICT_TO_DESC_FIELD(render_targets, std::vector<ref<ResourceView>>)
SGL_DICT_TO_DESC_FIELD(depth_stencil, ref<ResourceView>)
SGL_DICT_TO_DESC_FIELD(layout, ref<FramebufferLayout>)
SGL_DICT_TO_DESC_END()
} // namespace sgl

SGL_PY_EXPORT(device_framebuffer)
{
    using namespace sgl;

    nb::class_<FramebufferLayoutTargetDesc>(m, "FramebufferLayoutTargetDesc", D(FramebufferLayoutTargetDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](FramebufferLayoutTargetDesc* self, nb::dict dict)
            { new (self) FramebufferLayoutTargetDesc(dict_to_FramebufferLayoutTargetDesc(dict)); }
        )
        .def_rw("format", &FramebufferLayoutTargetDesc::format, D(FramebufferLayoutTargetDesc, format))
        .def_rw(
            "sample_count",
            &FramebufferLayoutTargetDesc::sample_count,
            D(FramebufferLayoutTargetDesc, sample_count)
        );
    nb::implicitly_convertible<nb::dict, FramebufferLayoutTargetDesc>();

    nb::class_<FramebufferLayoutDesc>(m, "FramebufferLayoutDesc", D(FramebufferLayoutDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](FramebufferLayoutDesc* self, nb::dict dict)
            { new (self) FramebufferLayoutDesc(dict_to_FramebufferLayoutDesc(dict)); }
        )
        .def_rw("render_targets", &FramebufferLayoutDesc::render_targets, D(FramebufferLayoutDesc, render_targets))
        .def_rw("depth_stencil", &FramebufferLayoutDesc::depth_stencil, D(FramebufferLayoutDesc, depth_stencil));
    nb::implicitly_convertible<nb::dict, FramebufferLayoutDesc>();

    nb::class_<FramebufferLayout, DeviceResource>(m, "FramebufferLayout", D(FramebufferLayout))
        .def_prop_ro("desc", &FramebufferLayout::desc, D(FramebufferLayout, desc));

    nb::class_<FramebufferDesc>(m, "FramebufferDesc", D(FramebufferDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](FramebufferDesc* self, nb::dict dict) { new (self) FramebufferDesc(dict_to_FramebufferDesc(dict)); }
        )
        .def_rw("render_targets", &FramebufferDesc::render_targets, D(FramebufferDesc, render_targets))
        .def_rw("depth_stencil", &FramebufferDesc::depth_stencil, D(FramebufferDesc, depth_stencil))
        .def_rw("layout", &FramebufferDesc::layout, D(FramebufferDesc, layout));
    nb::implicitly_convertible<nb::dict, FramebufferDesc>();

    nb::class_<Framebuffer, DeviceResource>(m, "Framebuffer", D(Framebuffer))
        .def_prop_ro("desc", &Framebuffer::desc, D(Framebuffer, desc))
        .def_prop_ro("layout", &Framebuffer::layout, D(Framebuffer, layout));
}
