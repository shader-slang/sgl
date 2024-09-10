// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/shader_cursor.h"
#include "sgl/device/shader_object.h"
#include "sgl/device/resource.h"
#include "sgl/device/sampler.h"
#include "sgl/device/raytracing.h"
#include "sgl/device/cuda_interop.h"

#include "sgl/device/python/cursor_shared.h"

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
        .def_prop_ro("offset", &ShaderCursor::offset, D(ShaderCursor, offset))
        .def("dereference", &ShaderCursor::dereference, D(ShaderCursor, dereference))
        .def("find_entry_point", &ShaderCursor::find_entry_point, "index"_a, D(ShaderCursor, find_entry_point))
        .def("set_object", &ShaderCursor::set_object, "object"_a, D(ShaderCursor, set_object))
        .def("set_resource", &ShaderCursor::set_resource, "resource_view"_a, D(ShaderCursor, set_resource))
        .def("set_buffer", &ShaderCursor::set_buffer, "buffer"_a, D(ShaderCursor, set_buffer))
        .def("set_texture", &ShaderCursor::set_texture, "texture"_a, D(ShaderCursor, set_texture))
        .def("set_sampler", &ShaderCursor::set_sampler, "sampler"_a, D(ShaderCursor, set_sampler))
        .def(
            "set_acceleration_structure",
            &ShaderCursor::set_acceleration_structure,
            "acceleration_structure"_a,
            D(ShaderCursor, set_acceleration_structure)
        )
        .def(
            "set_data",
            [](ShaderCursor& self, nb::ndarray<nb::device::cpu> data)
            {
                SGL_CHECK(is_ndarray_contiguous(data), "data is not contiguous");
                self.set_data(data.data(), data.nbytes());
            },
            "data"_a,
            D(ShaderCursor, set_data)
        );

    bind_traversable_cursor(shader_cursor);
    bind_writable_cursor_basic_types(shader_cursor);

#define def_setter(type)                                                                                               \
    shader_cursor.def(                                                                                                 \
        "__setitem__",                                                                                                 \
        [](ShaderCursor& self, std::string_view name, type value) { self[name] = value; }                              \
    );                                                                                                                 \
    shader_cursor.def("__setattr__", [](ShaderCursor& self, std::string_view name, type value) { self[name] = value; });

    def_setter(ref<MutableShaderObject>);
    def_setter(ref<ResourceView>);
    def_setter(ref<Buffer>);
    def_setter(ref<Texture>);
    def_setter(ref<Sampler>);
    def_setter(ref<AccelerationStructure>);

#undef def_setter

    auto set_cuda_tensor_field = [](ShaderCursor& self, std::string_view name, nb::ndarray<nb::device::cuda> ndarray)
    { self[name].set_cuda_tensor_view(ndarray_to_cuda_tensor_view(ndarray)); };

    auto set_cuda_tensor_element = [](ShaderCursor& self, int index, nb::ndarray<nb::device::cuda> ndarray)
    { self[index].set_cuda_tensor_view(ndarray_to_cuda_tensor_view(ndarray)); };

    shader_cursor.def("__setitem__", set_cuda_tensor_field);
    shader_cursor.def("__setitem__", set_cuda_tensor_element);
    shader_cursor.def("__setattr__", set_cuda_tensor_field);
}
