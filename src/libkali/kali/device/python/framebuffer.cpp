// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "kali/device/framebuffer.h"
#include "kali/device/resource.h"

namespace kali {
KALI_DICT_TO_DESC_BEGIN(FramebufferAttachmentDesc)
KALI_DICT_TO_DESC_FIELD(texture, ref<Texture>)
KALI_DICT_TO_DESC_FIELD(mip_level, uint32_t)
KALI_DICT_TO_DESC_FIELD(base_array_layer, uint32_t)
KALI_DICT_TO_DESC_FIELD(layer_count, uint32_t)
KALI_DICT_TO_DESC_END()

KALI_DICT_TO_DESC_BEGIN(FramebufferDesc)
KALI_DICT_TO_DESC_FIELD_LIST(render_targets, FramebufferAttachmentDesc)
KALI_DICT_TO_DESC_FIELD(depth_stencil, FramebufferAttachmentDesc)
KALI_DICT_TO_DESC_END()
} // namespace kali

KALI_PY_EXPORT(device_framebuffer)
{
    using namespace kali;

    nb::class_<FramebufferAttachmentDesc>(m, "FramebufferAttachmentDesc")
        .def(nb::init<>())
        .def(
            "__init__",
            [](FramebufferAttachmentDesc* self, nb::dict dict)
            { new (self) FramebufferAttachmentDesc(dict_to_FramebufferAttachmentDesc(dict)); }
        )
        .def_rw("texture", &FramebufferAttachmentDesc::texture)
        .def_rw("mip_level", &FramebufferAttachmentDesc::mip_level)
        .def_rw("base_array_layer", &FramebufferAttachmentDesc::base_array_layer)
        .def_rw("layer_count", &FramebufferAttachmentDesc::layer_count);
    nb::implicitly_convertible<nb::dict, FramebufferAttachmentDesc>();

    nb::class_<FramebufferDesc>(m, "FramebufferDesc")
        .def(nb::init<>())
        .def(
            "__init__",
            [](FramebufferDesc* self, nb::dict dict) { new (self) FramebufferDesc(dict_to_FramebufferDesc(dict)); }
        )
        .def_rw("render_targets", &FramebufferDesc::render_targets)
        .def_rw("depth_stencil", &FramebufferDesc::depth_stencil);
    nb::implicitly_convertible<nb::dict, FramebufferDesc>();

    nb::class_<Framebuffer, DeviceResource>(m, "Framebuffer", D(Framebuffer))
        .def_prop_ro("desc", &Framebuffer::desc, D(Framebuffer, desc));
}
