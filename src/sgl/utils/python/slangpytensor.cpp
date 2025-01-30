// SPDX-License-Identifier: Apache-2.0

#include <initializer_list>
#include "nanobind.h"

#include "sgl/device/device.h"
#include "sgl/device/buffer_cursor.h"

#include "sgl/utils/python/slangpytensor.h"

namespace sgl {

extern void write_shader_cursor(ShaderCursor& cursor, nb::object value);
extern nb::ndarray<nb::numpy> buffer_to_numpy(Buffer* self);
extern void buffer_from_numpy(Buffer* self, nb::ndarray<nb::numpy> data);
extern nb::ndarray<nb::pytorch, nb::device::cuda>
buffer_to_torch(Buffer* self, DataType type, std::vector<size_t> shape, std::vector<int64_t> strides, size_t offset);

} // namespace sgl

namespace sgl::slangpy {

NativeTensor::NativeTensor(NativeTensorDesc desc, const ref<Buffer>& storage)
    : m_desc(desc)
    , m_storage(storage)
{
    set_slangpy_signature(fmt::format("[{},{},{}]", desc.dtype->get_type_reflection()->name(), dims(), usage()));
}

ref<BufferCursor> NativeTensor::cursor(std::optional<int> start, std::optional<int> count) const
{
    size_t el_stride = m_desc.element_layout->stride();
    size_t size = (count.value_or(element_count())) * el_stride;
    size_t offset = (start.value_or(0)) * el_stride;
    return make_ref<BufferCursor>(m_desc.element_layout, m_storage, size, offset);
}

nb::dict NativeTensor::uniforms() const
{
    nb::dict res;
    res["buffer"] = m_storage;
    res["strides"] = m_desc.strides.as_vector();
    res["shape"] = m_desc.shape.as_vector();
    return res;
}

Shape NativeTensorMarshall::get_shape(nb::object data) const
{
    auto buffer = nb::cast<NativeTensor*>(data);
    return buffer->shape();
}

void NativeTensorMarshall::write_shader_cursor_pre_dispatch(
    CallContext* context,
    NativeBoundVariableRuntime* binding,
    ShaderCursor cursor,
    nb::object value,
    nb::list read_back
) const
{
    SGL_UNUSED(read_back);

    // TODO: This function can be a lot more efficient and cleaner
    // optimize in next pass.

    // Cast value to buffer, and get the cursor field to write to.
    auto buffer = nb::cast<NativeTensor*>(value);
    ShaderCursor field = cursor[binding->get_variable_name()];

    // Write the buffer storage.
    field["buffer"] = buffer->storage();

    // Write shape vector as an array of ints.
    const std::vector<int>& shape_vec = buffer->shape().as_vector();
    field["shape"]._set_array_unsafe(&shape_vec[0], shape_vec.size() * 4, shape_vec.size());

    // Generate and write strides vector, clearing strides to 0
    // for dimensions that are broadcast.
    std::vector<int> strides_vec = buffer->strides().as_vector();
    const std::vector<int>& transform = binding->get_transform().as_vector();
    const std::vector<int>& call_shape = context->call_shape().as_vector();
    for (size_t i = 0; i < transform.size(); i++) {
        int csidx = transform[i];
        if (call_shape[csidx] != shape_vec[i]) {
            strides_vec[i] = 0;
        }
    }

    // Write the strides vector as an array of ints.
    field["strides"]._set_array_unsafe(&strides_vec[0], strides_vec.size() * 4, strides_vec.size());
}

void NativeTensorMarshall::read_calldata(
    CallContext* context,
    NativeBoundVariableRuntime* binding,
    nb::object data,
    nb::object result
) const
{
    SGL_UNUSED(context);
    SGL_UNUSED(binding);
    SGL_UNUSED(data);
    SGL_UNUSED(result);
}

nb::object NativeTensorMarshall::create_output(CallContext* context, NativeBoundVariableRuntime* binding) const
{
    SGL_UNUSED(context);
    SGL_UNUSED(binding);
    return nb::none();
}

nb::object
NativeTensorMarshall::read_output(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data) const
{
    SGL_UNUSED(context);
    SGL_UNUSED(binding);
    return data;
}

nb::object NativeTensorMarshall::create_dispatchdata(nb::object data) const
{
    // Cast value to buffer, and get the cursor field to write to.
    auto buffer = nb::cast<NativeTensor*>(data);
    nb::dict res;
    res["buffer"] = buffer->storage();
    res["shape"] = buffer->shape().as_vector();
    res["strides"] = buffer->strides().as_vector();
    return res;
}

} // namespace sgl::slangpy

