// SPDX-License-Identifier: Apache-2.0

#include <initializer_list>
#include "nanobind.h"

#include "sgl/device/device.h"
#include "sgl/device/buffer_cursor.h"

#include "sgl/utils/python/slangpybuffer.h"

namespace sgl {

extern void write_shader_cursor(ShaderCursor& cursor, nb::object value);
extern nb::ndarray<nb::numpy> buffer_to_numpy(Buffer* self);
extern void buffer_copy_from_numpy(Buffer* self, nb::ndarray<nb::numpy> data);
extern nb::ndarray<nb::pytorch, nb::device::cuda>
buffer_to_torch(Buffer* self, DataType type, std::vector<size_t> shape, std::vector<int64_t> strides, size_t offset);

} // namespace sgl

namespace sgl::slangpy {

NativeNDBuffer::NativeNDBuffer(Device* device, NativeNDBufferDesc desc)
    : m_desc(desc)
{

    BufferDesc buffer_desc;
    buffer_desc.element_count = desc.shape.element_count();
    buffer_desc.struct_size = desc.element_layout->stride();
    buffer_desc.usage = desc.usage;
    buffer_desc.memory_type = desc.memory_type;
    m_storage = device->create_buffer(buffer_desc);

    set_slangpy_signature(
        fmt::format("[{},{},{}]", desc.dtype->get_type_reflection()->full_name(), desc.shape.size(), desc.usage)
    );
}

ref<BufferCursor> NativeNDBuffer::cursor(std::optional<int> start, std::optional<int> count) const
{
    size_t el_stride = m_desc.element_layout->stride();
    size_t size = (count.value_or(element_count())) * el_stride;
    size_t offset = (start.value_or(0)) * el_stride;
    return make_ref<BufferCursor>(m_desc.element_layout, m_storage, size, offset);
}

nb::dict NativeNDBuffer::uniforms() const
{
    nb::dict res;
    res["buffer"] = m_storage;
    res["strides"] = m_desc.strides.as_vector();
    res["shape"] = m_desc.shape.as_vector();
    return res;
}

Shape NativeNDBufferMarshall::get_shape(nb::object data) const
{
    auto buffer = nb::cast<NativeNDBuffer*>(data);
    return buffer->shape();
}

void NativeNDBufferMarshall::write_shader_cursor_pre_dispatch(
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
    auto buffer = nb::cast<NativeNDBuffer*>(value);
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

void NativeNDBufferMarshall::read_calldata(
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

nb::object NativeNDBufferMarshall::create_output(CallContext* context, NativeBoundVariableRuntime* binding) const
{
    SGL_UNUSED(binding);
    auto buffer = create_buffer(context->device(), context->call_shape());
    return nb::cast(buffer);
}

ref<NativeNDBuffer> NativeNDBufferMarshall::create_buffer(Device* device, const Shape& shape) const
{
    NativeNDBufferDesc desc;
    desc.dtype = m_slang_element_type;
    desc.element_layout = m_element_layout;
    desc.shape = shape;
    desc.strides = desc.shape.calc_contiguous_strides();
    desc.usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access;
    desc.memory_type = MemoryType::device_local;
    return make_ref<NativeNDBuffer>(device, desc);
}

nb::object NativeNDBufferMarshall::create_dispatchdata(nb::object data) const
{
    // Cast value to buffer, and get the cursor field to write to.
    auto buffer = nb::cast<NativeNDBuffer*>(data);
    nb::dict res;
    res["buffer"] = buffer->storage();
    res["shape"] = buffer->shape().as_vector();
    res["strides"] = buffer->strides().as_vector();
    return res;
}

nb::object
NativeNDBufferMarshall::read_output(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data) const
{
    SGL_UNUSED(context);
    SGL_UNUSED(binding);
    return data;
}

Shape NativeNumpyMarshall::get_shape(nb::object data) const
{
    auto ndarray = nb::cast<nb::ndarray<nb::numpy>>(data);
    std::vector<int> shape_vec;
    for (size_t i = 0; i < ndarray.ndim(); i++) {
        shape_vec.push_back((int)ndarray.shape(i));
    }
    return Shape(shape_vec);
}

void NativeNumpyMarshall::write_shader_cursor_pre_dispatch(
    CallContext* context,
    NativeBoundVariableRuntime* binding,
    ShaderCursor cursor,
    nb::object value,
    nb::list read_back
) const
{
    auto ndarray = nb::cast<nb::ndarray<nb::numpy>>(value);
    SGL_CHECK(ndarray.dtype() == m_dtype, "numpy array dtype does not match the expected dtype");

    std::vector<int> shape_vec;
    for (size_t i = 0; i < ndarray.ndim(); i++) {
        shape_vec.push_back((int)ndarray.shape(i));
    }

    std::vector<int> vector_shape = binding->get_vector_type()->get_shape().as_vector();
    for (size_t i = 0; i < vector_shape.size(); i++) {
        int vs_size = vector_shape[vector_shape.size() - i - 1];
        int arr_size = shape_vec[shape_vec.size() - i - 1];

        SGL_CHECK(
            vs_size == arr_size,
            "numpy array shape dim {} does not match the expected shape ({} != {})",
            shape_vec.size() - i - 1,
            vs_size,
            arr_size
        );
    }

    Shape shape(shape_vec);

    SGL_UNUSED(binding);
    auto buffer = create_buffer(context->device(), shape);
    buffer_copy_from_numpy(buffer->storage().get(), ndarray);

    auto buffer_obj = nb::cast(buffer);
    store_readback(binding, read_back, value, buffer_obj);

    NativeNDBufferMarshall::write_shader_cursor_pre_dispatch(context, binding, cursor, buffer_obj, read_back);
}

void NativeNumpyMarshall::read_calldata(
    CallContext* context,
    NativeBoundVariableRuntime* binding,
    nb::object data,
    nb::object result
) const
{
    SGL_UNUSED(context);
    SGL_UNUSED(binding);

    auto ndarray = nb::cast<nb::ndarray<nb::numpy>>(data);
    auto buffer = nb::cast<NativeNDBuffer*>(result);

    size_t numpy_data_size = ndarray.nbytes();
    size_t buffer_data_size = buffer->storage()->size();
    SGL_CHECK(
        numpy_data_size == buffer_data_size,
        "numpy array size does not match the buffer ({} > {})",
        numpy_data_size,
        buffer_data_size
    );

    buffer->storage()->get_data(ndarray.data(), buffer_data_size);
}

nb::object NativeNumpyMarshall::create_output(CallContext* context, NativeBoundVariableRuntime* binding) const
{
    SGL_UNUSED(context);
    SGL_UNUSED(binding);

    Shape shape = context->call_shape() + binding->get_vector_type()->get_shape();

    size_t data_size = element_stride() * shape.element_count();
    void* data = new uint8_t[data_size];
    nb::capsule owner(data, [](void* p) noexcept { delete[] reinterpret_cast<uint8_t*>(p); });

    std::vector<size_t> s(0, shape.size());
    for (auto sd : shape.as_vector()) {
        s.push_back(sd);
    }

    auto ndarray = nb::ndarray<nb::numpy>(data, s.size(), s.data(), owner, nullptr, m_dtype);
    return nb::cast(ndarray);
}

nb::object NativeNumpyMarshall::create_dispatchdata(nb::object data) const
{
    SGL_THROW("Raw dispatch is not supported for numpy arrays.");
}


} // namespace sgl::slangpy

SGL_PY_EXPORT(utils_slangpy_buffer)
{
    using namespace sgl;
    using namespace sgl::slangpy;

    nb::module_ slangpy = m.attr("slangpy");

    nb::class_<NativeNDBufferDesc>(slangpy, "NativeNDBufferDesc")
        .def(nb::init<>())
        .def_rw("dtype", &NativeNDBufferDesc::dtype)
        .def_rw("element_layout", &NativeNDBufferDesc::element_layout)
        .def_rw("shape", &NativeNDBufferDesc::shape)
        .def_rw("strides", &NativeNDBufferDesc::strides)
        .def_rw("usage", &NativeNDBufferDesc::usage)
        .def_rw("memory_type", &NativeNDBufferDesc::memory_type);

    nb::class_<NativeNDBuffer, NativeObject>(slangpy, "NativeNDBuffer")
        .def(nb::init<ref<Device>, NativeNDBufferDesc>())
        .def_prop_ro("device", &NativeNDBuffer::device)
        .def_prop_ro("dtype", &NativeNDBuffer::dtype)
        .def_prop_ro("shape", &NativeNDBuffer::shape)
        .def_prop_ro("strides", &NativeNDBuffer::strides)
        .def_prop_ro("element_count", &NativeNDBuffer::element_count)
        .def_prop_ro("usage", &NativeNDBuffer::usage)
        .def_prop_ro("memory_type", &NativeNDBuffer::memory_type)
        .def_prop_ro("storage", &NativeNDBuffer::storage)
        .def("cursor", &NativeNDBuffer::cursor, "start"_a.none() = std::nullopt, "count"_a.none() = std::nullopt)
        .def("uniforms", &NativeNDBuffer::uniforms)
        .def(
            "to_numpy",
            [](NativeNDBuffer& self) { return buffer_to_numpy(self.storage().get()); },
            D_NA(NativeNDBuffer, buffer_to_numpy)
        )
        .def(
            "copy_from_numpy",
            [](NativeNDBuffer& self, nb::ndarray<nb::numpy> data)
            { buffer_copy_from_numpy(self.storage().get(), data); },
            "data"_a,
            D_NA(NativeNDBuffer, buffer_copy_from_numpy)
        );


    nb::class_<NativeNDBufferMarshall, NativeMarshall>(slangpy, "NativeNDBufferMarshall") //
        .def(
            "__init__",
            [](NativeNDBufferMarshall& self,
               int dims,
               bool writable,
               ref<NativeSlangType> slang_type,
               ref<NativeSlangType> slang_element_type,
               ref<TypeLayoutReflection> element_layout)
            { new (&self) NativeNDBufferMarshall(dims, writable, slang_type, slang_element_type, element_layout); },
            "dims"_a,
            "writable"_a,
            "slang_type"_a,
            "slang_element_type"_a,
            "element_layout"_a,
            D_NA(NativeNDBufferMarshall, NativeNDBufferMarshall)
        )
        .def_prop_ro("dims", &sgl::slangpy::NativeNDBufferMarshall::dims)
        .def_prop_ro("writable", &sgl::slangpy::NativeNDBufferMarshall::writable)
        .def_prop_ro("slang_element_type", &sgl::slangpy::NativeNDBufferMarshall::slang_element_type);

    nb::class_<NativeNumpyMarshall, NativeNDBufferMarshall>(slangpy, "NativeNumpyMarshall") //
        .def(
            "__init__",
            [](NativeNumpyMarshall& self,
               int dims,
               ref<NativeSlangType> slang_type,
               ref<NativeSlangType> slang_element_type,
               ref<TypeLayoutReflection> element_layout,
               nb::object numpydtype)
            {
                nb::dlpack::dtype dtype;

                int bytes = nb::cast<int>(numpydtype.attr("itemsize"));
                char kind = nb::cast<char>(numpydtype.attr("kind"));
                dtype.bits = (uint8_t)bytes * 8;
                dtype.lanes = 1;
                switch (kind) {
                case 'i':
                    dtype.code = (uint8_t)nb::dlpack::dtype_code::Int;
                    break;
                case 'u':
                    dtype.code = (uint8_t)nb::dlpack::dtype_code::UInt;
                    break;
                case 'f':
                    dtype.code = (uint8_t)nb::dlpack::dtype_code::Float;
                    break;
                case 'b':
                    dtype.code = (uint8_t)nb::dlpack::dtype_code::Bool;
                    break;
                default:
                    SGL_THROW("Unsupported numpy dtype kind '{}'", kind);
                    break;
                }
                new (&self) NativeNumpyMarshall(dims, slang_type, slang_element_type, element_layout, dtype);
            },
            "dims"_a,
            "slang_type"_a,
            "slang_element_type"_a,
            "element_layout"_a,
            "numpydtype"_a,
            D_NA(NativeNumpyMarshall, NativeNumpyMarshall)
        )
        .def_prop_ro("dtype", &sgl::slangpy::NativeNumpyMarshall::dtype);
}
