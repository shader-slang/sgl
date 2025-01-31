// SPDX-License-Identifier: Apache-2.0

#include <initializer_list>
#include "nanobind.h"

#include "sgl/device/device.h"
#include "sgl/device/buffer_cursor.h"
#include "sgl/device/command.h"

#include "sgl/utils/python/slangpytensor.h"

namespace sgl {

extern void write_shader_cursor(ShaderCursor& cursor, nb::object value);

} // namespace sgl

namespace sgl::slangpy {

inline std::optional<nb::dlpack::dtype> scalartype_to_dtype(TypeReflection::ScalarType scalar_type)
{
    switch (scalar_type) {
    case TypeReflection::ScalarType::none_:
        return {};
    case TypeReflection::ScalarType::void_:
        return {};
    case TypeReflection::ScalarType::bool_:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Bool, 8, 1};
    case TypeReflection::ScalarType::int32:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Int, 32, 1};
    case TypeReflection::ScalarType::uint32:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::UInt, 32, 1};
    case TypeReflection::ScalarType::int64:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Int, 64, 1};
    case TypeReflection::ScalarType::uint64:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::UInt, 64, 1};
    case TypeReflection::ScalarType::float16:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Float, 16, 1};
    case TypeReflection::ScalarType::float32:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Float, 32, 1};
    case TypeReflection::ScalarType::float64:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Float, 64, 1};
    case TypeReflection::ScalarType::int8:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Int, 8, 1};
    case TypeReflection::ScalarType::uint8:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::UInt, 8, 1};
    case TypeReflection::ScalarType::int16:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Int, 16, 1};
    case TypeReflection::ScalarType::uint16:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::UInt, 16, 1};
    case TypeReflection::ScalarType::intptr:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Int, sizeof(intptr_t) * 8, 1};
    case TypeReflection::ScalarType::uintptr:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::UInt, sizeof(uintptr_t) * 8, 1};
    default:
        return {};
    }
}

NativeTensor::NativeTensor(
    NativeTensorDesc desc,
    const ref<Buffer>& storage,
    const ref<NativeTensor>& grad_in,
    const ref<NativeTensor>& grad_out
)
    : m_desc(desc)
    , m_storage(storage)
    , m_grad_in(grad_in)
    , m_grad_out(grad_out)
{
    set_slangpy_signature(fmt::format("[{},{},{}]", desc.dtype->get_type_reflection()->name(), dims(), usage()));
}

ref<NativeTensor> NativeTensor::broadcast_to(const Shape& new_shape) const
{
    auto& curr_shape_vec = this->shape().as_vector();
    auto& new_shape_vec = new_shape.as_vector();

    int D = (int)new_shape_vec.size() - (int)curr_shape_vec.size();
    if (D < 0) {
        SGL_THROW("Broadcast shape must be larger than tensor shape");
    }

    for (size_t i = 0; i < curr_shape_vec.size(); ++i) {
        // TODO: Should this be curr_shape_vec[i] != 1? Waiting for Benedikt response.
        if (new_shape_vec[D + i] != curr_shape_vec[i] && new_shape_vec[D + i] != 1) {
            SGL_THROW(
                "New dimension {}({}) must be equal to current dimension {}({}) or 1",
                D + i,
                new_shape_vec[D + i],
                i,
                curr_shape_vec[i]
            );
        }
    }

    auto& curr_strides_vec = this->strides().as_vector();
    std::vector<int> new_strides(new_shape.size(), 0);
    for (size_t i = 0; i < curr_strides_vec.size(); ++i) {
        if (curr_shape_vec[i] > 1) {
            new_strides[D + i] = curr_strides_vec[i];
        }
    }

    NativeTensorDesc new_desc = m_desc;
    new_desc.shape = new_shape;
    new_desc.strides = Shape(new_strides);
    return make_ref<NativeTensor>(new_desc, m_storage, m_grad_in, m_grad_out);
}

void NativeTensor::clear(CommandBuffer* cmd)
{
    if (cmd) {
        cmd->clear_resource_view(m_storage->get_uav(), uint4(0, 0, 0, 0));
    } else {
        ref<CommandBuffer> temp_cmd = device()->create_command_buffer();
        temp_cmd->clear_resource_view(m_storage->get_uav(), uint4(0, 0, 0, 0));
        temp_cmd->submit();
    }
}

ref<NativeSlangType> innermost_type(ref<NativeSlangType> type)
{
    ref<NativeSlangType> result = type;
    while (true) {
        ref<NativeSlangType> child = result->element_type();
        if (!child || child == result) {
            break;
        }
        result = child;
    }
    return result;
}

