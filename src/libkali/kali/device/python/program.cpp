#include "nanobind.h"

#include "kali/device/program.h"

NB_MAKE_OPAQUE(std::map<kali::TypeConformance, uint32_t>);
NB_MAKE_OPAQUE(std::map<std::string, std::string, std::less<>>);

KALI_PY_EXPORT(device_program)
{
    using namespace kali;

    nb::kali_enum<ShaderStage>(m, "ShaderStage");
    nb::kali_enum<ShaderModel>(m, "ShaderModel");
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

    nb::enum_<ShaderSourceType>(m, "ShaderSourceType")
        .value("file", ShaderSourceType::file)
        .value("string", ShaderSourceType::string);

    nb::class_<ShaderSourceDesc>(m, "ShaderSourceDesc")
        .def_rw("type", &ShaderSourceDesc::type)
        .def_rw("file", &ShaderSourceDesc::file)
        .def_rw("string", &ShaderSourceDesc::string);

    nb::class_<ShaderModuleDesc>(m, "ShaderModuleDesc")
        .def_rw("sources", &ShaderModuleDesc::sources)
        .def("add_file", &ShaderModuleDesc::add_file, "path"_a)
        .def("add_string", &ShaderModuleDesc::add_string, "string"_a);

    nb::class_<EntryPointDesc>(m, "EntryPointDesc")
        .def_rw("type", &EntryPointDesc::type)
        .def_rw("name", &EntryPointDesc::name)
        .def_rw("export_name", &EntryPointDesc::export_name);

    nb::class_<EntryPointGroupDesc>(m, "EntryPointGroupDesc")
        .def_rw("name", &EntryPointGroupDesc::name)
        .def_rw("shader_module_index", &EntryPointGroupDesc::shader_module_index)
        .def_rw("entry_points", &EntryPointGroupDesc::entry_points)
        .def("add_entry_point", &EntryPointGroupDesc::add_entry_point, "type"_a, "name"_a, "export_name"_a = "");

    nb::class_<ProgramDesc>(m, "ProgramDesc")
        .def_rw("shader_modules", &ProgramDesc::shader_modules)
        .def_rw("entry_point_groups", &ProgramDesc::entry_point_groups)
        .def_rw("shader_model", &ProgramDesc::shader_model)
        .def_rw("compiler_flags", &ProgramDesc::compiler_flags)
        .def_rw("prelude", &ProgramDesc::prelude)
        .def_rw("downstream_compiler_args", &ProgramDesc::downstream_compiler_args);

    nb::class_<Program, Object> program(m, "Program");
}
