#include "program.h"

#include "kali/device/device.h"
#include "kali/device/reflection.h"
#include "kali/device/helpers.h"

#include "kali/core/error.h"
#include "kali/core/type_utils.h"
#include "kali/core/resolver.h"
#include "kali/core/platform.h"

#include "types.inl"

// TODO
#define KALI_NVAPI_AVAILABLE 0

namespace kali {

Program::Program(ProgramDesc desc, ProgramManager* program_manager)
    : m_desc(std::move(desc))
    , m_program_manager(program_manager)
{
    KALI_ASSERT(m_program_manager);

    // If no shader model is selected, use the default shader model.
    if (m_desc.shader_model == ShaderModel::unknown)
        m_desc.shader_model = m_program_manager->get_device()->get_default_shader_model();

    // Validate descriptor.
    if (m_desc.shader_modules.empty())
        KALI_THROW("Program must have at least one module");
    for (const auto& module : m_desc.shader_modules) {
        if (module.sources.empty())
            KALI_THROW("Program must not have shader modules without sources");
    }
    if (m_desc.entry_point_groups.empty())
        KALI_THROW("Program must have at least one entry point group");
    for (const auto& entry_point_group : m_desc.entry_point_groups) {
        if (entry_point_group.entry_points.empty())
            KALI_THROW("Program must not have empty entry point groups");
        if (entry_point_group.shader_module_index >= m_desc.shader_modules.size())
            KALI_THROW(
                "Program entry point group '{}' references invalid shader module index {}",
                entry_point_group.name,
                entry_point_group.shader_module_index
            );
    }
    if (m_desc.shader_model > m_program_manager->get_device()->get_supported_shader_model())
        KALI_THROW("Shader model {} is not supported by the device", m_desc.shader_model);

    m_program_manager->add_loaded_program(this);
}

Program::~Program()
{
    m_program_manager->remove_loaded_program(this);
}

void Program::set_defines(const DefineList& defines)
{
    m_defines = defines;
    mark_dirty();
}

void Program::add_define(std::string_view name, std::string_view value)
{
    m_defines.add(name, value);
    mark_dirty();
}

void Program::add_defines(const DefineList& defines)
{
    m_defines.add(defines);
    mark_dirty();
}

void Program::remove_define(std::string_view name)
{
    m_defines.remove(name);
    mark_dirty();
}

const ProgramVersion* Program::get_active_version()
{
    if (!m_active_version) {
        // Lookup in cache.
        ProgramVersionKey key{m_defines, m_type_conformances};
        auto it = m_versions.find(key);
        if (it != m_versions.end()) {
            m_active_version = it->second;
            return m_active_version;
        }
        // Create a new version.
        std::string log;
        ref<ProgramVersion> version = m_program_manager->create_program_version(*this, log);
        if (!version)
            KALI_THROW("Failed to compile program:\n{}", log);
        m_versions.emplace(key, version);
        m_active_version = version;
    }

    return m_active_version;
}

bool Program::reload(bool force)
{
    // Check if any of the source files have changed.
    bool changed = false;
    for (auto it : m_source_file_timestamps) {
        if (std::filesystem::exists(it.first) && std::filesystem::last_write_time(it.first) != it.second) {
            changed = true;
            break;
        }
    }

    if (changed || force) {
        m_active_version = nullptr;
        m_versions.clear();
        m_source_file_timestamps.clear();
        return true;
    }

    return false;
}

ProgramVersion::ProgramVersion(
    const Program* program,
    Slang::ComPtr<slang::ICompileRequest> compile_request,
    Slang::ComPtr<slang::IComponentType> program_component,
    std::vector<Slang::ComPtr<slang::IComponentType>> entry_point_components,
    std::vector<Slang::ComPtr<slang::IComponentType>> linked_entry_point_components
)
    : m_program(program)
    , m_compile_request(std::move(compile_request))
    , m_program_component(std::move(program_component))
    , m_entry_point_components(std::move(entry_point_components))
    , m_linked_entry_point_components(std::move(linked_entry_point_components))
{
    KALI_ASSERT(m_program);
    m_program_layout = make_ref<ProgramLayout>(this, m_program_component->getLayout());
}

EntryPointLayout ProgramVersion::get_entry_point_layout(uint32_t entry_point) const
{
    KALI_ASSERT(entry_point < m_entry_point_components.size());
    return EntryPointLayout(
        this,
        entry_point,
        m_entry_point_components[entry_point]->getLayout()->getEntryPointByIndex(0)
    );
}

gfx::IShaderProgram* ProgramVersion::get_gfx_shader_program() const
{
    if (!m_gfx_shader_program) {

        slang::IComponentType* global_scope = m_program_component;
        std::vector<slang::IComponentType*> entry_points(
            m_entry_point_components.begin(),
            m_entry_point_components.end()
        );

        gfx::IShaderProgram::Desc shader_program_desc{
            .linkingStyle = gfx::IShaderProgram::LinkingStyle::SeparateEntryPointCompilation,
            .slangGlobalScope = global_scope,
            .entryPointCount = narrow_cast<gfx::GfxCount>(entry_points.size()),
            .slangEntryPoints = entry_points.data(),
        };

        Slang::ComPtr<ISlangBlob> diagnostics;
        if (SLANG_FAILED(m_program->get_program_manager()->get_device()->get_gfx_device()->createProgram(
                shader_program_desc,
                m_gfx_shader_program.writeRef(),
                diagnostics.writeRef()
            ))) {
            KALI_THROW(
                "Failed to create program version: {}",
                reinterpret_cast<const char*>(diagnostics->getBufferPointer())
            );
        }
    }

    return m_gfx_shader_program;
}

ProgramManager::ProgramManager(Device* device, slang::IGlobalSession* slang_session)
    : m_device(device)
    , m_slang_session(slang_session)
{
    KALI_ASSERT(device);
    KALI_ASSERT(slang_session);

    m_global_defines
        = { {"KALI_NVAPI_AVAILABLE", (KALI_NVAPI_AVAILABLE && m_device->get_type() == DeviceType::d3d12) ? "1" : "0"},
#if KALI_HAS_NVAPI
            {"NV_SHADER_EXTN_SLOT", "u999"},
#endif
          };

    m_disabled_warnings.push_back(15602); // #pragma once in modules
    m_disabled_warnings.push_back(30081); // implicit conversion
}

ref<Program> ProgramManager::create_program(ProgramDesc desc)
{
    return make_ref<Program>(std::move(desc), this);
}

ref<ProgramVersion> ProgramManager::create_program_version(const Program& program, std::string& out_log) const
{
    const ProgramDesc& program_desc = program.get_desc();

    slang::SessionDesc session_desc{};

    // Setup search paths.
    // Slang will search for files in the order they are specified.
    std::vector<std::string> search_path_strings(m_search_paths.size());
    std::vector<const char*> search_paths(m_search_paths.size());
    for (size_t i = 0; i < m_search_paths.size(); ++i) {
        search_path_strings[i] = m_search_paths[i].string();
        search_paths[i] = search_path_strings[i].c_str();
    }
    session_desc.searchPaths = search_paths.data();
    session_desc.searchPathCount = narrow_cast<SlangInt>(search_paths.size());

    // Select target profile.
    slang::TargetDesc target_desc;
    target_desc.format = SLANG_TARGET_UNKNOWN;
    std::string profile_str = fmt::format(
        "sm_{}_{}",
        get_shader_model_major_version(program_desc.shader_model),
        get_shader_model_minor_version(program_desc.shader_model)
    );
    target_desc.profile = m_slang_session->findProfile(profile_str.c_str());
    if (target_desc.profile == SLANG_PROFILE_UNKNOWN)
        KALI_THROW("Unsupported target profile: {}", profile_str);

    // Get compiler flags and override with global enabled/disabled flags.
    ShaderCompilerFlags compiler_flags = program_desc.compiler_flags;
    compiler_flags &= ~m_global_disabled_compiler_flags;
    compiler_flags |= m_global_enabled_compiler_flags;

    // Set floating point mode. If no shader compiler flags for this were set, we use Slang's default mode.
    bool flag_fast = is_set(compiler_flags, ShaderCompilerFlags::floating_point_mode_fast);
    bool flag_precise = is_set(compiler_flags, ShaderCompilerFlags::floating_point_mode_precise);
    if (flag_fast && flag_precise) {
        log_warn("Shader compiler flags 'floating_point_mode_fast' and 'floating_point_mode_precise' can't be used "
                 "simultaneously. Ignoring 'floating_point_mode_fast'.");
        flag_fast = false;
    }
    target_desc.floatingPointMode = SLANG_FLOATING_POINT_MODE_DEFAULT;
    if (flag_fast)
        target_desc.floatingPointMode = SLANG_FLOATING_POINT_MODE_FAST;
    else if (flag_precise)
        target_desc.floatingPointMode = SLANG_FLOATING_POINT_MODE_PRECISE;

    target_desc.forceGLSLScalarBufferLayout = true;

    // Select target format based on device type.
    const char* target_define{nullptr};
    switch (m_device->get_type()) {
    case DeviceType::d3d12:
        target_desc.format = SLANG_DXIL;
        target_define = "__TARGET_D3D12__";
        break;
    case DeviceType::vulkan:
        target_desc.format = SLANG_SPIRV;
        target_define = "__TARGET_VULKAN__";
        break;
    case DeviceType::cpu:
        target_desc.format = SLANG_SHADER_HOST_CALLABLE;
        target_define = "__TARGET_CPU__";
        break;
    case DeviceType::cuda:
        target_desc.format = SLANG_OBJECT_CODE;
        target_define = "__TARGET_CUDA__";
        break;
    default:
        KALI_UNREACHABLE();
    }
    KALI_ASSERT(target_define);

    // Add preprocessor macros.
    std::vector<slang::PreprocessorMacroDesc> macros;
    const auto add_macro = [&macros](const char* name, const char* value) { macros.push_back({name, value}); };

    // Add global followed by program specific defines.
    for (const auto& d : m_global_defines)
        add_macro(d.first.c_str(), d.second.c_str());
    for (const auto& d : program.get_defines())
        add_macro(d.first.c_str(), d.second.c_str());

    // Add target define.
    add_macro(target_define, "1");

    // Add shader model defines.
    std::string shader_model_major = std::to_string(get_shader_model_major_version(program_desc.shader_model));
    std::string shader_model_minor = std::to_string(get_shader_model_minor_version(program_desc.shader_model));
    add_macro("__SHADER_TARGET_MAJOR", shader_model_major.c_str());
    add_macro("__SHADER_TARGET_MINOR", shader_model_minor.c_str());

    session_desc.preprocessorMacros = macros.data();
    session_desc.preprocessorMacroCount = narrow_cast<SlangInt>(macros.size());

    session_desc.targets = &target_desc;
    session_desc.targetCount = 1;

    // We always use row-major matrix layout in Falcor so by default that's what we pass to Slang
    // to allow it to compute correct reflection information. Slang then invokes the downstream compiler.
    // Column major option can be useful when compiling external shader sources that don't depend
    // on anything Falcor.
    bool use_column_major = is_set(compiler_flags, ShaderCompilerFlags::matrix_layout_column_major);
    session_desc.defaultMatrixLayoutMode
        = use_column_major ? SLANG_MATRIX_LAYOUT_COLUMN_MAJOR : SLANG_MATRIX_LAYOUT_ROW_MAJOR;

    // Create a new slang session.
    // We do this because slang doesn't support reloading changed source files.
    Slang::ComPtr<slang::ISession> session;
    SLANG_CALL(m_slang_session->createSession(session_desc, session.writeRef()));
    KALI_ASSERT(session);

#if 0
    program.mFileTimeMap.clear(); // TODO @skallweit
#endif

    // Set language prelude. Only supported for HLSL.
    if (!program_desc.prelude.empty()) {
        if (target_desc.format == SLANG_DXIL) {
            m_slang_session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_HLSL, program_desc.prelude.c_str());
        } else {
            KALI_THROW("Language prelude set for unsupported target " + std::string(target_define));
        }
    }

