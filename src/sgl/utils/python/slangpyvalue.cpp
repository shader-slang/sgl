// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/utils/python/slangpyvalue.h"

namespace sgl {
extern void write_shader_cursor(ShaderCursor& cursor, nb::object value);
}

namespace sgl::slangpy {

void NativeValueMarshall::write_shader_cursor_pre_dispatch(
    CallContext* context,
    NativeBoundVariableRuntime* binding,
    ShaderCursor cursor,
    nb::object value,
    nb::list read_back
) const
{
    AccessType primal_access = binding->get_access().first;
    if (!value.is_none() && (primal_access == AccessType::read || primal_access == AccessType::readwrite)) {
        SGL_UNUSED(binding);
        SGL_UNUSED(context);
        ShaderCursor field = cursor[binding->get_variable_name()]["value"];
        write_shader_cursor(field, value);
    }
}

} // namespace sgl::slangpy


SGL_PY_EXPORT(utils_slangpy_value)
{
    using namespace sgl;
    using namespace sgl::slangpy;

    nb::module_ slangpy = m.attr("slangpy");

    nb::class_<NativeValueMarshall, NativeMarshall>(slangpy, "NativeValueMarshall") //
        .def(
            "__init__",
            [](NativeValueMarshall& self) { new (&self) NativeValueMarshall(); },
            D_NA(NativeValueMarshall, NativeValueMarshall)
        );
}
