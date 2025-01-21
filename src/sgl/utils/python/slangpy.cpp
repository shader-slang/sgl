// SPDX-License-Identifier: Apache-2.0

#include <sstream>

#include "nanobind.h"

#include "sgl/core/macros.h"
#include "sgl/core/logger.h"
#include "sgl/utils/slangpy.h"
#include "sgl/device/device.h"
#include "sgl/device/kernel.h"
#include "sgl/device/command.h"

#include "sgl/utils/python/slangpy.h"
#include "sgl/utils/python/slangpyvalue.h"
#include "sgl/utils/python/slangpybuffer.h"

namespace sgl {
extern void write_shader_cursor(ShaderCursor& cursor, nb::object value);
}

namespace sgl::slangpy {

void NativeMarshall::write_shader_cursor_pre_dispatch(
    CallContext* context,
    NativeBoundVariableRuntime* binding,
    ShaderCursor cursor,
    nb::object value,
    nb::list read_back
) const
{
    // We are a leaf node, so generate and store call data for this node.
    nb::object cd_val = create_calldata(context, binding, value);
    if (!cd_val.is_none()) {
        ShaderCursor child_field = cursor[binding->get_variable_name()];
        write_shader_cursor(child_field, cd_val);
        read_back.append(nb::make_tuple(binding, value, cd_val));
    }
}

void NativeBoundVariableRuntime::populate_call_shape(std::vector<int>& call_shape, nb::object value)
{
    if (m_children) {
        // We have children, so load each child value and recurse down the tree.
        for (const auto& [name, child_ref] : *m_children) {
            if (child_ref) {
                nb::object child_value = value[name.c_str()];
                child_ref->populate_call_shape(call_shape, child_value);
            }
        }
    } else if (!value.is_none()) {
        // We are a leaf node, so we can populate the call shape.
        if (!m_transform.valid()) {
            throw NativeBoundVariableException("Transform shape is not set.", ref(this));
        }

        // Read the transform and call shape size.
        auto tf = m_transform.as_vector();
        size_t csl = call_shape.size();

        // Get the shape of the value. In the case of none-concrete types,
        // only the container shape is needed, as we never map elements.
        if (m_python_type->get_concrete_shape().valid())
            m_shape = m_python_type->get_concrete_shape();
        else
            m_shape = m_python_type->get_shape(value);

        // Apply this shape to the overall call shape.
        auto shape = m_shape.as_vector();
        for (size_t i = 0; i < tf.size(); ++i) {
            int shape_dim = shape[i];
            int call_idx = tf[i];

            // If the call index loaded from the transform is
            // out of bounds, this dimension is a sub-element index,
            // so ignore it.
            if (call_idx >= static_cast<int>(csl)) {
                continue;
            }

            // Apply the new dimension to the call shape.
            //- if it's the same, we're fine
            //- if current call shape == 1, shape_dim != 1, call is expanded
            //- if current call shape != 1, shape_dim == 1, shape is broadcast
            //- if current call shape != 1, shape_dim != 1, it's a mismatch
            int& cs = call_shape[call_idx];
            if (cs != shape_dim) {
                if (cs != 1 && shape_dim != 1) {
                    throw NativeBoundVariableException(
                        "Shape mismatch for " + m_variable_name + " between input and output",
                        ref(this)
                    );
                }
                if (shape_dim != 1) {
                    cs = shape_dim;
                }
            }
        }
    }
}

void NativeBoundVariableRuntime::write_call_data_pre_dispatch(
    CallContext* context,
    nb::dict call_data,
    nb::object value
)
{
    if (m_children) {
        // We have children, so generate call data for each child and
        // store in a dictionary, then store the dictionary as the call data.
        nb::dict cd_val;
        for (const auto& [name, child_ref] : *m_children) {
            if (child_ref) {
                nb::object child_value = value[name.c_str()];
                child_ref->write_call_data_pre_dispatch(context, cd_val, child_value);
            }
        }
        if (cd_val.size() > 0) {
            call_data[m_variable_name.c_str()] = cd_val;
        }
    } else {
        // We are a leaf node, so generate and store call data for this node.
        nb::object cd_val = m_python_type->create_calldata(context, this, value);
        if (!cd_val.is_none()) {
            call_data[m_variable_name.c_str()] = cd_val;
        }
    }
}

void NativeBoundVariableRuntime::write_shader_cursor_pre_dispatch(
    CallContext* context,
    ShaderCursor cursor,
    nb::object value,
    nb::list read_back
)
{
    if (m_children) {
        // We have children, so generate call data for each child and
        // store in a dictionary, then store the dictionary as the call data.
        ShaderCursor child_field = cursor[m_variable_name.c_str()];
        for (const auto& [name, child_ref] : *m_children) {
            if (child_ref) {
                nb::object child_value = value[name.c_str()];
                child_ref->write_shader_cursor_pre_dispatch(context, child_field, child_value, read_back);
            }
        }
    } else {
        // We are a leaf node, so generate and store call data for this node.
        m_python_type->write_shader_cursor_pre_dispatch(context, this, cursor, value, read_back);
    }
}

void NativeBoundVariableRuntime::read_call_data_post_dispatch(
    CallContext* context,
    nb::dict call_data,
    nb::object value
)
{
    // Bail if the call data does not contain the variable name.
    if (!call_data.contains(m_variable_name.c_str())) {
        return;
    }

    // Get the call data value.
    auto cd_val = call_data[m_variable_name.c_str()];
    if (m_children) {
        // We have children, so the call data value should be a dictionary
        // containing the call data for each child.
        auto dict = nb::cast<nb::dict>(cd_val);
        for (const auto& [name, child_ref] : *m_children) {
            if (child_ref) {
                nb::object child_value = value[name.c_str()];
                child_ref->read_call_data_post_dispatch(context, dict, child_value);
            }
        }
    } else {
        // We are a leaf node, so the read call data.
        m_python_type->read_calldata(context, this, value, cd_val);
    }
}

void NativeBoundVariableRuntime::write_raw_dispatch_data(nb::dict call_data, nb::object value)
{
    if (m_children) {
        // We have children, so generate call data for each child and
        // store in a dictionary, then store the dictionary as the call data.
        nb::dict cd_val;
        for (const auto& [name, child_ref] : *m_children) {
            if (child_ref) {
                nb::object child_value = value[name.c_str()];
                child_ref->write_raw_dispatch_data(cd_val, child_value);
            }
        }
        if (cd_val.size() > 0) {
            call_data[m_variable_name.c_str()] = cd_val;
        }
    } else {
        // We are a leaf node, so generate and store call data for this node.
        nb::object cd_val = m_python_type->create_dispatchdata(value);
        if (!cd_val.is_none()) {
            call_data[m_variable_name.c_str()] = cd_val;
        }
    }
}

nb::object NativeBoundVariableRuntime::read_output(CallContext* context, nb::object data)
{
    if (m_children) {
        // We have children, so read the output for each child and store in a dictionary.
        nb::dict res;
        for (const auto& [name, child_ref] : *m_children) {
            if (res.contains(name.c_str())) {
                if (child_ref) {
                    nb::object child_data = data[child_ref->m_variable_name.c_str()];
                    res[name.c_str()] = child_ref->read_output(context, child_data);
                }
            }
        }
        return res;
    } else {
        // We are a leaf node, so read the output if the variable was writable.
        if (m_access.first == AccessType::write || m_access.first == AccessType::readwrite) {
            return m_python_type->read_output(context, this, data);
        }
        return nb::none();
    }
}

Shape NativeBoundCallRuntime::calculate_call_shape(int call_dimensionality, nb::list args, nb::dict kwargs)
{
    // Setup initial call shape of correct dimensionality, with all dimensions set to 1.
    std::vector<int> call_shape(call_dimensionality, 1);

    // Populate call shape for each positional argument.
    for (size_t idx = 0; idx < args.size(); ++idx) {
        m_args[idx]->populate_call_shape(call_shape, args[idx]);
    }

    // Populate call shape for each keyword argument.
    for (auto [key, value] : kwargs) {
        auto it = m_kwargs.find(nb::str(key).c_str());
        if (it != m_kwargs.end()) {
            it->second->populate_call_shape(call_shape, nb::cast<nb::object>(value));
        }
    }

    // Return finalized shape.
    return Shape(call_shape);
}

void NativeBoundCallRuntime::write_calldata_pre_dispatch(
    CallContext* context,
    nb::dict call_data,
    nb::list args,
    nb::dict kwargs
)
{
    // Write call data for each positional argument.
    for (size_t idx = 0; idx < args.size(); ++idx) {
        m_args[idx]->write_call_data_pre_dispatch(context, call_data, args[idx]);
    }

    // Write call data for each keyword argument.
    for (auto [key, value] : kwargs) {
        auto it = m_kwargs.find(nb::str(key).c_str());
        if (it != m_kwargs.end()) {
            it->second->write_call_data_pre_dispatch(context, call_data, nb::cast<nb::object>(value));
        }
    }
}

void NativeBoundCallRuntime::write_shader_cursor_pre_dispatch(
    CallContext* context,
    ShaderCursor cursor,
    nb::list args,
    nb::dict kwargs,
    nb::list read_back

)
{
    // Write call data for each positional argument.
    for (size_t idx = 0; idx < args.size(); ++idx) {
        m_args[idx]->write_shader_cursor_pre_dispatch(context, cursor, args[idx], read_back);
    }

    // Write call data for each keyword argument.
    for (auto [key, value] : kwargs) {
        auto it = m_kwargs.find(nb::str(key).c_str());
        if (it != m_kwargs.end()) {
            it->second->write_shader_cursor_pre_dispatch(context, cursor, nb::cast<nb::object>(value), read_back);
        }
    }
}


void NativeBoundCallRuntime::read_call_data_post_dispatch(
    CallContext* context,
    nb::dict call_data,
    nb::list args,
    nb::dict kwargs
)
{
    // Read call data for each positional argument.
    for (size_t idx = 0; idx < args.size(); ++idx) {
        m_args[idx]->read_call_data_post_dispatch(context, call_data, args[idx]);
    }

    // Read call data for each keyword argument.
    for (auto [key, value] : kwargs) {
        auto it = m_kwargs.find(nb::str(key).c_str());
        if (it != m_kwargs.end()) {
            it->second->read_call_data_post_dispatch(context, call_data, nb::cast<nb::object>(value));
        }
    }
}

void NativeBoundCallRuntime::write_raw_dispatch_data(nb::dict call_data, nb::dict kwargs)
{
    // Write call data for each keyword argument.
    for (auto [key, value] : kwargs) {
        auto it = m_kwargs.find(nb::str(key).c_str());
        if (it != m_kwargs.end()) {
            it->second->write_raw_dispatch_data(call_data, nb::cast<nb::object>(value));
        }
    }
}

nb::object NativeCallData::call(ref<NativeCallRuntimeOptions> opts, nb::args args, nb::kwargs kwargs)
{
    return exec(opts, nullptr, args, kwargs);
}

nb::object NativeCallData::append_to(
    ref<NativeCallRuntimeOptions> opts,
    ref<CommandBuffer> command_buffer,
    nb::args args,
    nb::kwargs kwargs
)
{
    return exec(opts, command_buffer.get(), args, kwargs);
}

nb::object NativeCallData::exec(
    ref<NativeCallRuntimeOptions> opts,
    CommandBuffer* command_buffer,
    nb::args args,
    nb::kwargs kwargs
)
{
    // Unpack args and kwargs.
    nb::list unpacked_args = unpack_args(args);
    nb::dict unpacked_kwargs = unpack_kwargs(kwargs);

    // Calculate call shape.
    Shape call_shape = m_runtime->calculate_call_shape(m_call_dimensionality, unpacked_args, unpacked_kwargs);
    m_last_call_shape = call_shape;

    // Setup context.
    auto context = make_ref<CallContext>(m_device, call_shape, m_call_mode);

    // Allocate return value if needed.
    if (!command_buffer && m_call_mode == CallMode::prim) {
        ref<NativeBoundVariableRuntime> rv_node = m_runtime->find_kwarg("_result");
        if (rv_node && (!kwargs.contains("_result") || kwargs["_result"].is_none())) {
            nb::object output = rv_node->get_python_type()->create_output(context, rv_node.get());
            kwargs["_result"] = output;
            unpacked_kwargs["_result"] = output;
            rv_node->populate_call_shape(call_shape.as_vector(), output);
        }
    }

    // Write uniforms to call data.
    // nb::dict call_data;
    // m_runtime->write_calldata_pre_dispatch(context, call_data, unpacked_args, unpacked_kwargs);

    // Calculate total threads and strides.
    int total_threads = 1;
    std::vector<int> strides;
    auto cs = call_shape.as_vector();
    for (auto it = cs.rbegin(); it != cs.rend(); ++it) {
        strides.push_back(total_threads);
        total_threads *= *it;
    }
    std::reverse(strides.begin(), strides.end());


    // Copy user provided vars and insert call data.

    // vars["call_data"] = call_data;

    nb::list read_back;

    // Dispatch the kernel.
    auto bind_vars = [&](ShaderCursor cursor)
    {
        auto call_data_cursor = cursor.find_field("call_data");

        if (!strides.empty()) {
            call_data_cursor["_call_stride"]
                ._set_array(&strides[0], strides.size() * 4, TypeReflection::ScalarType::int32, strides.size());
            call_data_cursor["_call_dim"]
                ._set_array(&cs[0], cs.size() * 4, TypeReflection::ScalarType::int32, cs.size());
        }
        call_data_cursor["_thread_count"] = uint3(total_threads, 1, 1);

        m_runtime
            ->write_shader_cursor_pre_dispatch(context, call_data_cursor, unpacked_args, unpacked_kwargs, read_back);

        nb::list uniforms = opts->get_uniforms();
        if (uniforms) {
            for (auto u : uniforms) {
                if (nb::isinstance<nb::dict>(u)) {
                    write_shader_cursor(cursor, nb::cast<nb::dict>(u));
                } else {
                    write_shader_cursor(cursor, nb::cast<nb::dict>(u(this)));
                }
            }
        }
    };
    m_kernel->dispatch(uint3(total_threads, 1, 1), bind_vars, command_buffer);

    // If command_buffer is not null, return early.
    if (command_buffer != nullptr) {
        return nanobind::none();
    }

    // Read call data post dispatch.
    // m_runtime->read_call_data_post_dispatch(context, call_data, unpacked_args, unpacked_kwargs);
    for (auto val : read_back) {
        auto t = nb::cast<nb::tuple>(val);
        auto bvr = nb::cast<ref<NativeBoundVariableRuntime>>(t[0]);
        auto rb_val = t[1];
        auto rb_data = t[2];
        bvr->get_python_type()->read_calldata(context, bvr.get(), rb_val, rb_data);
    }

    // Pack updated 'this' values back.
    for (size_t i = 0; i < args.size(); ++i) {
        pack_arg(args[i], unpacked_args[i]);
    }
    for (auto [k, v] : kwargs) {
        pack_arg(nb::cast<nb::object>(v), unpacked_kwargs[k]);
    }

    // Handle return value based on call mode.
    if (m_call_mode == CallMode::prim) {
        auto rv_node_it = m_runtime->find_kwarg("_result");
        if (rv_node_it && !unpacked_kwargs["_result"].is_none()) {
            return rv_node_it->read_output(context, unpacked_kwargs["_result"]);
        }
    }
    return nb::none();
}

nb::list unpack_args(nb::args args)
{
    nb::list unpacked;
    for (auto arg : args) {
        unpacked.append(unpack_arg(nb::cast<nb::object>(arg)));
    }
    return unpacked;
}

nb::dict unpack_kwargs(nb::kwargs kwargs)
{
    nb::dict unpacked;
    for (const auto& [k, v] : kwargs) {
        unpacked[k] = unpack_arg(nb::cast<nb::object>(v));
    }
    return unpacked;
}

nb::object unpack_arg(nb::object arg)
{
    auto obj = arg;

    // If object has 'get_this', read it.
    if (nb::hasattr(obj, "get_this")) {
        obj = nb::getattr(obj, "get_this")();
    }

    // Recursively unpack dictionaries.
    nb::dict d;
    if (nb::try_cast(obj, d)) {
        nb::dict res;
        for (auto [k, v] : d) {
            res[k] = unpack_arg(nb::cast<nb::object>(v));
        }
        obj = res;
    }

    // Recursively unpack lists.
    nb::list l;
    if (nb::try_cast(obj, l)) {
        nb::list res;
        for (auto v : l) {
            res.append(unpack_arg(nb::cast<nb::object>(v)));
        }
        obj = res;
    }

    // Return unpacked object.
    return obj;
}

void pack_arg(nanobind::object arg, nanobind::object unpacked_arg)
{
    // If object has 'update_this', update it.
    if (nb::hasattr(arg, "update_this")) {
        nb::getattr(arg, "update_this")(unpacked_arg);
    }

    // Recursively pack dictionaries.
    nb::dict d;
    if (nb::try_cast(arg, d)) {
        for (auto [k, v] : d) {
            pack_arg(nb::cast<nb::object>(v), unpacked_arg[k]);
        }
    }

    // Recursively pack lists.
    nb::list l;
    if (nb::try_cast(arg, l)) {
        for (size_t i = 0; i < l.size(); ++i) {
            pack_arg(l[i], unpacked_arg[i]);
        }
    }
}

void _get_value_signature(
    const std::function<std::string(nb::handle)>& value_to_id,
    nb::handle o,
    std::stringstream& stream
)
{
    // Add type name.
    auto type_name = nb::str(nb::getattr(o.type(), "__name__"));
    stream << type_name.c_str() << "\n";

    // Handle objects with get_this method.
    auto get_this = nb::getattr(o, "get_this", nb::none());
    if (!get_this.is_none()) {
        auto this_ = get_this();
        _get_value_signature(value_to_id, this_, stream);
        return;
    }

    // If x has signature attribute, use it.
    if (nb::hasattr(o, "slangpy_signature")) {

        auto slangpy_sig = nb::getattr(o, "slangpy_signature");
        stream << nb::str(slangpy_sig).c_str() << "\n";
        return;
    }

    // If x is a dictionary get signature of its children.
    nb::dict dict;
    if (nb::try_cast(o, dict)) {
        stream << "\n";
        for (const auto& [k, v] : dict) {
            stream << nb::str(k).c_str() << ":";
            _get_value_signature(value_to_id, v, stream);
        }
        return;
    }

    // Use value_to_id function.
    std::string s = value_to_id(o);
    if (!s.empty()) {
        stream << s;
    }
    stream << "\n";
}

void hash_signature(
    const std::function<std::string(nb::handle)>& value_to_id,
    nb::args args,
    nb::kwargs kwargs,
    std::stringstream& stream
)
{
    stream << "args\n";
    for (const auto& arg : args) {
        stream << "N:";
        _get_value_signature(value_to_id, arg, stream);
    }

    stream << "kwargs\n";
    for (const auto& [k, v] : kwargs) {
        stream << nb::str(k).c_str() << ":";
        _get_value_signature(value_to_id, v, stream);
    }
}

} // namespace sgl::slangpy