    // Create compile request.
    Slang::ComPtr<slang::ICompileRequest> compile_request;
    SLANG_CALL(session->createCompileRequest(compile_request.writeRef()));
    KALI_ASSERT(compile_request);

    // Disable warnings.
    for (int warning : m_disabled_warnings)
        compile_request->overrideDiagnosticSeverity(warning, SLANG_SEVERITY_DISABLED);

    // Enable/disable intermediates dump.
    bool dump_intermediates = is_set(compiler_flags, ShaderCompilerFlags::dump_intermediates);
    compile_request->setDumpIntermediates(dump_intermediates);
    // TODO(@skallweit): set dump prefix

    // Set debug level.
    bool generate_debug_info = is_set(compiler_flags, ShaderCompilerFlags::generate_debug_info);
    if (generate_debug_info)
        compile_request->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_STANDARD);

    // Set additional flags.
    SlangCompileFlags flags = 0;

    // When we invoke the Slang compiler front-end, skip code generation step
    // so that the compiler does not complain about missing arguments for
    // specialization parameters.
    flags |= SLANG_COMPILE_FLAG_NO_CODEGEN;

    compile_request->setCompileFlags(flags);

    // Set downstream compiler arguments.
    {
        std::vector<const char*> args;
        for (const auto& arg : program_desc.downstream_compiler_args)
            args.push_back(arg.c_str());
#if KALI_NVAPI_AVAILABLE
        // TODO(@skallweit) check that downstream compiler is dxc
        // If NVAPI is available, we need to inform slang/dxc where to find it.
        std::string nvapi_include = "-I" + (get_runtime_directory() / "shaders/nvapi").string();
        args.push_back("-Xdxc");
        args.push_back(nvapi_include.c_str());
#endif
        if (!args.empty())
            SLANG_CALL(compile_request->processCommandLineArguments(args.data(), narrow_cast<int>(args.size())));
    }

    // Add shader modules.
    // Each shader module is a separate translation unit.
    for (size_t i = 0; i < program_desc.shader_modules.size(); ++i) {
        const ShaderModuleDesc& shader_module_desc = program_desc.shader_modules[i];
        int translation_unit_index = compile_request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
        KALI_ASSERT(translation_unit_index == i);
        for (const ShaderSourceDesc& source_desc : shader_module_desc.sources) {
            switch (source_desc.type) {
            case ShaderSourceType::file:
                compile_request->addTranslationUnitSourceFile(
                    translation_unit_index,
                    resolve_path(source_desc.file).string().c_str()
                );
                break;
            case ShaderSourceType::string:
                compile_request->addTranslationUnitSourceString(
                    translation_unit_index,
                    source_desc.file.string().c_str(),
                    source_desc.string.c_str()
                );
                break;
            }
        }
    }

    // Add entry points.
    for (const EntryPointGroupDesc& entry_point_group : program_desc.entry_point_groups) {
        for (const EntryPointDesc& entry_point_desc : entry_point_group.entry_points) {
            compile_request->addEntryPoint(
                narrow_cast<int>(entry_point_group.shader_module_index),
                entry_point_desc.name.c_str(),
                get_gfx_slang_stage(entry_point_desc.type)
            );
        }
    }

    // Compile program.
    SlangResult result = compile_request->compile();
    out_log += compile_request->getDiagnosticOutput();
    if (SLANG_FAILED(result))
        return nullptr;

    for (int i = 0; i < compile_request->getDependencyFileCount(); ++i) {
        std::filesystem::path path = compile_request->getDependencyFilePath(i);
        if (std::filesystem::exists(path))
            program.m_source_file_timestamps[path] = std::filesystem::last_write_time(path);
    }

    Slang::ComPtr<slang::IComponentType> program_component;
    SLANG_CALL(compile_request->getProgram(program_component.writeRef()));

    // Prepare entry points.
    std::vector<Slang::ComPtr<slang::IComponentType>> entry_point_components;
    SlangUInt entry_point_index = 0;
    for (const EntryPointGroupDesc& entry_point_group : program_desc.entry_point_groups) {
        for (const EntryPointDesc& entry_point_desc : entry_point_group.entry_points) {
            Slang::ComPtr<slang::IComponentType> entry_point;
            SLANG_CALL(compile_request->getEntryPoint(entry_point_index++, entry_point.writeRef()));
            KALI_ASSERT(entry_point);

            // Rename entry point in the generated code if the exported name differs from the source name.
            // This makes it possible to generate different specializations of the same source entry point,
            // for example by setting different type conformances.
            if (!entry_point_desc.export_name.empty() && entry_point_desc.export_name != entry_point_desc.name) {
                Slang::ComPtr<slang::IComponentType> renamed_entry_point;
                SLANG_CALL(
                    entry_point->renameEntryPoint(entry_point_desc.export_name.c_str(), renamed_entry_point.writeRef())
                );
                entry_point_components.push_back(renamed_entry_point);
            } else {
                entry_point_components.push_back(entry_point);
            }
        }
    }

    // Prepare linked entry points.
    std::vector<Slang::ComPtr<slang::IComponentType>> linked_entry_point_components;
    for (const auto& entry_point_component : entry_point_components) {
        slang::IComponentType* components[2] = {program_component, entry_point_component};
        Slang::ComPtr<slang::IComponentType> linked_entry_point;
        // TODO handle diagnostics
        SLANG_CALL(session->createCompositeComponentType(
            components,
            std::size(components),
            linked_entry_point.writeRef(),
            nullptr
        ));
        linked_entry_point_components.push_back(linked_entry_point);
    }

