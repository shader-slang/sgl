#include "nanobind.h"

#include "kali/device/shader.h"
#include "kali/device/reflection.h"

NB_MAKE_OPAQUE(std::map<kali::TypeConformance, uint32_t>);
NB_MAKE_OPAQUE(std::map<std::string, std::string, std::less<>>);

KALI_PY_EXPORT(device_shader)
{
    using namespace kali;

    nb::kali_enum<ShaderCompilerFlags>(m, "ShaderCompilerFlags");

    nb::class_<TypeConformance>(m, "TypeConformance")
        .def_rw("type_name", &TypeConformance::type_name)
        .def_rw("interface_name", &TypeConformance::interface_name);

    nb::bind_map<std::map<TypeConformance, uint32_t>>(m, "TypeConformanceListBase");
    nb::class_<TypeConformanceList, std::map<TypeConformance, uint32_t>>(m, "TypeConformanceList")
        .def(
            "add",
            nb::overload_cast<std::string, std::string, uint32_t>(&TypeConformanceList::add),
            "type_name"_a,
            "interface_name"_a,
            "id"_a = -1
        )
        .def(
            "remove",
            nb::overload_cast<std::string, std::string>(&TypeConformanceList::remove),
            "type_name"_a,
            "interface_name"_a
        )
        .def("add", nb::overload_cast<const TypeConformanceList&>(&TypeConformanceList::add), "other"_a)
        .def("remove", nb::overload_cast<const TypeConformanceList&>(&TypeConformanceList::remove), "other"_a);

    nb::bind_map<std::map<std::string, std::string, std::less<>>>(m, "DefineListBase");
    nb::class_<DefineList, std::map<std::string, std::string, std::less<>>>(m, "DefineList")
        .def("add", nb::overload_cast<std::string_view, std::string_view>(&DefineList::add), "name"_a, "value"_a = "")
        .def("remove", nb::overload_cast<std::string_view>(&DefineList::remove), "name"_a)
        .def("add", nb::overload_cast<const DefineList&>(&DefineList::add), "other"_a)
        .def("remove", nb::overload_cast<const DefineList&>(&DefineList::remove), "other"_a);

    nb::class_<SlangSessionDesc>(m, "SlangSessionDesc")
        .def_rw("shader_model", &SlangSessionDesc::shader_model)
        .def_rw("compiler_flags", &SlangSessionDesc::compiler_flags)
        .def_rw("compiler_args", &SlangSessionDesc::compiler_args)
        .def_rw("search_paths", &SlangSessionDesc::search_paths)
        .def_rw("defines", &SlangSessionDesc::defines);

    // Disambiguate from the types in slang.h
    using kali::SlangSession;
    using kali::SlangEntryPoint;

    nb::class_<SlangSession, Object>(m, "SlangSession")
        .def_prop_ro("desc", &SlangSession::desc)
        .def("load_module", &SlangSession::load_module, "path"_a)
        .def(
            "load_module_from_source",
            &SlangSession::load_module_from_source,
            "source"_a,
            "path"_a = std::filesystem::path{},
            "name"_a = ""
        );

    nb::class_<SlangModule, Object>(m, "SlangModule")
        .def_prop_ro("global_scope", &SlangModule::global_scope)
        .def_prop_ro("entry_points", &SlangModule::entry_points)
        .def("entry_point", &SlangModule::entry_point, "name"_a)
        .def("create_program", nb::overload_cast<std::string_view>(&SlangModule::create_program), "entry_point_name"_a)
        .def(
            "create_program",
            nb::overload_cast<ref<SlangGlobalScope>, ref<SlangEntryPoint>>(&SlangModule::create_program),
            "global_scope"_a,
            "entry_point"_a
        )
        .def(
            "create_program",
            nb::overload_cast<ref<SlangGlobalScope>, std::vector<ref<SlangEntryPoint>>>(&SlangModule::create_program),
            "global_scope"_a,
            "entry_points"_a
        );

    nb::class_<SlangComponentType, Object>(m, "SlangComponentType");
    nb::class_<SlangGlobalScope, SlangComponentType>(m, "SlangGlobalScope");

    nb::class_<SlangEntryPoint, SlangComponentType>(m, "SlangEntryPoint")
        .def_prop_ro("name", &SlangEntryPoint::name)
        .def_prop_ro("stage", &SlangEntryPoint::stage)
        .def_prop_ro("layout", &SlangEntryPoint::layout, nb::rv_policy::reference_internal)
        .def("rename", &SlangEntryPoint::rename, "new_name"_a);

    nb::class_<ShaderProgram, Object>(m, "ShaderProgram")
        .def_prop_ro("program_layout", &ShaderProgram::program_layout, nb::rv_policy::reference_internal)
        // .def_prop_ro("entry_point_layouts", &ShaderProgram::entry_point_layouts)
        .def("entry_point_layout", &ShaderProgram::entry_point_layout, "index"_a, nb::rv_policy::reference_internal);
}
