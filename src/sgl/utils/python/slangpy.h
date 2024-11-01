// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vector>
#include <map>

#include "nanobind.h"

#include "sgl/core/macros.h"
#include "sgl/core/fwd.h"
#include "sgl/core/object.h"
#include "sgl/device/fwd.h"
#include "sgl/utils/slangpy.h"

namespace sgl::slangpy {

class NativeBoundVariableRuntime;

/// General exception that includes a message and the bound variable from which the error
/// originated.
class NativeBoundVariableException : public std::exception {
public:
    NativeBoundVariableException(std::string_view message, ref<NativeBoundVariableRuntime> source = nullptr)
        : m_message(message)
        , m_source(std::move(source))
    {
    }

    virtual const char* what() const noexcept override { return m_message.c_str(); }

    std::string_view message() const { return m_message; }
    ref<NativeBoundVariableRuntime> source() const { return m_source; }

private:
    std::string m_message;
    ref<NativeBoundVariableRuntime> m_source;
};

/// Base class for a marshal to a slangpy supported type.
class NativeType : public Object {
public:
    virtual ~NativeType() = default;

    /// Get the name of the type (eg 'int', 'vector<float,3>', 'Foo').
    std::string_view name() const { return m_name; }

    /// Set the name of the type
    void set_name(std::string_view name) { m_name = name; }

    /// Get the element type (eg vector<float,3> -> float).
    ref<NativeType> element_type() const { return m_element_type; }

    /// Set the element type.
    void set_element_type(ref<NativeType> element_type) { m_element_type = std::move(element_type); }

    /// Get the concrete shape of the type. For none-concrete types such as buffers,
    /// this will return an invalid shape.
    Shape concrete_shape() const { return m_concrete_shape; }

    /// Set the concrete shape of the type.
    void set_concrete_shape(const Shape& concrete_shape) { m_concrete_shape = concrete_shape; }

    /// Calculate size in bytes of a value of this type.
    virtual int get_byte_size(nb::object value) const
    {
        SGL_UNUSED(value);
        return 0;
    }

    /// Get the container shape when it is a container type (eg buffer, array, etc).
    /// This does not include the shape of the element type, so a float3[10] array
    /// would return a shape of [10]. This is typically only called from get_shape
    /// and is only required if a concrete shape has not been set.
    virtual Shape get_container_shape(nb::object value) const
    {
        SGL_UNUSED(value);
        return Shape(std::vector<int>());
    }

    /// Get the full shape of the type. This includes the container shape and the
    /// shape of the element type. Returns the concrete shape if set, or calculates
    /// the shape from the element type and container shape if not.
    virtual Shape get_shape(nb::object value) const
    {
        if (m_concrete_shape.valid())
            return m_concrete_shape;

        auto et = element_type();
        if (!et) {
            return get_container_shape(value);
        } else {
            return get_container_shape(value) + element_type()->get_shape(value);
        }
    }

    /// Create call data (uniform values) to be passed to a compute kernel.
    virtual nb::object create_calldata(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data) const
    {
        SGL_UNUSED(context);
        SGL_UNUSED(binding);
        SGL_UNUSED(data);
        return nb::none();
    }

    /// Optionally reads back changes from call data after a kernel has been executed.
    virtual void
    read_calldata(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data, nb::object result) const
    {
        SGL_UNUSED(context);
        SGL_UNUSED(binding);
        SGL_UNUSED(data);
        SGL_UNUSED(result);
    }

    /// For types that can be used to return data from a kernel, allocate
    /// an instance of the correct size to match the call context.
    virtual nb::object create_output(CallContext* context, NativeBoundVariableRuntime* binding) const
    {
        SGL_UNUSED(context);
        SGL_UNUSED(binding);
        return nb::none();
    };

    /// For types that can create_output, read the data allocated in create_output and
    /// return the python object to be provided to the user.
    virtual nb::object read_output(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data) const
    {
        SGL_UNUSED(context);
        SGL_UNUSED(binding);
        SGL_UNUSED(data);
        return nb::none();
    };

private:
    std::string m_name;
    ref<NativeType> m_element_type;
    Shape m_concrete_shape;
};

/// Nanobind trampoline class for NativeType
struct PyNativeType : public NativeType {
    NB_TRAMPOLINE(NativeType, 9);

