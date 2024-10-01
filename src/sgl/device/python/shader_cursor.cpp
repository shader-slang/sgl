// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/shader_cursor.h"
#include "sgl/device/shader_object.h"
#include "sgl/device/resource.h"
#include "sgl/device/sampler.h"
#include "sgl/device/raytracing.h"
#include "sgl/device/cuda_interop.h"

#include "sgl/device/python/cursor_utils.h"

namespace sgl {
namespace detail {

    class ShaderCursorWriteConverterTable : public WriteConverterTable<ShaderCursor> {
        bool write_value(ShaderCursor& self, nb::object nbval) override
        {
            if (WriteConverterTable<ShaderCursor>::write_value(self, nbval))
                return true;

            ref<MutableShaderObject> msoref;
            if (nb::try_cast(nbval, msoref)) {
                self.set_object(msoref);
                return true;
            }

            ref<ResourceView> rvref;
            if (nb::try_cast(nbval, rvref)) {
                self.set_resource(rvref);
                return true;
            }

            ref<Buffer> bufref;
            if (nb::try_cast(nbval, bufref)) {
                self.set_buffer(bufref);
                return true;
            }

            ref<Texture> texref;
            if (nb::try_cast(nbval, texref)) {
                self.set_texture(texref);
                return true;
            }

            ref<Sampler> samplerref;
            if (nb::try_cast(nbval, samplerref)) {
                self.set_sampler(samplerref);
                return true;
            }

            ref<AccelerationStructure> asref;
            if (nb::try_cast(nbval, asref)) {
                self.set_acceleration_structure(asref);
                return true;
            }

            nb::ndarray<nb::device::cuda> cudaarray;
            if (nb::try_cast(nbval, cudaarray)) {
                self.set_cuda_tensor_view(ndarray_to_cuda_tensor_view(cudaarray));
                return true;
            }

            return false;
        }
    };

    static ShaderCursorWriteConverterTable _writeconv;
} // namespace detail
} // namespace sgl

SGL_PY_EXPORT(device_shader_cursor)
{
    using namespace sgl;

    nb::class_<ShaderOffset>(m, "ShaderOffset", D(ShaderOffset))
        .def_ro("uniform_offset", &ShaderOffset::uniform_offset, D(ShaderOffset, uniform_offset))
        .def_ro("binding_range_index", &ShaderOffset::binding_range_index, D(ShaderOffset, binding_range_index))
        .def_ro("binding_array_index", &ShaderOffset::binding_array_index, D(ShaderOffset, binding_array_index))
        .def("is_valid", &ShaderOffset::is_valid, D(ShaderOffset, is_valid));

    nb::class_<ShaderCursor> shader_cursor(m, "ShaderCursor", D(ShaderCursor));

    shader_cursor //
        .def(nb::init<ShaderObject*>(), "shader_object"_a, D(ShaderCursor, ShaderCursor))
        .def_prop_ro("_offset", &ShaderCursor::offset, D(ShaderCursor, offset))
        .def("dereference", &ShaderCursor::dereference, D(ShaderCursor, dereference))
        .def("find_entry_point", &ShaderCursor::find_entry_point, "index"_a, D(ShaderCursor, find_entry_point));

    bind_traversable_cursor(shader_cursor);

    bind_writable_cursor(detail::_writeconv, shader_cursor);
}
