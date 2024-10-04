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

class NativeBoundVariableException : public std::exception {
public:
    NativeBoundVariableException(const std::string& message, ref<NativeBoundVariableRuntime> source = nullptr)
        : m_message(message)
        , m_source(std::move(source))
    {
    }

    virtual const char* what() const noexcept override { return m_message.c_str(); }

    const std::string& message() const { return m_message; }
    NativeBoundVariableRuntime* source() const { return m_source; }

private:
    std::string m_message;
    ref<NativeBoundVariableRuntime> m_source;
};

class NativeType : public Object {
public:
    virtual ~NativeType() = default;

    std::string name() const { return m_name; }
    void set_name(const std::string& name) { m_name = name; }
    ref<NativeType> element_type() const { return m_element_type; }
    void set_element_type(const ref<NativeType>& element_type) { m_element_type = element_type; }
    Shape concrete_shape() const { return m_concrete_shape; }
    void set_concrete_shape(const Shape& concrete_shape) { m_concrete_shape = concrete_shape; }

    virtual int get_byte_size(nb::object value) const
    {
        SGL_UNUSED(value);
        return 0;
    }

    virtual Shape get_container_shape(nb::object value) const
    {
        SGL_UNUSED(value);
        return Shape();
    }

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

    virtual nb::object create_calldata(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data) const
    {
        SGL_UNUSED(context);
        SGL_UNUSED(binding);
        SGL_UNUSED(data);
        return nb::none();
    }

    virtual void
    read_calldata(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data, nb::object result) const
    {
        SGL_UNUSED(context);
        SGL_UNUSED(binding);
        SGL_UNUSED(data);
        SGL_UNUSED(result);
    }

    virtual nb::object create_output(CallContext* context) const
    {
        SGL_UNUSED(context);
        return nb::none();
    };
    virtual nb::object read_output(CallContext* context, nb::object data) const
    {
        SGL_UNUSED(context);
        SGL_UNUSED(data);
        return nb::none();
    };

private:
    std::string m_name;
    ref<NativeType> m_element_type;
    Shape m_concrete_shape;
};

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

    nb::object create_output(CallContext* context) const override { NB_OVERRIDE(create_output, context); }
    nb::object read_output(CallContext* context, nb::object data) const override
    {
        NB_OVERRIDE(read_output, context, data);
    }
};

class NativeBoundVariableRuntime : public Object {
public:
    NativeBoundVariableRuntime() = default;

    std::pair<AccessType, AccessType> get_access() const { return m_access; }
    void set_access(const std::pair<AccessType, AccessType>& access) { m_access = access; }

    Shape get_transform() const { return m_transform; }
    void set_transform(const Shape& transform) { m_transform = transform; }

    Shape get_slang_shape() const { return m_slang_shape; }
    void set_slang_shape(const Shape& slang_shape) { m_slang_shape = slang_shape; }

    ref<NativeType> get_python_type() const { return m_python_type; }
    void set_python_type(const ref<NativeType>& python_type) { m_python_type = python_type; }

    Shape get_shape() const { return m_shape; }
    void set_shape(const Shape& shape) { m_shape = shape; }

    std::string get_name() const { return m_name; }
    void set_name(const std::string& name) { m_name = name; }

    std::string get_variable_name() const { return m_variable_name; }
    void set_variable_name(const std::string& variable_name) { m_variable_name = variable_name; }

    std::optional<std::map<std::string, ref<NativeBoundVariableRuntime>>> get_children() const { return m_children; }
    void set_children(const std::optional<std::map<std::string, ref<NativeBoundVariableRuntime>>>& children)
    {
        m_children = children;
    }


    void populate_call_shape(std::vector<int>& call_shape, nb::object value);

    void write_call_data_pre_dispatch(CallContext* context, nb::dict call_data, nb::object value);

    void read_call_data_post_dispatch(CallContext* context, nb::dict call_data, nb::object value);

    nb::object read_output(CallContext* context, nb::object data);

private:
    std::pair<AccessType, AccessType> m_access{AccessType::none, AccessType::none};
    Shape m_transform;
    Shape m_slang_shape;
    ref<NativeType> m_python_type;
    Shape m_shape;
    std::string m_name;
    std::string m_variable_name;
    std::optional<std::map<std::string, ref<NativeBoundVariableRuntime>>> m_children;
};

class NativeBoundCallRuntime : Object {
public:
    NativeBoundCallRuntime() = default;

    std::vector<ref<NativeBoundVariableRuntime>> get_args() const { return m_args; }

    void set_args(const std::vector<ref<NativeBoundVariableRuntime>>& args) { m_args = args; }

    std::map<std::string, ref<NativeBoundVariableRuntime>> get_kwargs() const { return m_kwargs; }

    void set_kwargs(const std::map<std::string, ref<NativeBoundVariableRuntime>>& kwargs) { m_kwargs = kwargs; }

    ref<NativeBoundVariableRuntime> find_kwarg(const char* name) const
    {
        auto it = m_kwargs.find(name);
        if (it == m_kwargs.end()) {
            return nullptr;
        }
        return it->second;
    }

    Shape calculate_call_shape(int call_dimensionality, nb::list args, nb::dict kwargs);

    void write_calldata_pre_dispatch(CallContext* context, nb::dict call_data, nb::list args, nb::dict kwargs);

    void read_call_data_post_dispatch(CallContext* context, nb::dict call_data, nb::list args, nb::dict kwargs);

private:
    std::vector<ref<NativeBoundVariableRuntime>> m_args;
    std::map<std::string, ref<NativeBoundVariableRuntime>> m_kwargs;
};

class NativeCallData : Object {
public:
    NativeCallData() = default;

    ref<Device> get_device() const { return m_device; }
    void set_device(const ref<Device>& device) { m_device = device; }

    ref<ComputeKernel> get_kernel() const { return m_kernel; }
    void set_kernel(const ref<ComputeKernel>& kernel) { m_kernel = kernel; }

    int get_call_dimensionality() const { return m_call_dimensionality; }
    void set_call_dimensionality(int call_dimensionality) { m_call_dimensionality = call_dimensionality; }

    ref<NativeBoundCallRuntime> get_runtime() const { return m_runtime; }
    void set_runtime(const ref<NativeBoundCallRuntime>& runtime) { m_runtime = runtime; }

    const nb::dict& get_vars() const { return m_vars; }
    void set_vars(const nb::dict& vars) { m_vars = vars; }

    CallMode get_call_mode() const { return m_call_mode; }
    void set_call_mode(CallMode call_mode) { m_call_mode = call_mode; }

    void add_before_dispatch_hook(const std::function<void(nb::dict)>& hook)
    {
        m_before_dispatch_hooks.push_back(hook);
    }

    void add_after_dispatch_hooks(const std::function<void(nb::dict)>& hook) { m_after_dispatch_hooks.push_back(hook); }

    const Shape& get_last_call_shape() const { return m_last_call_shape; }

    nb::object call(nb::args args, nb::kwargs kwargs);

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