ref<NativeTensor> NativeTensor::with_grads(ref<NativeTensor> grad_in, ref<NativeTensor> grad_out, bool zero) const
{
    ref<NativeTensor> new_grad_in = std::move(grad_in);
    ref<NativeTensor> new_grad_out = std::move(grad_out);

    // Create new, empty tensor for grads if none specified.
    if (!new_grad_in && !new_grad_out) {

        // Get the derivative type + buffer layout.
        ref<NativeSlangType> dtype = m_desc.dtype->derivative();
        if (!dtype)
            SGL_THROW("No derivative type found for {}", m_desc.dtype->get_type_reflection()->name());
        ref<TypeLayoutReflection> layout = dtype->buffer_type_layout();

        // Create a new structured buffer for storage.
        BufferDesc buffer_desc;
        buffer_desc.usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access | ResourceUsage::shared;
        buffer_desc.struct_size = layout->stride();
        buffer_desc.element_count = element_count();
        ref<Buffer> buffer = device()->create_buffer(buffer_desc);

        // Create new native tensor to hold the grads.
        new_grad_in = make_ref<NativeTensor>(
            NativeTensorDesc{
                .dtype = dtype,
                .element_layout = layout,
                .shape = m_desc.shape,
                .strides = m_desc.shape.calc_contiguous_strides(),
                .offset = m_desc.offset},
            buffer,
            nullptr,
            nullptr
        );
        new_grad_out = new_grad_in;
    }

    // Create a new tensor object that refers to the same data as this one, but with
    // associated grads.
    ref<NativeTensor> result = make_ref<NativeTensor>(m_desc, m_storage, new_grad_in, new_grad_out);

    // Optionally clear both.
    if (zero) {
        if (new_grad_in) {
            new_grad_in->clear();
        }
        if (new_grad_out && new_grad_out != new_grad_in) {
            new_grad_out->clear();
        }
    }

    return result;
}

nb::ndarray<nb::numpy> NativeTensor::to_numpy() const
{
    // Get dlpack type from scalar type.
    size_t dtype_size = dtype()->buffer_type_layout()->stride();
    ref<NativeSlangType> innermost = innermost_type(dtype());
    ref<TypeLayoutReflection> innermost_layout = innermost->buffer_type_layout();
    size_t innermost_size = innermost_layout->stride();
    TypeReflection::ScalarType scalar_type = innermost_layout->type()->scalar_type();
    auto dlpack_type = scalartype_to_dtype(scalar_type);
    auto dtype_shape = dtype()->get_shape();
    auto dtype_strides = dtype_shape.calc_contiguous_strides();

    // Create data and nanobind capsule to contain the data.
    size_t data_size = m_storage->size();
    void* data = new uint8_t[data_size];
    m_storage->get_data(data, data_size);
    nb::capsule owner(data, [](void* p) noexcept { delete[] reinterpret_cast<uint8_t*>(p); });

    // Build sizes/strides arrays in form numpy wants them.
    std::vector<size_t> sizes;
    std::vector<int64_t> strides;

    for (size_t i = 0; i < m_desc.shape.size(); ++i) {
        sizes.push_back(m_desc.shape[i]);
        strides.push_back(m_desc.strides[i] * dtype_size / innermost_size);
    }
    for (size_t i = 0; i < dtype_shape.size(); ++i) {
        sizes.push_back(dtype_shape[i]);
        strides.push_back(dtype_strides[i]);
    }

    // Return numpy array.
    return nb::ndarray<
        nb::numpy>(data, sizes.size(), sizes.data(), owner, strides.data(), *dlpack_type, nb::device::cpu::value);
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
    // The native tensor marshall can be inherited for other types, so
    // we check if we have a native tensor, and if not revert to the
    // base class implementation.
    NativeTensor* primal;
    if (nb::try_cast(value, primal)) {
        ShaderCursor field = cursor[binding->get_variable_name()];

        const ref<NativeTensor>& grad_in = primal->grad_in();
        const ref<NativeTensor>& grad_out = primal->grad_out();

        if (!has_derivative()) {
            write_shader_cursor_fields(context, binding, field, primal, read_back);
        } else {
            write_shader_cursor_fields(context, binding, field["primal"], primal, read_back);
            if (m_d_in) {
                SGL_CHECK(grad_in, "Missing required input gradients");
                write_shader_cursor_fields(context, binding, field["d_in"], grad_in.get(), read_back);
            }
            if (m_d_out) {
                SGL_CHECK(grad_out, "Missing required input gradients");
                write_shader_cursor_fields(context, binding, field["d_out"], grad_out.get(), read_back);
            }
        }

        if (context->call_mode() != CallMode::prim && grad_in && grad_in == grad_out) {
            if (binding->get_access().second == AccessType::readwrite)
                SGL_THROW(
                    "inout parameter gradients need separate buffers for inputs and outputs (see Tensor.with_grads)"
                );
        }
    } else {
        NativeMarshall::write_shader_cursor_pre_dispatch(context, binding, cursor, value, read_back);
    }
}

