// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vector>
#include <map>

#include "nanobind.h"

#include "sgl/utils/python/slangpy.h"

namespace sgl::slangpy {

/// Base class for marshalling simple value types between Python and Slang.
class NativeValueMarshall : public NativeMarshall {
public:
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
};

} // namespace sgl::slangpy