#if 0
    ProgramReflection::SharedPtr pReflector;
    if (!doSlangReflection(*pVersion, program_component, entry_point_components, pReflector, log))
    {
        return nullptr;
    }
#endif

#if 0
    auto descStr = program.getProgramDescString();
    pVersion->init(program.getDefineList(), pReflector, descStr, entry_point_components);

    timer.update();
    double time = timer.delta();
    mCompilationStats.programVersionCount++;
    mCompilationStats.programVersionTotalTime += time;
    mCompilationStats.programVersionMaxTime = std::max(mCompilationStats.programVersionMaxTime, time);
    logDebug("Created program version in {:.3f} s: {}", timer.delta(), descStr);
#endif

    return make_ref<ProgramVersion>(
        &program,
        compile_request,
        program_component,
        entry_point_components,
        linked_entry_point_components
    );
}

size_t ProgramManager::reload_programs(bool force)
{
    size_t count = 0;
    for (Program* program : m_loaded_programs)
        if (program->reload(force))
            count++;

    return count;
}

void ProgramManager::add_search_path(std::filesystem::path path)
{
    m_search_paths.push_back(std::move(path));
}

void ProgramManager::remove_search_path(std::filesystem::path path)
{
    m_search_paths.erase(std::remove(m_search_paths.begin(), m_search_paths.end(), path), m_search_paths.end());
}