SGL_PY_EXPORT(utils_slangpy)
{
    using namespace sgl;
    using namespace sgl::slangpy;

    nb::module_ slangpy = m.attr("slangpy");

    nb::sgl_enum<AccessType>(slangpy, "AccessType");
    nb::sgl_enum<CallMode>(slangpy, "CallMode");

    slangpy.def(
        "hash_signature",
        [](const std::function<std::string(nb::handle)>& value_to_id, nb::args args, nb::kwargs kwargs)
        {
            std::stringstream stream;
            hash_signature(value_to_id, args, kwargs, stream);
            return stream.str();
        },
        "value_to_id"_a,
        "args"_a,
        "kwargs"_a,
        D_NA(slangpy, hash_signature)
    );

    slangpy.def(
        "unpack_args",
        [](nb::args args) { return unpack_args(args); },
        "args"_a,
        D_NA(slangpy, unpack_args)
    );
    slangpy.def(
        "unpack_kwargs",
        [](nb::kwargs kwargs) { return unpack_kwargs(kwargs); },
        "kwargs"_a,
        D_NA(slangpy, unpack_kwargs)
    );
    slangpy.def(
        "unpack_arg",
        [](nb::object arg) { return unpack_arg(arg); },
        "arg"_a,
        D_NA(slangpy, unpack_arg)
    );
    slangpy.def(
        "pack_arg",
        [](nb::object arg, nb::object unpacked_arg) { pack_arg(arg, unpacked_arg); },
        "arg"_a,
        "unpacked_arg"_a,
        D_NA(slangpy, pack_arg)
    );

    nb::register_exception_translator(
        [](const std::exception_ptr& p, void* /* unused */)
        {
            try {
                std::rethrow_exception(p);
            } catch (const NativeBoundVariableException& e) {
                nb::dict data;
                data["message"] = e.message();
                data["source"] = e.source();
                PyErr_SetObject(PyExc_ValueError, data.ptr());
            }
        }
    );

    nb::class_<NativeSlangType, Object>(slangpy, "NativeSlangType") //
        .def(nb::init<>(), D_NA(NativeSlangType, NativeSlangType))
        .def_prop_rw(
            "type_reflection",
            &NativeSlangType::get_type_reflection,
            &NativeSlangType::set_type_reflection,
            D_NA(NativeSlangType, type_reflection)
        );

    nb::class_<NativeMarshall, PyNativeMarshall, Object>(slangpy, "NativeMarshall") //
        .def(
            "__init__",
            [](NativeMarshall& self) { new (&self) PyNativeMarshall(); },
            D_NA(NativeMarshall, NativeMarshall)
        )

        .def_prop_rw(
            "concrete_shape",
            &NativeMarshall::get_concrete_shape,
            &NativeMarshall::set_concrete_shape,
            D_NA(NativeMarshall, concrete_shape)
        )
        .def_prop_rw(
            "slang_type",
            &NativeMarshall::get_slang_type,
            &NativeMarshall::set_slang_type,
            D_NA(NativeMarshall, slang_type)
        )
        .def(
            "write_shader_cursor_pre_dispatch",
            &NativeMarshall::write_shader_cursor_pre_dispatch,
            "context"_a,
            "binding"_a,
            "cursor"_a,
            "value"_a,
            "read_back"_a,
            D_NA(NativeMarshall, write_shader_cursor_pre_dispatch)
        )
        .def("create_calldata", &NativeMarshall::create_calldata, D_NA(NativeMarshall, create_calldata))
        .def("read_calldata", &NativeMarshall::read_calldata, D_NA(NativeMarshall, read_calldata))
        .def("create_output", &NativeMarshall::create_output, D_NA(NativeMarshall, create_output))
        .def("read_output", &NativeMarshall::read_output, D_NA(NativeMarshall, read_output));

    nb::class_<NativeValueMarshall, NativeMarshall>(slangpy, "NativeValueMarshall") //
        .def(
            "__init__",
            [](NativeValueMarshall& self) { new (&self) NativeValueMarshall(); },
            D_NA(NativeValueMarshall, NativeValueMarshall)
        );

    nb::class_<NativeBoundVariableRuntime, Object>(slangpy, "NativeBoundVariableRuntime") //
        .def(nb::init<>(), D_NA(NativeBoundVariableRuntime, NativeBoundVariableRuntime))
        .def_prop_rw(
            "access",
            &NativeBoundVariableRuntime::get_access,
            &NativeBoundVariableRuntime::set_access,
            D_NA(NativeBoundVariableRuntime, access)
        )
        .def_prop_rw(
            "transform",
            &NativeBoundVariableRuntime::get_transform,
            &NativeBoundVariableRuntime::set_transform,
            D_NA(NativeBoundVariableRuntime, transform)
        )
        .def_prop_rw(
            "python_type",
            &NativeBoundVariableRuntime::get_python_type,
            &NativeBoundVariableRuntime::set_python_type,
            D_NA(NativeBoundVariableRuntime, python_type)
        )
        .def_prop_rw(
            "vector_type",
            &NativeBoundVariableRuntime::get_vector_type,
            &NativeBoundVariableRuntime::set_vector_type,
            D_NA(NativeBoundVariableRuntime, vector_type)
        )
        .def_prop_rw(
            "shape",
            &NativeBoundVariableRuntime::get_shape,
            &NativeBoundVariableRuntime::set_shape,
            D_NA(NativeBoundVariableRuntime, shape)
        )
        .def_prop_rw(
            "variable_name",
            &NativeBoundVariableRuntime::get_variable_name,
            &NativeBoundVariableRuntime::set_variable_name,
            D_NA(NativeBoundVariableRuntime, variable_name)
        )
        .def_prop_rw(
            "children",
            &NativeBoundVariableRuntime::get_children,
            &NativeBoundVariableRuntime::set_children,
            D_NA(NativeBoundVariableRuntime, children)
        )
        .def(
            "populate_call_shape",
            &NativeBoundVariableRuntime::populate_call_shape,
            D_NA(NativeBoundVariableRuntime, populate_call_shape)
        )
        .def(
            "write_call_data_pre_dispatch",
            &NativeBoundVariableRuntime::write_call_data_pre_dispatch,
            D_NA(NativeBoundVariableRuntime, write_call_data_pre_dispatch)
        )
        .def(
            "read_call_data_post_dispatch",
            &NativeBoundVariableRuntime::read_call_data_post_dispatch,
            D_NA(NativeBoundVariableRuntime, read_call_data_post_dispatch)
        )
        .def(
            "write_raw_dispatch_data",
            &NativeBoundVariableRuntime::write_raw_dispatch_data,
            D_NA(NativeBoundVariableRuntime, write_raw_dispatch_data)
        )
        .def("read_output", &NativeBoundVariableRuntime::read_output, D_NA(NativeBoundVariableRuntime, read_output));

    nb::class_<NativeBoundCallRuntime, Object>(slangpy, "NativeBoundCallRuntime") //
        .def(nb::init<>(), D_NA(NativeBoundCallRuntime, NativeBoundCallRuntime))
        .def_prop_rw(
            "args",
            &NativeBoundCallRuntime::get_args,
            &NativeBoundCallRuntime::set_args,
            D_NA(NativeBoundCallRuntime, args)
        )
        .def_prop_rw(
            "kwargs",
            &NativeBoundCallRuntime::get_kwargs,
            &NativeBoundCallRuntime::set_kwargs,
            D_NA(NativeBoundCallRuntime, kwargs)
        )
        .def("find_kwarg", &NativeBoundCallRuntime::find_kwarg, D_NA(NativeBoundCallRuntime, find_kwarg))
        .def(
            "calculate_call_shape",
            &NativeBoundCallRuntime::calculate_call_shape,
            D_NA(NativeBoundCallRuntime, calculate_call_shape)
        )
        .def(
            "write_calldata_pre_dispatch",
            &NativeBoundCallRuntime::write_calldata_pre_dispatch,
            D_NA(NativeBoundCallRuntime, write_calldata_pre_dispatch)
        )
        .def(
            "read_call_data_post_dispatch",
            &NativeBoundCallRuntime::read_call_data_post_dispatch,
            D_NA(NativeBoundCallRuntime, read_call_data_post_dispatch)
        )
        .def(
            "write_raw_dispatch_data",
            &NativeBoundCallRuntime::write_raw_dispatch_data,
            D_NA(NativeBoundCallRuntime, write_raw_dispatch_data)
        );

    nb::class_<NativeCallRuntimeOptions, Object>(slangpy, "NativeCallRuntimeOptions") //
        .def(nb::init<>(), D_NA(NativeCallRuntimeOptions, NativeCallRuntimeOptions))
        .def_prop_rw(
            "uniforms",
            &NativeCallRuntimeOptions::get_uniforms,
            &NativeCallRuntimeOptions::set_uniforms,
            D_NA(NativeCallRuntimeOptions, uniforms)
        );

    nb::class_<NativeCallData, Object>(slangpy, "NativeCallData") //
        .def(nb::init<>(), D_NA(NativeCallData, NativeCallData))
        .def_prop_rw("device", &NativeCallData::get_device, &NativeCallData::set_device, D_NA(NativeCallData, device))
        .def_prop_rw("kernel", &NativeCallData::get_kernel, &NativeCallData::set_kernel, D_NA(NativeCallData, kernel))
        .def_prop_rw(
            "call_dimensionality",
            &NativeCallData::get_call_dimensionality,
            &NativeCallData::set_call_dimensionality,
            D_NA(NativeCallData, call_dimensionality)
        )
        .def_prop_rw(
            "runtime",
            &NativeCallData::get_runtime,
            &NativeCallData::set_runtime,
            D_NA(NativeCallData, runtime)
        )
        .def_prop_rw(
            "call_mode",
            &NativeCallData::get_call_mode,
            &NativeCallData::set_call_mode,
            D_NA(NativeCallData, call_mode)
        )
        .def_prop_ro("last_call_shape", &NativeCallData::get_last_call_shape, D_NA(NativeCallData, last_call_shape))
        .def(
            "call",
            &NativeCallData::call,
            nb::arg("opts"),
            nb::arg("args"),
            nb::arg("kwargs"),
            D_NA(NativeCallData, call)
        )
        .def(
            "append_to",
            &NativeCallData::append_to,
            nb::arg("opts"),
            nb::arg("command_buffer"),
            nb::arg("args"),
            nb::arg("kwargs"),
            D_NA(NativeCallData, append_to)
        );

    nb::class_<Shape>(slangpy, "Shape") //
        .def(
            "__init__",
            [](Shape& self, nb::args args)
            {
                if (args.size() == 0) {
                    new (&self) Shape(std::vector<int>());
                } else if (args.size() == 1) {
                    if (args[0].is_none()) {
                        new (&self) Shape(std::nullopt);
                    } else if (nb::isinstance<nb::tuple>(args[0])) {
                        new (&self) Shape(nb::cast<std::vector<int>>(args[0]));
                    } else if (nb::isinstance<Shape>(args[0])) {
                        new (&self) Shape(nb::cast<Shape>(args[0]));
                    } else {
                        new (&self) Shape(nb::cast<std::vector<int>>(args));
                    }
                } else {
                    new (&self) Shape(nb::cast<std::vector<int>>(args));
                }
            },
            "args"_a,
            D_NA(Shape, Shape)
        )
        .def(
            "__add__",
            [](const Shape& self, const Shape& other) { return self + other; },
            nb::is_operator(),
            D_NA(Shape, operator+)
        )
        .def(
            "__getitem__",
            [](const Shape& self, size_t i) -> int
            {
                if (i >= self.size())
                    throw nb::index_error(); // throwing index_error allows this to be used as a python iterator
                return self[i];
            },
            nb::arg("index"),
            D_NA(Shape, operator[])
        )
        .def("__len__", &Shape::size, D_NA(Shape, size))
        .def_prop_ro("valid", &Shape::valid, D_NA(Shape, valid))
        .def_prop_ro("concrete", &Shape::concrete, D_NA(Shape, concrete))
        .def(
            "as_tuple",
            [](Shape& self)
            {
                std::vector<int>& v = self.as_vector();
                nb::list py_list;
                for (const int& item : v) {
                    py_list.append(item);
                }
                return nb::tuple(py_list);
            },
            D_NA(Shape, as_tuple)
        )
        .def(
            "as_list",
            [](Shape& self) { return self.as_vector(); },
            nb::rv_policy::reference_internal,
            D_NA(Shape, as_list)
        )
        .def("__repr__", &Shape::to_string, D_NA(Shape, to_string))
        .def("__str__", &Shape::to_string, D_NA(Shape, to_string))
        .def(
            "__eq__",
            [](const Shape& self, nb::object other)
            {
                if (nb::isinstance<Shape>(other)) {
                    return self.as_vector() == nb::cast<Shape>(other).as_vector();
                }

                std::vector<int> v;
                if (nb::try_cast(other, v)) {
                    return self.as_vector() == v;
                }

                return false;
            },
            D_NA(Shape, operator==)
        );

    nb::class_<CallContext, Object>(slangpy, "CallContext") //
        .def(
            nb::init<ref<Device>, const Shape&, CallMode>(),
            nb::arg("device"),
            nb::arg("call_shape"),
            nb::arg("call_mode"),
            D_NA(CallContext, CallContext)
        )
        .def_prop_ro(
            "device",
            [](const CallContext& self) -> Device* { return self.device(); },
            D_NA(CallContext, device)
        )
        .def_prop_ro(
            "call_shape",
            &CallContext::call_shape,
            nb::rv_policy::reference_internal,
            D_NA(CallContext, call_shape)
        )
        .def_prop_ro("call_mode", &CallContext::call_mode, D_NA(CallContext, call_mode));

    nb::class_<NativeNDBufferDesc>(slangpy, "NativeNDBufferDesc")
        .def(nb::init<>())
        .def_rw("dtype", &NativeNDBufferDesc::dtype)
        .def_rw("element_stride", &NativeNDBufferDesc::element_stride)
        .def_rw("shape", &NativeNDBufferDesc::shape)
        .def_rw("strides", &NativeNDBufferDesc::strides)
        .def_rw("usage", &NativeNDBufferDesc::usage)
        .def_rw("memory_type", &NativeNDBufferDesc::memory_type);

    nb::class_<NativeNDBuffer, Object>(slangpy, "NativeNDBuffer")
        .def(nb::init<ref<Device>, NativeNDBufferDesc>())
        .def_prop_ro("device", &NativeNDBuffer::device)
        .def_prop_rw("slangpy_signature", &NativeNDBuffer::slangpy_signature, &NativeNDBuffer::set_slagpy_signature)
        .def_prop_ro("dtype", &NativeNDBuffer::dtype)
        .def_prop_ro("shape", &NativeNDBuffer::shape)
        .def_prop_ro("strides", &NativeNDBuffer::strides)
        .def_prop_ro("element_count", &NativeNDBuffer::element_count)
        .def_prop_ro("usage", &NativeNDBuffer::usage)
        .def_prop_ro("memory_type", &NativeNDBuffer::memory_type)
        .def_prop_ro("storage", &NativeNDBuffer::storage);
}
