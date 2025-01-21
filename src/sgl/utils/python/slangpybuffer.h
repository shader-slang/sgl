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
    NativeNDBuffer(ref<Device> device, NativeNDBufferDesc desc);

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
        ref<NativeSlangType> slang_element_type
    )
        : NativeMarshall(slang_type)
        , m_dims(dims)
        , m_writable(writable)
        , m_slang_element_type(slang_element_type)
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

private:
    int m_dims;
    bool m_writable;
    ref<NativeSlangType> m_slang_element_type;
};

} // namespace sgl::slangpy
