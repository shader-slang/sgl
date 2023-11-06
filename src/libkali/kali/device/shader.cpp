#include "shader.h"

#include "kali/device/device.h"
#include "kali/device/helpers.h"
#include "kali/device/reflection.h"
#include "kali/device/kernel.h"

#include "kali/core/type_utils.h"

#include <slang.h>

#include "kali/device/types.inl"

namespace kali {


SlangSession::SlangSession(ref<Device> device, SlangSessionDesc desc)
    : m_device(std::move(device))
    , m_desc(std::move(desc))
{
    KALI_CHECK(m_device, "device is null");

    // If no shader model is selected, use the default shader model.
    if (m_desc.shader_model == ShaderModel::unknown)
        m_desc.shader_model = m_device->default_shader_model();

    // Check that requested shader model is supported.
    ShaderModel supported_shader_model = m_device->supported_shader_model();
    KALI_CHECK(
        m_desc.shader_model <= supported_shader_model,
        "Shader model {} is not supported (max shader model is {})",
        m_desc.shader_model,
        supported_shader_model
    );

    DeviceType device_type = m_device->type();

    slang::SessionDesc session_desc{};

    // Setup search paths.
    // Slang will search for files in the order they are specified.
    // Use module local search paths first, followed by global search paths.
    const auto& search_paths = m_desc.search_paths;
    std::vector<std::string> search_path_strings(search_paths.size());
    std::vector<const char*> search_paths_cstrings(search_paths.size());
    for (size_t i = 0; i < search_paths.size(); ++i) {
        search_path_strings[i] = search_paths[i].string();
        search_paths_cstrings[i] = search_path_strings[i].c_str();
    }

    session_desc.searchPaths = search_paths_cstrings.data();
    session_desc.searchPathCount = narrow_cast<SlangInt>(search_paths_cstrings.size());

    // Select target profile.
    slang::TargetDesc target_desc;
    target_desc.format = SLANG_TARGET_UNKNOWN;
    std::string profile_str = fmt::format(
        "sm_{}_{}",
        get_shader_model_major_version(m_desc.shader_model),
        get_shader_model_minor_version(m_desc.shader_model)
    );
    target_desc.profile = m_device->get_global_session()->findProfile(profile_str.c_str());
    KALI_CHECK(target_desc.profile != SLANG_PROFILE_UNKNOWN, "Unsupported target profile: {}", profile_str);

    // Get compiler flags and override with global enabled/disabled flags.
    ShaderCompilerFlags compiler_flags = m_desc.compiler_flags;
    // TODO handle global flags
    // compiler_flags &= ~m_program_manager->get_global_disabled_compiler_flags();
    // compiler_flags |= m_program_manager->get_global_enabled_compiler_flags();

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
    switch (device_type) {
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

    // Add module local defines first, followed by global defines.
    for (const auto& d : m_desc.defines)
        add_macro(d.first.c_str(), d.second.c_str());
    // TODO handle global defines
    // for (const auto& d : m_program_manager->get_global_defines())
    //     add_macro(d.first.c_str(), d.second.c_str());

    // Add target define.
    add_macro(target_define, "1");

    // Add shader model defines.
    std::string shader_model_major = std::to_string(get_shader_model_major_version(m_desc.shader_model));
    std::string shader_model_minor = std::to_string(get_shader_model_minor_version(m_desc.shader_model));
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

    SLANG_CALL(m_device->get_global_session()->createSession(session_desc, m_slang_session.writeRef()));
}

SlangSession::~SlangSession() { }

ref<SlangModule> SlangSession::load_module(const std::filesystem::path& path)
{
    return make_ref<SlangModule>(
        ref<SlangSession>(this),
        SlangModuleDesc{.type = SlangModuleDesc::Type::file, .path = path}
    );
}

ref<SlangModule> SlangSession::load_module_from_source(
    const std::string& source,
    const std::filesystem::path& path,
    const std::string& name
)
{
    KALI_UNUSED(name);
    return make_ref<SlangModule>(
        ref<SlangSession>(this),
        SlangModuleDesc{.type = SlangModuleDesc::Type::source, .path = path, .source = source}
    );
}

std::filesystem::path SlangSession::resolve_path(const std::filesystem::path& path)
{
    if (path.is_absolute())
        return path;

    for (const std::filesystem::path& search_path : m_desc.search_paths) {
        std::filesystem::path full_path = search_path / path;
        if (std::filesystem::exists(full_path))
            return full_path;
    }

    return path;
}

SlangModule::SlangModule(ref<SlangSession> session, SlangModuleDesc desc)
    : m_session(std::move(session))
    , m_desc(std::move(desc))
{
    // Create compile request.
    SLANG_CALL(m_session->get_slang_session()->createCompileRequest(m_compile_request.writeRef()));

    // Disable warnings.
    // for (int warning : m_disabled_warnings)
    //     m_compile_request->overrideDiagnosticSeverity(warning, SLANG_SEVERITY_DISABLED);

    ShaderCompilerFlags compiler_flags = m_session->desc().compiler_flags;

    // Enable/disable intermediates dump.
    bool dump_intermediates = is_set(compiler_flags, ShaderCompilerFlags::dump_intermediates);
    m_compile_request->setDumpIntermediates(dump_intermediates);
    // TODO(@skallweit): set dump prefix

    // Set debug level.
    bool generate_debug_info = is_set(compiler_flags, ShaderCompilerFlags::generate_debug_info);
    if (generate_debug_info)
        m_compile_request->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_STANDARD);

    // Set additional flags.
    SlangCompileFlags flags = 0;

    // When we invoke the Slang compiler front-end, skip code generation step
    // so that the compiler does not complain about missing arguments for
    // specialization parameters.
    flags |= SLANG_COMPILE_FLAG_NO_CODEGEN;

    m_compile_request->setCompileFlags(flags);

    // Set compiler arguments.
    {
        std::vector<const char*> args;
        for (const auto& arg : m_session->desc().compiler_args)
            args.push_back(arg.c_str());
#if KALI_NVAPI_AVAILABLE
        std::string nvapi_include = "-I" + (get_runtime_directory() / "shaders/nvapi").string();
        if (device_type == DeviceType::d3d12) {
            // If NVAPI is available, we need to inform slang/dxc where to find it.
            args.push_back("-Xdxc");
            args.push_back(nvapi_include.c_str());
        }
#endif
        if (!args.empty())
            SLANG_CALL(m_compile_request->processCommandLineArguments(args.data(), narrow_cast<int>(args.size())));
    }

    // Add source.
    m_compile_request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    switch (m_desc.type) {
    case SlangModuleDesc::Type::file:
        m_compile_request->addTranslationUnitSourceFile(0, m_session->resolve_path(m_desc.path).string().c_str());
        break;
    case SlangModuleDesc::Type::source:
        m_compile_request->addTranslationUnitSourceString(0, m_desc.path.string().c_str(), m_desc.source.c_str());
        break;
    }

    // Compile.
    SlangResult result = m_compile_request->compile();
    std::string log = m_compile_request->getDiagnosticOutput();
    if (result != SLANG_OK) {
        throw SlangCompileError(fmt::format("Failed to compile shader module:\n{}", log));
    } else if (!log.empty()) {
        log_warn("Slang compiler warnings:\n{}", log);
    }

    // Query number of entry points.
    slang::ProgramLayout* layout = slang::ProgramLayout::get(m_compile_request);
    m_entry_point_count = layout->getEntryPointCount();
}

SlangModule::~SlangModule() { }

ref<SlangGlobalScope> SlangModule::global_scope()
{
    Slang::ComPtr<slang::IComponentType> component_type;
    SLANG_CALL(m_compile_request->getProgram(component_type.writeRef()));
    return make_ref<SlangGlobalScope>(ref<SlangModule>(this), component_type);
}

ref<SlangEntryPoint> SlangModule::entry_point(std::string_view name)
{
    for (const auto& entry_point : entry_points()) {
        if (entry_point->name() == name)
            return entry_point;
    }
    return {};
}

std::vector<ref<SlangEntryPoint>> SlangModule::entry_points()
{
    std::vector<ref<SlangEntryPoint>> entry_points;
    for (size_t i = 0; i < m_entry_point_count; i++) {
        Slang::ComPtr<slang::IComponentType> component_type;
        if (m_compile_request->getEntryPoint(i, component_type.writeRef()) != SLANG_OK || !component_type)
            break;
        entry_points.push_back(make_ref<SlangEntryPoint>(ref<SlangModule>(this), component_type));
    }
    return entry_points;
}

ref<ShaderProgram> SlangModule::create_program(std::string_view entry_point_name)
{
    return create_program(global_scope(), entry_point(entry_point_name));
}

ref<ShaderProgram> SlangModule::create_program(ref<SlangGlobalScope> global_scope, ref<SlangEntryPoint> entry_point)
{
    std::vector<ref<SlangEntryPoint>> entry_points(1);
    entry_points[0] = std::move(entry_point);
    return make_ref<ShaderProgram>(m_session->device(), std::move(global_scope), std::move(entry_points));
}

ref<ShaderProgram>
SlangModule::create_program(ref<SlangGlobalScope> global_scope, std::vector<ref<SlangEntryPoint>> entry_points)
{
    return make_ref<ShaderProgram>(m_session->device(), std::move(global_scope), std::move(entry_points));
}

ref<ComputeKernel> SlangModule::create_compute_kernel(std::string_view entry_point_name)
{
    return make_ref<ComputeKernel>(m_session->device(), create_program(entry_point_name));
}

SlangComponentType::SlangComponentType(ref<SlangModule> module, Slang::ComPtr<slang::IComponentType> component_type)
    : m_module(std::move(module))
    , m_component_type(std::move(component_type))
{
}

SlangGlobalScope::SlangGlobalScope(ref<SlangModule> module, Slang::ComPtr<slang::IComponentType> component_type)
    : SlangComponentType(std::move(module), std::move(component_type))
{
}

const ProgramLayout* SlangGlobalScope::layout() const
{
    return ProgramLayout::from_slang(m_component_type->getLayout());
}

std::string SlangGlobalScope::to_string() const
{
    return fmt::format("SlangGlobalScope()");
}

SlangEntryPoint::SlangEntryPoint(ref<SlangModule> module, Slang::ComPtr<slang::IComponentType> component_type)
    : SlangComponentType(std::move(module), std::move(component_type))
{
    slang::EntryPointLayout* layout = m_component_type->getLayout()->getEntryPointByIndex(0);
    m_name = layout->getNameOverride() ? layout->getNameOverride() : layout->getName();
    m_stage = get_shader_stage(layout->getStage());
}

ref<SlangEntryPoint> SlangEntryPoint::rename(const std::string& new_name)
{
    Slang::ComPtr<slang::IComponentType> renamed_entry_point;
    SLANG_CALL(m_component_type->renameEntryPoint(new_name.c_str(), renamed_entry_point.writeRef()));
    return make_ref<SlangEntryPoint>(m_module, renamed_entry_point);
}

const EntryPointLayout* SlangEntryPoint::layout() const
{
    return EntryPointLayout::from_slang(m_component_type->getLayout()->getEntryPointByIndex(0));
}

std::string SlangEntryPoint::to_string() const
{
    return fmt::format("SlangEntryPoint(name=\"{}\", stage={})", m_name, m_stage);
}

ShaderProgram::ShaderProgram(
    ref<Device> device,
    ref<SlangGlobalScope> global_scope,
    std::vector<ref<SlangEntryPoint>> entry_points
)
    : m_device(std::move(device))
    , m_global_scope(std::move(global_scope))
    , m_entry_points(std::move(entry_points))
{
    slang::IComponentType* global_scope_component = m_global_scope->m_component_type;
    std::vector<slang::IComponentType*> entry_point_components(m_entry_points.size());
    for (size_t i = 0; i < m_entry_points.size(); ++i) {
        entry_point_components[i] = m_entry_points[i]->m_component_type;
    }

    // std::vector<Slang::ComPtr<slang::IComponentType>> linked_entry_points(m_entry_points.size());
    // std::vector<slang::IComponentType*> entry_point_components(m_entry_points.size());
    // for (size_t i = 0; i < entry_points.size(); ++i) {
    //     slang::IComponentType* components[2] = {global_scope_component, m_entry_points[i]->m_component_type};
    //     // TODO handle diagnostics
    //     SLANG_CALL(global_scope_component->getSession()->createCompositeComponentType(
    //         components,
    //         std::size(components),
    //         linked_entry_points[i].writeRef(),
    //         nullptr
    //     ));
    //     entry_point_components[i] = linked_entry_points[i];
    // }

    gfx::IShaderProgram::Desc gfx_desc{
        .linkingStyle = gfx::IShaderProgram::LinkingStyle::SeparateEntryPointCompilation,
        .slangGlobalScope = global_scope_component,
        .entryPointCount = narrow_cast<gfx::GfxCount>(entry_point_components.size()),
        .slangEntryPoints = entry_point_components.data(),
    };

    Slang::ComPtr<ISlangBlob> diagnostics;
    if (m_device->get_gfx_device()->createProgram(gfx_desc, m_gfx_shader_program.writeRef(), diagnostics.writeRef())
        != SLANG_OK) {
        KALI_THROW(
            "Failed to create shader program:\n{}",
            diagnostics ? (const char*)diagnostics->getBufferPointer() : ""
        );
    }
    if (diagnostics) {
        log_warn("Slang compiler warnings:\n{}", (const char*)diagnostics->getBufferPointer());
    }
}

std::vector<const EntryPointLayout*> ShaderProgram::entry_point_layouts() const
{
    return {};
    // std::vector<const EntryPointLayout> layouts;
    // for (size_t i = 0; i < m_entry_points.size(); ++i)
    //     layouts.push_back(m_entry_points[i]->layout());
    // return layouts;
}

std::string ShaderProgram::to_string() const
{
    return fmt::format(
        "ShaderProgram(\n"
        "    global_scope={}\n"
        "    entry_points=[\n"
        "        {}\n"
        "    ]\n"
        ")",
        m_global_scope->to_string(),
        "" // fmt::join(m_entry_points, ",\n        ")
    );
}

} // namespace kali
