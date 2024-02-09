#include "nanobind.h"

#include "kali/device/shader.h"
#include "kali/device/reflection.h"
#include "kali/device/kernel.h"

namespace kali {
KALI_DICT_TO_DESC_BEGIN(SlangCompilerOptions)
KALI_DICT_TO_DESC_FIELD(shader_model, ShaderModel);
KALI_DICT_TO_DESC_FIELD(compiler_flags, SlangCompilerFlags);
KALI_DICT_TO_DESC_FIELD(compiler_args, std::vector<std::string>);
KALI_DICT_TO_DESC_FIELD(search_paths, std::vector<std::filesystem::path>);
KALI_DICT_TO_DESC_FIELD(add_default_search_paths, bool);
KALI_DICT_TO_DESC_FIELD(defines, DefineList);
KALI_DICT_TO_DESC_END()
} // namespace kali

NB_MAKE_OPAQUE(std::map<kali::TypeConformance, uint32_t>);
NB_MAKE_OPAQUE(std::map<std::string, std::string, std::less<>>);

KALI_PY_EXPORT(device_shader)
{
    using namespace kali;

    nb::class_<TypeConformance>(m, "TypeConformance", D(TypeConformance))
        .def_rw("type_name", &TypeConformance::type_name, D(TypeConformance, type_name))
        .def_rw("interface_name", &TypeConformance::interface_name, D(TypeConformance, interface_name));

    nb::bind_map<TypeConformanceList>(m, "TypeConformanceList", D(TypeConformanceList))
        .def(
            "add",
            nb::overload_cast<std::string, std::string, uint32_t>(&TypeConformanceList::add),
            "type_name"_a,
            "interface_name"_a,
            "id"_a = -1,
            D(TypeConformanceList, add)
        )
        .def(
            "remove",
            nb::overload_cast<std::string, std::string>(&TypeConformanceList::remove),
            "type_name"_a,
            "interface_name"_a,
            D(TypeConformanceList, remove)
        )
        .def(
            "add",
            nb::overload_cast<const TypeConformanceList&>(&TypeConformanceList::add),
            "other"_a,
            D(TypeConformanceList, add_2)
        )
        .def(
            "remove",
            nb::overload_cast<const TypeConformanceList&>(&TypeConformanceList::remove),
            "other"_a,
            D(TypeConformanceList, remove_2)
        );

    nb::bind_map<DefineList>(m, "DefineList", D(DefineList))
        .def(
            "add",
            nb::overload_cast<std::string_view, std::string_view>(&DefineList::add),
            "name"_a,
            "value"_a = "",
            D(DefineList, add)
        )
        .def("remove", nb::overload_cast<std::string_view>(&DefineList::remove), "name"_a, D(DefineList, remove))
        .def("add", nb::overload_cast<const DefineList&>(&DefineList::add), "other"_a, D(DefineList, add_2))
        .def("remove", nb::overload_cast<const DefineList&>(&DefineList::remove), "other"_a, D(DefineList, remove_2));

    nb::kali_enum_flags<SlangCompilerFlags>(m, "SlangCompilerFlags");

    nb::class_<SlangCompilerOptions>(m, "SlangCompilerOptions")
        .def(nb::init<>())
        .def(
            "__init__",
            [](SlangCompilerOptions* self, nb::dict dict)
            { new (self) SlangCompilerOptions(dict_to_SlangCompilerOptions(dict)); }
        )
        .def_rw("shader_model", &SlangCompilerOptions::shader_model)
        .def_rw("compiler_flags", &SlangCompilerOptions::compiler_flags)
        .def_rw("compiler_args", &SlangCompilerOptions::compiler_args)
        .def_rw("search_paths", &SlangCompilerOptions::search_paths)
        .def_rw("add_default_search_paths", &SlangCompilerOptions::add_default_search_paths)
        .def_rw("defines", &SlangCompilerOptions::defines);
    nb::implicitly_convertible<nb::dict, SlangCompilerOptions>();

    nb::class_<SlangSessionDesc>(m, "SlangSessionDesc")
        .def(nb::init<>())
        .def_rw("compiler_options", &SlangSessionDesc::compiler_options);

    // Disambiguate from the types in slang.h
    using kali::SlangSession;
    using kali::SlangEntryPoint;

    nb::class_<SlangSession, Object>(m, "SlangSession", D(SlangSession))
        .def_prop_ro("desc", &SlangSession::desc, D(SlangSession, desc))
        .def(
            "load_module",
            &SlangSession::load_module,
            "path"_a,
            "defines"_a.none() = nb::none(),
            D(SlangSession, load_module)
        )
        .def(
            "load_module_from_source",
            &SlangSession::load_module_from_source,
            "source"_a,
            "path"_a = "",
            "name"_a = "",
            "defines"_a.none() = nb::none(),
            D(SlangSession, load_module_from_source)
        );

    nb::class_<SlangModule, Object>(m, "SlangModule", D(SlangModule))
        .def_prop_ro("global_scope", &SlangModule::global_scope, D(SlangModule, global_scope))
        .def_prop_ro("entry_points", &SlangModule::entry_points, D(SlangModule, entry_points))
        .def("entry_point", &SlangModule::entry_point, "name"_a, D(SlangModule, entry_point))
        .def(
            "create_program",
            nb::overload_cast<std::string_view>(&SlangModule::create_program),
            "entry_point_name"_a,
            D(SlangModule, create_program)
        )
        .def(
            "create_program",
            nb::overload_cast<ref<SlangGlobalScope>, ref<SlangEntryPoint>>(&SlangModule::create_program),
            "global_scope"_a,
            "entry_point"_a,
            D(SlangModule, create_program_2)
        )
        .def(
            "create_program",
            nb::overload_cast<ref<SlangGlobalScope>, std::vector<ref<SlangEntryPoint>>>(&SlangModule::create_program),
            "global_scope"_a,
            "entry_points"_a,
            D(SlangModule, create_program_3)
        )
        .def(
            "create_compute_kernel",
            &SlangModule::create_compute_kernel,
            "entry_point_name"_a,
            D(SlangModule, create_compute_kernel)
        );

    nb::class_<SlangComponentType, Object>(m, "SlangComponentType", D(SlangComponentType));
    nb::class_<SlangGlobalScope, SlangComponentType>(m, "SlangGlobalScope", D(SlangGlobalScope));

    nb::class_<SlangEntryPoint, SlangComponentType>(m, "SlangEntryPoint", D(SlangEntryPoint))
        .def_prop_ro("name", &SlangEntryPoint::name, D(SlangEntryPoint, name))
        .def_prop_ro("stage", &SlangEntryPoint::stage, D(SlangEntryPoint, stage))
        .def_prop_ro("layout", &SlangEntryPoint::layout, nb::rv_policy::reference_internal, D(SlangEntryPoint, layout))
        .def("rename", &SlangEntryPoint::rename, "new_name"_a, D(SlangEntryPoint, rename));

    nb::class_<ShaderProgram, Object>(m, "ShaderProgram", D(ShaderProgram))
        .def_prop_ro(
            "program_layout",
            &ShaderProgram::program_layout,
            nb::rv_policy::reference_internal,
            D(ShaderProgram, program_layout)
        )
        // .def_prop_ro("entry_point_layouts", &ShaderProgram::entry_point_layouts)
        .def(
            "entry_point_layout",
            &ShaderProgram::entry_point_layout,
            "index"_a,
            nb::rv_policy::reference_internal,
            D(ShaderProgram, entry_point_layout)
        )
        .def_prop_ro("reflection", &ShaderProgram::reflection, D(ShaderProgram, reflection));
}