SGL_PY_EXPORT(utils_slangpy_tensor)
{
    using namespace sgl;
    using namespace sgl::slangpy;

    nb::module_ slangpy = m.attr("slangpy");

    nb::class_<NativeTensorDesc>(slangpy, "NativeTensorDesc")
        .def(nb::init<>())
        .def_rw("dtype", &NativeTensorDesc::dtype)
        .def_rw("element_layout", &NativeTensorDesc::element_layout)
        .def_rw("shape", &NativeTensorDesc::shape)
        .def_rw("strides", &NativeTensorDesc::strides)
        .def_rw("offset", &NativeTensorDesc::offset)
        .def_rw("grad_in", &NativeTensorDesc::grad_in, nb::arg().none())
        .def_rw("grad_out", &NativeTensorDesc::grad_out, nb::arg().none());

    nb::class_<NativeTensor, NativeObject>(slangpy, "NativeTensor")
        .def(nb::init<NativeTensorDesc, const ref<Buffer>&>())
        .def_prop_ro("device", &NativeTensor::device)
        .def_prop_ro("dtype", &NativeTensor::dtype)
        .def_prop_ro("shape", &NativeTensor::shape)
        .def_prop_ro("strides", &NativeTensor::strides)
        .def_prop_ro("offset", &NativeTensor::offset)
        .def_prop_ro("element_count", &NativeTensor::element_count)
        .def_prop_ro("usage", &NativeTensor::usage)
        .def_prop_ro("memory_type", &NativeTensor::memory_type)
        .def_prop_ro("storage", &NativeTensor::storage)
        .def_prop_ro("grad_in", &NativeTensor::grad_in, nb::none())
        .def_prop_ro("grad_out", &NativeTensor::grad_out, nb::none())
        .def("cursor", &NativeTensor::cursor, "start"_a.none() = std::nullopt, "count"_a.none() = std::nullopt)
        .def("uniforms", &NativeTensor::uniforms)
        .def(
            "to_numpy",
            [](NativeTensor& self) { return buffer_to_numpy(self.storage().get()); },
            D_NA(NativeTensor, buffer_to_numpy)
        )
        .def(
            "from_numpy",
            [](NativeTensor& self, nb::ndarray<nb::numpy> data) { buffer_from_numpy(self.storage().get(), data); },
            "data"_a,
            D_NA(NativeTensor, buffer_from_numpy)
        );


    nb::class_<NativeTensorMarshall, NativeMarshall>(slangpy, "NativeTensorMarshall") //
        .def(
            "__init__",
            [](NativeTensorMarshall& self,
               int dims,
               bool writable,
               ref<NativeSlangType> slang_type,
               ref<NativeSlangType> slang_element_type,
               ref<TypeLayoutReflection> element_layout,
               ref<NativeTensorMarshall> d_in,
               ref<NativeTensorMarshall> d_out) {
                new (&self)
                    NativeTensorMarshall(dims, writable, slang_type, slang_element_type, element_layout, d_in, d_out);
            },
            "dims"_a,
            "writable"_a,
            "slang_type"_a,
            "slang_element_type"_a,
            "element_layout"_a,
            "d_in"_a,
            "d_out"_a,
            D_NA(NativeTensorMarshall, NativeTensorMarshall)
        )
        .def_prop_ro("dims", &sgl::slangpy::NativeTensorMarshall::dims)
        .def_prop_ro("writable", &sgl::slangpy::NativeTensorMarshall::writable)
        .def_prop_ro("slang_element_type", &sgl::slangpy::NativeTensorMarshall::slang_element_type);
}
