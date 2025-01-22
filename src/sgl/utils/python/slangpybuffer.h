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

    int dims() const { return m_dims; }
    bool writable() const { return m_writable; }
    ref<NativeSlangType> slang_element_type() const { return m_slang_element_type; }
    int element_stride() const { return m_element_stride; }

    Shape get_shape(nb::object data) const override;

    void write_shader_cursor_pre_dispatch(
        CallContext* context,
        NativeBoundVariableRuntime* binding,
        ShaderCursor cursor,
        nb::object value,
        nb::list read_back
    ) const override;

    void read_calldata(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data, nb::object result)
        const override;

    nb::object create_output(CallContext* context, NativeBoundVariableRuntime* binding) const override;

    nb::object create_dispatchdata(nb::object data) const override;

    nb::object read_output(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data) const override;

protected:
    ref<NativeNDBuffer> create_buffer(Device* device, const Shape& shape) const;

private:
    int m_dims;
    bool m_writable;
    ref<NativeSlangType> m_slang_element_type;
    int m_element_stride;
};

class NativeNumpyMarshall : public NativeNDBufferMarshall {
public:
    NativeNumpyMarshall(
        int dims,
        ref<NativeSlangType> slang_type,
        ref<NativeSlangType> slang_element_type,
        int element_stride,
        nb::dlpack::dtype dtype
    )
        : NativeNDBufferMarshall(dims, true, slang_type, slang_element_type, element_stride)
        , m_dtype(dtype)
    {
    }

    nb::dlpack::dtype dtype() const { return m_dtype; }

    Shape get_shape(nb::object data) const override;

    void write_shader_cursor_pre_dispatch(
        CallContext* context,
        NativeBoundVariableRuntime* binding,
        ShaderCursor cursor,
        nb::object value,
        nb::list read_back
    ) const override;

    void read_calldata(CallContext* context, NativeBoundVariableRuntime* binding, nb::object data, nb::object result)
        const override;

    nb::object create_output(CallContext* context, NativeBoundVariableRuntime* binding) const override;

    nb::object create_dispatchdata(nb::object data) const override;

private:
    nb::dlpack::dtype m_dtype;
};

} // namespace sgl::slangpy
