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
    ref<TypeLayoutReflection> element_layout;
    Shape shape;
    Shape strides;
    ResourceUsage usage{ResourceUsage::shader_resource | ResourceUsage::unordered_access};
    MemoryType memory_type{MemoryType::device_local};
};

class NativeNDBuffer : public NativeObject {
public:
    NativeNDBuffer(Device* device, NativeNDBufferDesc desc);

    Device* device() const { return storage()->device(); }
    ref<NativeSlangType> dtype() const { return m_desc.dtype; }
    const Shape& shape() const { return m_desc.shape; }
    const Shape& strides() const { return m_desc.strides; }
    size_t element_count() const { return m_desc.shape.element_count(); }
    ResourceUsage usage() const { return m_desc.usage; }
    MemoryType memory_type() const { return m_desc.memory_type; }
    ref<Buffer> storage() const { return m_storage; }
    size_t element_stride() const { return m_desc.element_layout->stride(); }

    ref<BufferCursor> cursor(std::optional<int> start = std::nullopt, std::optional<int> count = std::nullopt) const;
    nb::dict uniforms() const;

private:
    NativeNDBufferDesc m_desc;
    ref<Buffer> m_storage;
};


class NativeNDBufferMarshall : public NativeMarshall {
public:
    NativeNDBufferMarshall(
        int dims,
        bool writable,
        ref<NativeSlangType> slang_type,
        ref<NativeSlangType> slang_element_type,
        ref<TypeLayoutReflection> element_layout
    )
        : NativeMarshall(slang_type)
        , m_dims(dims)
        , m_writable(writable)
        , m_slang_element_type(slang_element_type)
        , m_element_layout(element_layout)
    {
    }

    int dims() const { return m_dims; }
    bool writable() const { return m_writable; }
    ref<NativeSlangType> slang_element_type() const { return m_slang_element_type; }
    ref<TypeLayoutReflection> element_layout() const { return m_element_layout; }
    size_t element_stride() const { return m_element_layout->stride(); }

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
    ref<TypeLayoutReflection> m_element_layout;
};

class NativeNumpyMarshall : public NativeNDBufferMarshall {
public:
    NativeNumpyMarshall(
        int dims,
        ref<NativeSlangType> slang_type,
        ref<NativeSlangType> slang_element_type,
        ref<TypeLayoutReflection> element_layout,
        nb::dlpack::dtype dtype
    )
        : NativeNDBufferMarshall(dims, true, slang_type, slang_element_type, element_layout)
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
