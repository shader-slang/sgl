// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vector>
#include <map>

#include "nanobind.h"

#include "sgl/core/macros.h"
#include "sgl/core/fwd.h"
#include "sgl/core/object.h"

#include "sgl/device/fwd.h"
#include "sgl/device/resource.h"

#include "sgl/utils/python/slangpy.h"

namespace sgl::slangpy {

struct NativeNDBufferDesc {
    ref<NativeSlangType> dtype;
    int element_stride;
    Shape shape;
    Shape strides;
    ResourceUsage usage{ResourceUsage::shader_resource | ResourceUsage::unordered_access};
    MemoryType memory_type{MemoryType::device_local};
};

class NativeNDBuffer : public Object {
public:
    NativeNDBuffer(Device* device, NativeNDBufferDesc desc);

    Device* device() const { return storage()->device(); }
    std::string_view slangpy_signature() const { return m_signature; }
    void set_slagpy_signature(std::string_view signature) { m_signature = signature; }
    ref<NativeSlangType> dtype() const { return m_desc.dtype; }
    Shape shape() const { return m_desc.shape; }
    Shape strides() const { return m_desc.strides; }
    size_t element_count() const { return m_desc.shape.element_count(); }
    ResourceUsage usage() const { return m_desc.usage; }
    MemoryType memory_type() const { return m_desc.memory_type; }
    ref<Buffer> storage() const { return m_storage; }

private:
    NativeNDBufferDesc m_desc;
    ref<Buffer> m_storage;
    std::string m_signature;
};


class NativeNDBufferMarshall : public NativeMarshall {
public:
    NativeNDBufferMarshall(
        int dims,
        bool writable,
        ref<NativeSlangType> slang_type,
        ref<NativeSlangType> slang_element_type,
        int element_stride
    )
        : NativeMarshall(slang_type)
        , m_dims(dims)
        , m_writable(writable)
        , m_slang_element_type(slang_element_type)
        , m_element_stride(element_stride)
    {
    }
    /*
    /// Writes call data to a shader cursor before dispatch, optionally writing data for
    /// read back after the kernel has executed. By default, this calls through to
    /// create_calldata, which is typically overridden python side to generate a dictionary.
    void write_shader_cursor_pre_dispatch(
        CallContext* context,
        NativeBoundVariableRuntime* binding,
        ShaderCursor cursor,
        nb::object value,
        nb::list read_back
    ) const override;


    /// Dispatch data is just the value.
    nb::object create_dispatchdata(nb::object data) const override { return data; }

    /// If requested, output is just the input value (as it can't have changed).
    nb::object read_output(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data) const override
    {
        SGL_UNUSED(context);
        SGL_UNUSED(binding);
        return data;
    };
    */

    int dims() const { return m_dims; }
    bool writable() const { return m_writable; }
    ref<NativeSlangType> slang_element_type() const { return m_slang_element_type; }

    Shape get_shape(nb::object data) const override
    {
        auto buffer = nb::cast<NativeNDBuffer*>(data);
        return buffer->shape();
    }

    void write_shader_cursor_pre_dispatch(
        CallContext* context,
        NativeBoundVariableRuntime* binding,
        ShaderCursor cursor,
        nb::object value,
        nb::list read_back
    ) const override
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
        std::vector<int> shape_vec = buffer->shape().as_vector();
        field["shape"]
            ._set_array(&shape_vec[0], shape_vec.size() * 4, TypeReflection::ScalarType::int32, shape_vec.size());

        // Generate and write strides vector, clearing strides to 0
        // for dimensions that are broadcast.
        std::vector<int> strides_vec = buffer->strides().as_vector();
        std::vector<int> transform = binding->get_transform().as_vector();
        std::vector<int> call_shape = context->call_shape().as_vector();
        for (size_t i = 0; i < transform.size(); i++) {
            int csidx = transform[i];
            if (call_shape[csidx] != shape_vec[i]) {
                strides_vec[i] = 0;
            }
        }

        // Write the strides vector as an array of ints.
        field["strides"]
            ._set_array(&strides_vec[0], strides_vec.size() * 4, TypeReflection::ScalarType::int32, strides_vec.size());
    }

    void read_calldata(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data, nb::object result)
        const override
    {
        SGL_UNUSED(context);
        SGL_UNUSED(binding);
        SGL_UNUSED(data);
        SGL_UNUSED(result);
    }

    nb::object create_output(CallContext* context, NativeBoundVariableRuntime* binding) const override
    {
        SGL_UNUSED(context);
        SGL_UNUSED(binding);
        NativeNDBufferDesc desc;
        desc.dtype = m_slang_element_type;
        desc.element_stride = m_element_stride;
        desc.shape = context->call_shape();
        desc.strides = desc.shape.calc_contiguous_strides();
        desc.usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access;
        desc.memory_type = MemoryType::device_local;
        auto buffer = make_ref<NativeNDBuffer>(context->device(), desc);
        return nb::cast(buffer);
    }

    nb::object create_dispatchdata(nb::object data) const override
    {
        // Cast value to buffer, and get the cursor field to write to.
        auto buffer = nb::cast<NativeNDBuffer*>(data);
        nb::dict res;
        res["buffer"] = buffer->storage();
        res["shape"] = buffer->shape().as_vector();
        res["strides"] = buffer->strides().as_vector();
        return res;
    }

    nb::object read_output(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data) const override
    {
        SGL_UNUSED(context);
        SGL_UNUSED(binding);
        return data;
    }


private:
    int m_dims;
    bool m_writable;
    ref<NativeSlangType> m_slang_element_type;
    int m_element_stride;
};

/// Nanobind trampoline class for NativeNDBufferMarshall as can't currently implement create_output in native.
struct PyNativeNDBufferMarshall : public NativeNDBufferMarshall {
    NB_TRAMPOLINE(NativeNDBufferMarshall, 1);

    nb::object create_output(CallContext* context, NativeBoundVariableRuntime* binding) const override
    {
        NB_OVERRIDE(create_output, context, binding);
    }
};

} // namespace sgl::slangpy