    int get_byte_size(nb::object value) const override { NB_OVERRIDE(get_byte_size, value); }

    Shape get_container_shape(nb::object value) const override { NB_OVERRIDE(get_container_shape, value); }

    Shape get_shape(nb::object value) const override { NB_OVERRIDE(get_shape, value); }

    nb::object
    create_calldata(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data) const override
    {
        NB_OVERRIDE(create_calldata, context, binding, data);
    }

    void read_calldata(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data, nb::object result)
        const override
    {
        NB_OVERRIDE(read_calldata, context, binding, data, result);
    }

    nb::object create_output(CallContext* context, NativeBoundVariableRuntime* binding) const override
    {
        NB_OVERRIDE(create_output, context, binding);
    }
    nb::object read_output(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data) const override
    {
        NB_OVERRIDE(read_output, context, binding, data);
    }
};

/// Binding information that links a python argument to a slang parameter. In
/// the case of structs, this can be nested, mapping python dictionary fields to
/// slang struct fields.
class NativeBoundVariableRuntime : public Object {
public:
    NativeBoundVariableRuntime() = default;

    /// Get required access for primal and derivative.
    std::pair<AccessType, AccessType> get_access() const { return m_access; }

    /// Set required access for primal and derivative.
    void set_access(const std::pair<AccessType, AccessType>& access) { m_access = access; }

    /// Get the call transform.
    Shape get_transform() const { return m_transform; }

    /// Set the call transform.
    void set_transform(const Shape& transform) { m_transform = transform; }

    /// Get the python type marshal.
    ref<NativeType> get_python_type() const { return m_python_type; }

    /// Set the python type marshal.
    void set_python_type(const ref<NativeType>& python_type) { m_python_type = python_type; }

    /// Get the shape being used for the current call.
    Shape get_shape() const { return m_shape; }

    /// Set the shape being used for the current call.
    void set_shape(const Shape& shape) { m_shape = shape; }

    /// Get the uniform variable name.
    std::string_view get_variable_name() const { return m_variable_name; }

    /// Set the uniform variable name.
    void set_variable_name(std::string_view variable_name) { m_variable_name = variable_name; }

    /// Get children (for structs).
    std::optional<std::map<std::string, ref<NativeBoundVariableRuntime>>> get_children() const { return m_children; }

    /// Set children (for structs).
    void set_children(const std::optional<std::map<std::string, ref<NativeBoundVariableRuntime>>>& children)
    {
        m_children = children;
    }

    /// Recursively populate the overall kernel call shape.
    void populate_call_shape(std::vector<int>& call_shape, nb::object value);

    /// Write call data to be passed to a compute kernel by calling create_calldata on the marshal.
    void write_call_data_pre_dispatch(CallContext* context, nb::dict call_data, nb::object value);

    /// Read back changes from call data after a kernel has been executed by calling read_calldata on the marshal.
    void read_call_data_post_dispatch(CallContext* context, nb::dict call_data, nb::object value);

    /// Read output data from a compute kernel by calling read_output on the marshal.
    nb::object read_output(CallContext* context, nb::object data);

private:
    std::pair<AccessType, AccessType> m_access{AccessType::none, AccessType::none};
    Shape m_transform;
    ref<NativeType> m_python_type;
    Shape m_shape;
    std::string m_variable_name;
    std::optional<std::map<std::string, ref<NativeBoundVariableRuntime>>> m_children;
};

/// Binding information for a call to a compute kernel. Includes a set of positional
/// and keyword arguments as bound variables.
class NativeBoundCallRuntime : Object {
public:
    NativeBoundCallRuntime() = default;

    /// Get positional arguments.
    const std::vector<ref<NativeBoundVariableRuntime>>& get_args() const { return m_args; }

    /// Set positional arguments.
    void set_args(const std::vector<ref<NativeBoundVariableRuntime>>& args) { m_args = args; }

    /// Get keyword arguments.
    const std::map<std::string, ref<NativeBoundVariableRuntime>>& get_kwargs() const { return m_kwargs; }

    /// Set keyword arguments.
    void set_kwargs(const std::map<std::string, ref<NativeBoundVariableRuntime>>& kwargs) { m_kwargs = kwargs; }