void ProgramManager::add_global_define(std::string_view name, std::string_view value)
{
    m_global_defines.add(name, value);
}

void ProgramManager::remove_global_define(std::string_view name)
{
    m_global_defines.remove(name);
}

void ProgramManager::set_global_enabled_compiler_flags(ShaderCompilerFlags flags)
{
    m_global_enabled_compiler_flags = flags;
}

void ProgramManager::set_global_disabled_compiler_flags(ShaderCompilerFlags flags)
{
    m_global_disabled_compiler_flags = flags;
}

void ProgramManager::add_loaded_program(Program* program)
{
    KALI_ASSERT(program);
    m_loaded_programs.push_back(program);
}

void ProgramManager::remove_loaded_program(Program* program)
{
    KALI_ASSERT(program);
    m_loaded_programs.erase(
        std::remove(m_loaded_programs.begin(), m_loaded_programs.end(), program),
        m_loaded_programs.end()
    );
}

std::filesystem::path ProgramManager::resolve_path(const std::filesystem::path& path) const
{
    if (!path.is_absolute()) {
        for (const std::filesystem::path& search_path : m_search_paths) {
            std::filesystem::path full_path = search_path / path;
            if (std::filesystem::exists(full_path))
                return full_path;
        }
    }
    return path;
}

} // namespace kali