void NativeTensorMarshall::write_shader_cursor_fields(
    CallContext* context,
    NativeBoundVariableRuntime* binding,
    ShaderCursor field,
    NativeTensor* buffer,
    nb::list read_back
) const
{
    SGL_UNUSED(read_back);

    // Write the buffer storage.
    field["buffer"] = buffer->storage();

    // Write shape vector as an array of ints.
    const std::vector<int>& shape_vec = buffer->shape().as_vector();
    field["_shape"]._set_array_unsafe(&shape_vec[0], shape_vec.size() * 4, shape_vec.size());

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
    auto layout_field = field["layout"];
    layout_field["strides"]._set_array_unsafe(&strides_vec[0], strides_vec.size() * 4, strides_vec.size());
    layout_field["offset"] = buffer->offset();
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
    SGL_UNUSED(binding);

    // Get type, buffer layout and shape.
    ref<NativeSlangType> dtype = m_slang_element_type;
    ref<TypeLayoutReflection> layout = m_element_layout;
    auto& shape = context->call_shape();

    // Create a new structured buffer for storage.
    BufferDesc buffer_desc;
    buffer_desc.usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access | ResourceUsage::shared;
    buffer_desc.struct_size = layout->stride();
    buffer_desc.element_count = shape.element_count();
    ref<Buffer> buffer = context->device()->create_buffer(buffer_desc);

    // Create new native tensor to hold the grads.
    return nb::cast(make_ref<NativeTensor>(
        NativeTensorDesc{
            .dtype = dtype,
            .element_layout = layout,
            .shape = shape,
            .strides = shape.calc_contiguous_strides(),
            .offset = 0},
        buffer,
        nullptr,
        nullptr
    ));
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
    res["_shape"] = buffer->shape().as_vector();
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
        .def_rw("offset", &NativeTensorDesc::offset);

    nb::class_<NativeTensor, NativeObject>(slangpy, "NativeTensor")
        .def(
            nb::init<NativeTensorDesc, const ref<Buffer>&, const ref<NativeTensor>&, const ref<NativeTensor>&>(),
            "desc"_a,
            "storage"_a,
            "grad_in"_a.none(),
            "grad_out"_a.none()
        )
        .def_prop_ro("device", &NativeTensor::device)
        .def_prop_ro("dtype", &NativeTensor::dtype)
        .def_prop_ro("shape", &NativeTensor::shape)
        .def_prop_ro("strides", &NativeTensor::strides)
        .def_prop_ro("offset", &NativeTensor::offset)
        .def_prop_ro("element_count", &NativeTensor::element_count)
        .def_prop_ro("usage", &NativeTensor::usage)
        .def_prop_ro("memory_type", &NativeTensor::memory_type)
        .def_prop_ro("storage", &NativeTensor::storage)
        .def_prop_rw("grad_in", &NativeTensor::grad_in, &NativeTensor::set_grad_in, nb::none())
        .def_prop_rw("grad_out", &NativeTensor::grad_out, &NativeTensor::set_grad_out, nb::none())
        .def_prop_ro("grad", &NativeTensor::grad)
        .def("broadcast_to", &NativeTensor::broadcast_to, "shape"_a)
        .def("clear", &NativeTensor::clear, "cmd"_a.none() = nullptr)
        .def(
            "with_grads",
            &NativeTensor::with_grads,
            "grad_in"_a.none() = nullptr,
            "grad_out"_a.none() = nullptr,
            "zero"_a = false
        )
        .def("cursor", &NativeTensor::cursor, "start"_a.none() = std::nullopt, "count"_a.none() = std::nullopt)
        .def("uniforms", &NativeTensor::uniforms)
        .def("to_numpy", &NativeTensor::to_numpy, D_NA(NativeTensor, to_numpy));


    nb::class_<NativeTensorMarshall, PyNativeTensorMarshall, NativeMarshall>(slangpy, "NativeTensorMarshall") //
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
                    PyNativeTensorMarshall(dims, writable, slang_type, slang_element_type, element_layout, d_in, d_out);
            },
            "dims"_a,
            "writable"_a,
            "slang_type"_a,
            "slang_element_type"_a,
            "element_layout"_a,
            "d_in"_a.none(),
            "d_out"_a.none(),
            D_NA(NativeTensorMarshall, NativeTensorMarshall)
        )
        .def_prop_ro("dims", &sgl::slangpy::NativeTensorMarshall::dims)
        .def_prop_ro("writable", &sgl::slangpy::NativeTensorMarshall::writable)
        .def_prop_ro("slang_element_type", &sgl::slangpy::NativeTensorMarshall::slang_element_type)
        .def_prop_ro("d_in", &NativeTensorMarshall::d_in)
        .def_prop_ro("d_out", &NativeTensorMarshall::d_out);
}
