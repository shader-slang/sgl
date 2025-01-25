// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/device.h"
#include "sgl/device/command.h"

#include "sgl/utils/python/slangpyfunction.h"

namespace sgl {

} // namespace sgl

namespace sgl::slangpy {

nb::object NativeFunctionNode::call(nb::dict cache, nb::args args, nb::kwargs kwargs)
{
    SGL_UNUSED(cache);
    SGL_UNUSED(args);
    SGL_UNUSED(kwargs);
    return nb::none();
}

void NativeFunctionNode::append_to(nb::dict cache, CommandBuffer* command_buffer, nb::args args, nb::kwargs kwargs)
{
    SGL_UNUSED(cache);
    SGL_UNUSED(command_buffer);
    SGL_UNUSED(args);
    SGL_UNUSED(kwargs);
}

} // namespace sgl::slangpy

SGL_PY_EXPORT(utils_slangpy_function)
{
    using namespace sgl;
    using namespace sgl::slangpy;

    nb::module_ slangpy = m.attr("slangpy");

    nb::sgl_enum<FunctionNodeType>(slangpy, "FunctionNodeType");

    nb::class_<NativeFunctionNode, PyNativeFunctionNode, NativeObject>(slangpy, "NativeFunctionNode")
        .def(
            "__init__",
            [](NativeFunctionNode& self,
               std::optional<NativeFunctionNode*> parent,
               FunctionNodeType type,
               nb::object data) { new (&self) PyNativeFunctionNode(parent.value_or(nullptr), type, data); },
            "parent"_a.none(),
            "type"_a,
            "data"_a.none(),
            D_NA(NativeFunctionNode, NativeFunctionNode)
        )
        .def_prop_ro("_native_parent", &NativeFunctionNode::parent)
        .def_prop_ro("_native_type", &NativeFunctionNode::type)
        .def_prop_ro("_native_data", &NativeFunctionNode::data)
        .def("_find_native_root", &NativeFunctionNode::find_root, D_NA(NativeFunctionNode, find_root))
        .def("_native_call", &NativeFunctionNode::call, "cache"_a, "args"_a, "kwargs"_a, D_NA(NativeFunctionNode, call))
        .def(
            "_native_append_to",
            &NativeFunctionNode::append_to,
            "cache"_a,
            "command_buffer"_a,
            "args"_a,
            "kwargs"_a,
            D_NA(NativeFunctionNode, append_to)
        )
        .def(
            "generate_call_data",
            &NativeFunctionNode::generate_call_data,
            "args"_a,
            "kwargs"_a,
            D_NA(NativeFunctionNode, generate_call_data)
        )
        .def(
            "read_signature",
            &NativeFunctionNode::read_signature,
            "builder"_a,
            D_NA(NativeFunctionNode, read_signature)
        )
        .def(
            "gather_runtime_options",
            &NativeFunctionNode::gather_runtime_options,
            "options"_a,
            D_NA(NativeFunctionNode, gather_runtime_options)
        );
}
