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

namespace sgl {
extern void write_shader_cursor(ShaderCursor& cursor, nb::object value);
}

namespace sgl::slangpy {

void NativeBoundVariableRuntime::populate_call_shape(std::vector<int>& call_shape, nb::object value)
{
    if (m_children) {
        for (auto& [name, child_ref] : *m_children) {
            nb::object child_value = value[name.c_str()];
            if (child_ref) {
                child_ref->populate_call_shape(call_shape, child_value);
            }
        }
    } else if (!value.is_none()) {
        if (!m_transform.valid()) {
            throw NativeBoundVariableException("Transform shape is not set.", ref(this));
        }
        auto tf = m_transform.as_vector();
        size_t csl = call_shape.size();

        m_shape = m_python_type->get_shape(value);
        auto shape = m_shape.as_vector();

        for (size_t i = 0; i < tf.size(); ++i) {
            int shape_dim = shape[i];
            int call_idx = tf[i];

            if (call_idx >= static_cast<int>(csl)) {
                continue;
            }

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
        nb::dict cd_val;
        for (auto& [name, child_ref] : *m_children) {
            nb::object child_value = value[name.c_str()];
            if (child_ref) {
                child_ref->write_call_data_pre_dispatch(context, cd_val, child_value);
            }
        }
        if (cd_val.size() > 0) {
            call_data[m_variable_name.c_str()] = cd_val;
        }
    } else {
        nb::object cd_val = m_python_type->create_calldata(context, this, value);
        if (!cd_val.is_none()) {
            call_data[m_variable_name.c_str()] = cd_val;
        }
    }
}

void NativeBoundVariableRuntime::read_call_data_post_dispatch(
    CallContext* context,
    nb::dict call_data,
    nb::object value
)
{
    if (!call_data.contains(m_variable_name.c_str())) {
        return;
    }
    auto cd_val = call_data[m_variable_name.c_str()];

    if (m_children) {
        auto dict = nb::cast<nb::dict>(cd_val);
        for (auto& [name, child_ref] : *m_children) {
            nb::object child_value = value[name.c_str()];
            if (child_ref) {
                child_ref->read_call_data_post_dispatch(context, dict, child_value);
            }
        }
    } else {
        m_python_type->read_calldata(context, this, value, cd_val);
    }
}

nb::object NativeBoundVariableRuntime::read_output(CallContext* context, nb::object data)
{
    if (m_children) {
        nb::dict res;
        for (auto& [name, child_ref] : *m_children) {
            if (res.contains(name.c_str())) {
                nb::object child_data = data[child_ref->m_name.c_str()];
                if (child_ref) {
                    res[name.c_str()] = child_ref->read_output(context, child_data);
                }
            }
        }
        return res;
    } else {
        if (m_access.first == AccessType::write || m_access.first == AccessType::readwrite) {
            return m_python_type->read_output(context, data);
        }
        return nb::none();
    }
}

Shape NativeBoundCallRuntime::calculate_call_shape(int call_dimensionality, nb::list args, nb::dict kwargs)
{
    std::vector<int> call_shape(call_dimensionality, 1);
    for (size_t idx = 0; idx < args.size(); ++idx) {
        m_args[idx]->populate_call_shape(call_shape, args[idx]);
    }
    for (auto [key, value] : kwargs) {
        auto it = m_kwargs.find(nb::str(key).c_str());
        if (it != m_kwargs.end()) {
            it->second->populate_call_shape(call_shape, nb::cast<nb::object>(value));
        }
    }
    return Shape(call_shape);
}

void NativeBoundCallRuntime::write_calldata_pre_dispatch(
    CallContext* context,
    nb::dict call_data,
    nb::list args,
    nb::dict kwargs
)
{
    for (size_t idx = 0; idx < args.size(); ++idx) {
        m_args[idx]->write_call_data_pre_dispatch(context, call_data, args[idx]);
    }
    for (auto [key, value] : kwargs) {
        auto it = m_kwargs.find(nb::str(key).c_str());
        if (it != m_kwargs.end()) {
            it->second->write_call_data_pre_dispatch(context, call_data, nb::cast<nb::object>(value));
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
    for (size_t idx = 0; idx < args.size(); ++idx) {
        m_args[idx]->read_call_data_post_dispatch(context, call_data, args[idx]);
    }
    for (auto [key, value] : kwargs) {
        auto it = m_kwargs.find(nb::str(key).c_str());
        if (it != m_kwargs.end()) {
            it->second->read_call_data_post_dispatch(context, call_data, nb::cast<nb::object>(value));
        }
    }
}

nb::object NativeCallData::call(nb::args args, nb::kwargs kwargs)
{
    return exec(nullptr, args, kwargs);
}

nb::object NativeCallData::append_to(ref<CommandBuffer> command_buffer, nb::args args, nb::kwargs kwargs)
{
    return exec(command_buffer.get(), args, kwargs);
}

void log_nb(nb::handle o)
{
    nb::dict d;
    if (nb::try_cast(o, d)) {
        for (auto [k, v] : d) {
            log_info("{}: {{\n", nb::str(k).c_str());
            log_nb(v);
            log_info("}}\n");
        }
    } else {
        log_info("{}\n", nb::str(o).c_str());
    }
}

nb::object NativeCallData::exec(CommandBuffer* command_buffer, nb::args args, nb::kwargs kwargs)
{
    SGL_UNUSED(command_buffer);
    SGL_UNUSED(args);
    SGL_UNUSED(kwargs);
#if true
    nb::dict call_data;

    // rv_node handling would depend on specific implementation

    // Unpack args and kwargs
    nb::list unpacked_args = unpack_args(args);
    nb::dict unpacked_kwargs = unpack_kwargs(kwargs);

    // log_nb(kwargs);
    // log_nb(unpacked_kwargs);

    // Calculate call shape
    Shape call_shape = m_runtime->calculate_call_shape(m_call_dimensionality, unpacked_args, unpacked_kwargs);
    m_last_call_shape = call_shape;


    // Setup context
    auto context = make_ref<CallContext>(m_device, call_shape);

    // Allocate return value if needed
    if (!command_buffer && m_call_mode == CallMode::prim) {
        ref<NativeBoundVariableRuntime> rv_node = m_runtime->find_kwarg("_result");
        if (rv_node && (!kwargs.contains("_result") || kwargs["_result"].is_none())) {
            nb::object output = rv_node->get_python_type()->create_output(context);
            kwargs["_result"] = output;
            unpacked_kwargs["_result"] = output;
            rv_node->populate_call_shape(call_shape.as_vector(), output);
        }
    }

    m_runtime->write_calldata_pre_dispatch(context, call_data, unpacked_args, unpacked_kwargs);

    // Calculate total threads and strides
    int total_threads = 1;
    std::vector<int> strides;
    auto cs = call_shape.as_vector();
    for (auto it = cs.rbegin(); it != cs.rend(); ++it) {
        strides.push_back(total_threads);
        total_threads *= *it;
    }
    std::reverse(strides.begin(), strides.end());

    if (!strides.empty()) {
        call_data["_call_stride"] = nb::cast(strides);
        call_data["_call_dim"] = nb::cast(cs);
    }
    call_data["_thread_count"] = uint3(total_threads, 1, 1);

    // Merge sets
    nb::dict vars = nb::dict(m_vars);
    vars["call_data"] = call_data;

    // Execute before dispatch hooks
    for (auto& hook : m_before_dispatch_hooks) {
        hook(vars);
    }

    // Dispatch the kernel
    auto bind_vars = [&](ShaderCursor cursor) { write_shader_cursor(cursor, vars); };
    m_kernel->dispatch(uint3(total_threads, 1, 1), bind_vars, command_buffer);

    // If command_buffer is not null, return early
    if (command_buffer != nullptr) {
        return nanobind::none();
    }

    // Execute after dispatch hooks
    for (auto& hook : m_after_dispatch_hooks) {
        hook(vars);
    }

    // Read call data post dispatch
    m_runtime->read_call_data_post_dispatch(context, call_data, unpacked_args, unpacked_kwargs);

    // Pack updated 'this' values back
    for (size_t i = 0; i < args.size(); ++i) {
        pack_arg(args[i], unpacked_args[i]);
    }
    for (auto [k, v] : kwargs) {
        pack_arg(nb::cast<nb::object>(v), unpacked_kwargs[k]);
    }

    // Handle return value based on call mode
    if (m_call_mode == CallMode::prim) {
        auto rv_node_it = m_runtime->find_kwarg("_result");
        if (rv_node_it && !unpacked_kwargs["_result"].is_none()) {
            return rv_node_it->read_output(context, unpacked_kwargs["_result"]);
        }
    }
#endif
    return nb::none();
}

nb::list NativeCallData::unpack_args(nb::args args)
{
    nb::list unpacked;
    for (auto arg : args) {
        unpacked.append(unpack_arg(nb::cast<nb::object>(arg)));
    }
    return unpacked;
}

nb::dict NativeCallData::unpack_kwargs(nb::kwargs kwargs)
{
    nb::dict unpacked;
    for (const auto& [k, v] : kwargs) {
        unpacked[k] = unpack_arg(nb::cast<nb::object>(v));
    }
    return unpacked;
}

nb::object NativeCallData::unpack_arg(nb::object arg)
{
    auto obj = arg;

    if (nb::hasattr(obj, "get_this")) {
        obj = nb::getattr(obj, "get_this")();
    }

    nb::dict d;
    if (nb::try_cast(obj, d)) {
        nb::dict res;
        for (auto [k, v] : d) {
            res[k] = unpack_arg(nb::cast<nb::object>(v));
        }
        obj = res;
    }

    nb::list l;
    if (nb::try_cast(obj, l)) {
        nb::list res;
        for (auto v : l) {
            res.append(unpack_arg(nb::cast<nb::object>(v)));
        }
        obj = res;
    }

    return obj;
}

void NativeCallData::pack_arg(nanobind::object arg, nanobind::object unpacked_arg)
{
    if (nb::hasattr(arg, "update_this")) {
        nb::getattr(arg, "update_this")(unpacked_arg);
    }

    nb::dict d;
    if (nb::try_cast(arg, d)) {
        for (auto [k, v] : d) {
            pack_arg(nb::cast<nb::object>(v), unpacked_arg[k]);
        }
    }

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

    nb::exception<NativeBoundVariableException>(slangpy, "NativeBoundVariableException");

    nb::class_<NativeType, PyNativeType, Object>(slangpy, "NativeType") //
        .def(
            "__init__",
            [](NativeType& self) { new (&self) PyNativeType(); },
            D_NA(NativeType, NativeType)
        )
        .def_prop_rw("name", &NativeType::name, &NativeType::set_name, D_NA(NativeType, name))
        .def_prop_rw(
            "element_type",
            &NativeType::element_type,
            &NativeType::set_element_type,
            D_NA(NativeType, element_type)
        )
        .def_prop_rw(
            "concrete_shape",
            &NativeType::concrete_shape,
            &NativeType::set_concrete_shape,
            D_NA(NativeType, concrete_shape)
        )
        .def("get_byte_size", &NativeType::get_byte_size, D_NA(NativeType, get_byte_size))
        .def("get_container_shape", &NativeType::get_container_shape, D_NA(NativeType, get_container_shape))
        .def("get_shape", &NativeType::get_shape, "value"_a = nb::none(), D_NA(NativeType, get_shape))
        .def("create_calldata", &NativeType::create_calldata, D_NA(NativeType, create_calldata))
        .def("read_calldata", &NativeType::read_calldata, D_NA(NativeType, read_calldata))
        .def("create_output", &NativeType::create_output, D_NA(NativeType, create_output))
        .def("read_output", &NativeType::read_output, D_NA(NativeType, read_output));

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
            "slang_shape",
            &NativeBoundVariableRuntime::get_slang_shape,
            &NativeBoundVariableRuntime::set_slang_shape,
            D_NA(NativeBoundVariableRuntime, slang_shape)
        )
        .def_prop_rw(
            "python_type",
            &NativeBoundVariableRuntime::get_python_type,
            &NativeBoundVariableRuntime::set_python_type,
            D_NA(NativeBoundVariableRuntime, python_type)
        )
        .def_prop_rw(
            "shape",
            &NativeBoundVariableRuntime::get_shape,
            &NativeBoundVariableRuntime::set_shape,
            D_NA(NativeBoundVariableRuntime, shape)
        )
        .def_prop_rw(
            "name",
            &NativeBoundVariableRuntime::get_name,
            &NativeBoundVariableRuntime::set_name,
            D_NA(NativeBoundVariableRuntime, name)
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
        .def_prop_rw("vars", &NativeCallData::get_vars, &NativeCallData::set_vars, D_NA(NativeCallData, vars))
        .def_prop_rw(
            "call_mode",
            &NativeCallData::get_call_mode,
            &NativeCallData::set_call_mode,
            D_NA(NativeCallData, call_mode)
        )
        .def_prop_ro("last_call_shape", &NativeCallData::get_last_call_shape, D_NA(NativeCallData, last_call_shape))
        .def(
            "add_before_dispatch_hook",
            &NativeCallData::add_before_dispatch_hook,
            D_NA(NativeCallData, add_before_dispatch_hook)
        )
        .def(
            "add_after_dispatch_hook",
            &NativeCallData::add_after_dispatch_hooks,
            D_NA(NativeCallData, add_after_dispatch_hooks)
        )
        .def("call", &NativeCallData::call, nb::arg("args"), nb::arg("kwargs"), D_NA(NativeCallData, call))
        .def(
            "append_to",
            &NativeCallData::append_to,
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

    /* def __eq__(self, value
                   : object)
                ->bool : if isinstance (value, Shape)
            : return self.shape
            == value.shape else : return self.shape
            == value*/


    nb::class_<CallContext, Object>(slangpy, "CallContext") //
        .def(
            nb::init<ref<Device>, const Shape&>(),
            nb::arg("device"),
            nb::arg("call_shape"),
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
        );
}
