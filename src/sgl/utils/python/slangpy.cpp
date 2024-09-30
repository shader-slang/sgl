// SPDX-License-Identifier: Apache-2.0

#include <sstream>

#include "nanobind.h"

#include "sgl/core/macros.h"
#include "sgl/core/logger.h"
#include "sgl/utils/slangpy.h"

namespace sgl::slangpy {
void build_string_signature(nb::handle o, std::stringstream& stream)
{
    auto type_name = nb::str(nb::getattr(o.type(), "__name__"));
    stream << type_name.c_str() << "\n";

    if (nb::hasattr(o, "slangpy_signature")) {

        auto slangpy_sig = nb::getattr(o, "slangpy_signature");
        stream << nb::str(slangpy_sig).c_str() << "\n";
    }
}
} // namespace sgl::slangpy

SGL_PY_EXPORT(utils_slangpy)
{
    using namespace sgl;
    using namespace sgl::slangpy;

    nb::module_ slangpy = m.attr("slangpy");

    nb::sgl_enum<AccessType>(slangpy, "AccessType");

    slangpy.def(
        "hash_signature",
        [](nb::args args, nb::kwargs kwargs)
        {
            std::stringstream stream;
            int i = 0;
            for (const auto& value : args) {
                stream << i++ << "\n";
                build_string_signature(value, stream);
            }

            for (const auto& [key, value] : kwargs) {
                stream << nb::str(key).c_str() << "\n";
                build_string_signature(value, stream);
            }
            return stream.str();
        },
        "args"_a,
        "kwargs"_a,
        D_NA(slangpy, hash_signature)
    );

    nb::class_<NativeType, Object>(slangpy, "NativeType") //
        .def("__init__", [](NativeType& self) { new (&self) NativeType(); });

    nb::class_<BoundCallRuntime, Object>(slangpy, "BoundCallRuntime") //
        .def(
            "__init__",
            [](BoundCallRuntime& self, nb::args args, nb::kwargs kwargs)
            {
                new (&self) BoundCallRuntime();
                SGL_UNUSED(self);
                SGL_UNUSED(args);
                SGL_UNUSED(kwargs);
            },
            "args"_a,
            "kwargs"_a
        );

    nb::class_<BoundVariableRuntime, Object>(slangpy, "BoundVariableRuntime") //
        .def(
            "__init__",
            [](BoundVariableRuntime& self, nb::object bound_variable)
            {
                new (&self) BoundVariableRuntime();

                SGL_UNUSED(self);
                SGL_UNUSED(bound_variable);

                auto access_pair = nb::cast<std::vector<AccessType>>(bound_variable.attr("access"));

                auto nb_transform = bound_variable.attr("transform");
                auto transform = nb_transform.is_none() ? std::vector<int>() : nb::cast<std::vector<int>>(nb_transform);

                auto nb_shape = bound_variable.attr("slang").attr("primal").attr("get_shape")(nb::none());
                auto shape = nb::cast<std::vector<int>>(nb_shape);

                log_info("Transform: {}\n", transform);
                log_info("Shape: {}\n", shape);

                // self.access = source.access
                // self.transform: Optional[TConcreteShape] = check_concrete(
                //     source.transform) if source.transform is not None else None
                // self.slang_shape = source.slang.primal.get_shape()
                //
                // self.shape: TConcreteShape = ()
                //
                // self._get_shape = source.python.primal.get_shape
                // self._create_calldata = source.python.primal.create_calldata
                // self._read_calldata = source.python.primal.read_calldata
                // self._create_output = source.python.primal.create_output
                // self._read_output = source.python.primal.read_output
                //
                // self._source_for_exceptions = source
                // self._name = source.python.name
                // self._variable_name = source.variable_name
                // self._children: Optional[dict[str, BoundVariableRuntime]] = {
                //     name: BoundVariableRuntime(child) for name, child in source.children.items()
                // } if source.children is not None else None
            }
        );
}