    /// Find a keyword argument by name.
    ref<NativeBoundVariableRuntime> find_kwarg(const char* name) const
    {
        auto it = m_kwargs.find(name);
        if (it == m_kwargs.end()) {
            return nullptr;
        }
        return it->second;
    }

    /// Calculate the overall call shape by combining the shapes of all arguments.
    Shape calculate_call_shape(int call_dimensionality, nb::list args, nb::dict kwargs);

    /// Write call data to be passed to a compute kernel by calling create_calldata on the argument marshals.
    void write_calldata_pre_dispatch(CallContext* context, nb::dict call_data, nb::list args, nb::dict kwargs);

    /// Read back changes from call data after a kernel has been executed by calling read_calldata on the argument
    /// marshals.
    void read_call_data_post_dispatch(CallContext* context, nb::dict call_data, nb::list args, nb::dict kwargs);

private:
    std::vector<ref<NativeBoundVariableRuntime>> m_args;
    std::map<std::string, ref<NativeBoundVariableRuntime>> m_kwargs;
};

/// Contains the compute kernel for a call, the corresponding bindings and any additional
/// options provided by the user.
class NativeCallData : Object {
public:
    NativeCallData() = default;

    /// Get the device.
    ref<Device> get_device() const { return m_device; }

    /// Set the device.
    void set_device(const ref<Device>& device) { m_device = device; }

    /// Get the compute kernel.
    ref<ComputeKernel> get_kernel() const { return m_kernel; }

    /// Set the compute kernel.
    void set_kernel(const ref<ComputeKernel>& kernel) { m_kernel = kernel; }

    /// Get the call dimensionality.
    int get_call_dimensionality() const { return m_call_dimensionality; }

    /// Set the call dimensionality.
    void set_call_dimensionality(int call_dimensionality) { m_call_dimensionality = call_dimensionality; }

    /// Get the runtime bindings.
    ref<NativeBoundCallRuntime> get_runtime() const { return m_runtime; }

    /// Set the runtime bindings.
    void set_runtime(const ref<NativeBoundCallRuntime>& runtime) { m_runtime = runtime; }

    /// Get user provided uniforms.
    const nb::dict& get_vars() const { return m_vars; }

    /// Set user provided uniforms.
    void set_vars(const nb::dict& vars) { m_vars = vars; }

    /// Get the call mode (primitive/forward/backward).
    CallMode get_call_mode() const { return m_call_mode; }

    /// Set the call mode (primitive/forward/backward).
    void set_call_mode(CallMode call_mode) { m_call_mode = call_mode; }

    /// Add a hook to be called before dispatching or appending the kernel.
    void add_before_dispatch_hook(const std::function<void(nb::dict)>& hook)
    {
        m_before_dispatch_hooks.push_back(hook);
    }

    /// Add a hook to be called after dispatching the kernel. Not valid
    /// for append_to.
    void add_after_dispatch_hooks(const std::function<void(nb::dict)>& hook) { m_after_dispatch_hooks.push_back(hook); }

    /// Get the shape of the last call (useful for debugging).
    const Shape& get_last_call_shape() const { return m_last_call_shape; }

    /// Call the compute kernel with the provided arguments and keyword arguments.
    nb::object call(nb::args args, nb::kwargs kwargs);

    /// Append the compute kernel to a command buffer with the provided arguments and keyword arguments.
    nb::object append_to(ref<CommandBuffer> command_buffer, nb::args args, nb::kwargs kwargs);

private:
    ref<Device> m_device;
    ref<ComputeKernel> m_kernel;
    int m_call_dimensionality{0};
    ref<NativeBoundCallRuntime> m_runtime;
    nb::dict m_vars;
    CallMode m_call_mode{CallMode::prim};
    std::vector<std::function<void(nb::dict)>> m_before_dispatch_hooks;
    std::vector<std::function<void(nb::dict)>> m_after_dispatch_hooks;
    Shape m_last_call_shape;

    nb::object exec(CommandBuffer* command_buffer, nb::args args, nb::kwargs kwargs);

    nb::list unpack_args(nb::args args);

    nb::dict unpack_kwargs(nb::kwargs kwargs);

    nb::object unpack_arg(nanobind::object arg);

    void pack_arg(nb::object arg, nb::object unpacked_arg);
};

} // namespace sgl::slangpy
