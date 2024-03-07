// SPDX-License-Identifier: Apache-2.0

#include "shader.h"

#include "kali/device/device.h"
#include "kali/device/helpers.h"
#include "kali/device/reflection.h"
#include "kali/device/kernel.h"
#include "kali/device/print.h"
#include "kali/device/slang_utils.h"

#include "kali/core/type_utils.h"
#include "kali/core/platform.h"
#include "kali/core/string.h"
#include "kali/core/crypto.h"

#include <slang.h>
#include <slang-tag-version.h>

namespace kali {

// ----------------------------------------------------------------------------
// TypeConformanceList
// ----------------------------------------------------------------------------

TypeConformanceList& TypeConformanceList::add(std::string type_name, std::string interface_name, uint32_t id)
{
    (*this)[TypeConformance{std::move(type_name), std::move(interface_name)}] = id;
    return *this;
}

TypeConformanceList& TypeConformanceList::remove(std::string type_name, std::string interface_name)
{
    (*this).erase(TypeConformance{std::move(type_name), std::move(interface_name)});
    return *this;
}

TypeConformanceList& TypeConformanceList::add(const TypeConformanceList& other)
{
    for (const auto& p : other)
        add(p.first.type_name, p.first.interface_name, p.second);
    return *this;
}

TypeConformanceList& TypeConformanceList::remove(const TypeConformanceList& other)
{
    for (const auto& p : other)
        remove(p.first.type_name, p.first.interface_name);
    return *this;
}

// ----------------------------------------------------------------------------
// CompilerOptionEntries
// ----------------------------------------------------------------------------

/// Helper class for creating slang::CompilerOptionEntry entries.
class CompilerOptionEntries {
public:
    void add(slang::CompilerOptionName name, bool value)
    {
        m_entries.push_back({
            .name = name,
            .value = {
                .kind = slang::CompilerOptionValueKind::Int,
                .int0 = value ? 1 : 0,
            },
        });
    }

    void add(slang::CompilerOptionName name, std::string_view value)
    {
        m_entries.push_back({
            .name = name,
            .value = {
                .kind = slang::CompilerOptionValueKind::String,
                .string0 = std::string{value},
            },
        });
    }

    void add(slang::CompilerOptionName name, std::string_view value0, std::string_view value1)
    {
        m_entries.push_back({
            .name = name,
            .value = {
                .kind = slang::CompilerOptionValueKind::String,
                .string0 = std::string{value0},
                .string1 = std::string{value1},
            },
        });
    }

    void add_macro_define(std::string_view name, std::string_view value)
    {
        add(slang::CompilerOptionName::MacroDefine, name, value);
    }

    void add_include(const std::filesystem::path& path) { add(slang::CompilerOptionName::Include, path.string()); }

    void hash(SHA1& hash) const
    {
        for (const auto& entry : m_entries) {
            hash.update(entry.name);
            hash.update(entry.value.kind);
            switch (entry.value.kind) {
            case slang::CompilerOptionValueKind::Int:
                hash.update(entry.value.int0);
                hash.update(entry.value.int1);
                break;
            case slang::CompilerOptionValueKind::String:
                hash.update(entry.value.string0);
                hash.update(entry.value.string1);
                break;
            }
        }
    }

    std::vector<slang::CompilerOptionEntry> slang_entries() const
    {
        std::vector<slang::CompilerOptionEntry> result{m_entries.size()};
        for (size_t i = 0; i < m_entries.size(); ++i) {
            result[i].name = m_entries[i].name;
            result[i].value.kind = m_entries[i].value.kind;
            switch (m_entries[i].value.kind) {
            case slang::CompilerOptionValueKind::Int:
                result[i].value.intValue0 = m_entries[i].value.int0;
                result[i].value.intValue1 = m_entries[i].value.int1;
                break;
            case slang::CompilerOptionValueKind::String:
                result[i].value.stringValue0 = m_entries[i].value.string0.c_str();
                result[i].value.stringValue1 = m_entries[i].value.string1.c_str();
                break;
            }
        }
        return result;
    }

private:
    struct Entry {
        slang::CompilerOptionName name;
        struct {
            slang::CompilerOptionValueKind kind;
            int int0{0};
            int int1{0};
            std::string string0;
            std::string string1;
        } value;
    };

    std::vector<Entry> m_entries;
};

// ----------------------------------------------------------------------------
// SlangSession
// ----------------------------------------------------------------------------

/// Append slang diagnostics to a string if not null.
inline std::string append_diagnostics(std::string msg, ISlangBlob* diagnostics)
{
    if (diagnostics)
        msg += fmt::format("\n{}", static_cast<const char*>(diagnostics->getBufferPointer()));
    return msg;
}

/// Report slang diagnostics to log if not null.
inline void report_diagnostics(ISlangBlob* diagnostics)
{
    if (diagnostics)
        log_warn("Slang compiler warnings:\n{}", static_cast<const char*>(diagnostics->getBufferPointer()));
}

SlangSession::SlangSession(ref<Device> device, SlangSessionDesc desc)
    : m_device(std::move(device))
    , m_desc(std::move(desc))
{
    KALI_CHECK_NOT_NULL(m_device);

    CompilerOptionEntries session_options;
    CompilerOptionEntries target_options;

    SlangCompilerOptions& options = m_desc.compiler_options;

    // Use device's highest supported shader model if none is provided explicitly.
    ShaderModel supported_shader_model = m_device->supported_shader_model();
    ShaderModel default_shader_model = supported_shader_model;
    // TODO: Slang generates invalid HLSL for SM 6.7 when using ray payloads.
    if (default_shader_model == ShaderModel::sm_6_7)
        default_shader_model = ShaderModel::sm_6_6;
    ShaderModel shader_model = options.shader_model;
    if (options.shader_model == ShaderModel::unknown)
        shader_model = default_shader_model;

    // Check that requested shader model is supported.
    KALI_CHECK(
        shader_model <= supported_shader_model,
        "Shader model {} is not supported (max shader model is {})",
        shader_model,
        supported_shader_model
    );

    // Set matrix layout.
    if (options.matrix_layout == SlangMatrixLayout::row_major)
        session_options.add(slang::CompilerOptionName::MatrixLayoutRow, true);
    else if (options.matrix_layout == SlangMatrixLayout::column_major)
        session_options.add(slang::CompilerOptionName::MatrixLayoutColumn, true);

    // Set warnings.
    for (const auto& warning : options.enable_warnings)
        session_options.add(slang::CompilerOptionName::EnableWarning, warning);
    for (const auto& warning : options.disable_warnings)
        session_options.add(slang::CompilerOptionName::DisableWarning, warning);
    for (const auto& warning : options.warnings_as_errors)
        session_options.add(slang::CompilerOptionName::WarningsAsErrors, warning);

    // Set diagnostic options.
    session_options.add(slang::CompilerOptionName::ReportDownstreamTime, options.report_downstream_time);
    session_options.add(slang::CompilerOptionName::ReportPerfBenchmark, options.report_perf_benchmark);
    session_options.add(slang::CompilerOptionName::SkipSPIRVValidation, options.skip_spirv_validation);

    DeviceType device_type = m_device->type();

    slang::SessionDesc session_desc{};

    // Set search paths.
    // Slang will search for files in the order they are specified.
    // Use provided search paths first, followed by default search paths.
    // Keep a copy of the search paths for local resolution of module names.
    m_include_paths = options.include_paths;
    if (m_desc.add_default_include_paths)
        m_include_paths.push_back(platform::runtime_directory() / "shaders");
    for (const auto& path : m_include_paths)
        session_options.add_include(path);

    // Set macro defines.
    for (const auto& define : options.defines)
        session_options.add_macro_define(define.first, define.second);

    // Select target profile.
    slang::TargetDesc target_desc;
    target_desc.format = SLANG_TARGET_UNKNOWN;
    uint32_t shader_model_major = get_shader_model_major_version(shader_model);
    uint32_t shader_model_minor = get_shader_model_minor_version(shader_model);
    std::string profile_str = fmt::format("sm_{}_{}", shader_model_major, shader_model_minor);

    target_desc.profile = m_device->global_session()->findProfile(profile_str.c_str());
    KALI_CHECK(target_desc.profile != SLANG_PROFILE_UNKNOWN, "Unsupported target profile: {}", profile_str);

    // Set floating point mode.
    target_desc.floatingPointMode = static_cast<::SlangFloatingPointMode>(options.floating_point_mode);
    target_options.add(slang::CompilerOptionName::FloatingPointMode, int(options.floating_point_mode));

    // Force GLSL scalar buffer layout.
    target_desc.forceGLSLScalarBufferLayout = true;
    target_options.add(slang::CompilerOptionName::GLSLForceScalarLayout, true);

    // Select target format based on device type.
    const char* target_define{nullptr};
    switch (device_type) {
    case DeviceType::d3d12:
        target_desc.format = SLANG_DXIL;
        target_define = "__TARGET_D3D12__";
        break;
    case DeviceType::vulkan:
        target_desc.format = SLANG_SPIRV;
        target_desc.flags |= SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;
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

    // Add target define.
    session_options.add_macro_define(target_define, "1");

    // Add shader model defines.
    session_options.add_macro_define("__SHADER_TARGET_MAJOR", fmt::format("{}", shader_model_major));
    session_options.add_macro_define("__SHADER_TARGET_MINOR", fmt::format("{}", shader_model_minor));

    // Add NVAPI defines.
    session_options.add_macro_define(
        "KALI_ENABLE_NVAPI",
        (KALI_HAS_NVAPI && m_device->type() == DeviceType::d3d12) ? "1" : "0"
    );
#if KALI_HAS_NVAPI
    session_options.add_macro_define("NV_SHADER_EXTN_SLOT", "u999");
    session_options.add(
        slang::CompilerOptionName::DownstreamArgs,
        "dxc",
        fmt::format("-I{}", (platform::runtime_directory() / "shaders/nvapi").string())
    );
#endif

    // Add device print enable flag.
    session_options.add_macro_define("KALI_ENABLE_PRINT", m_device->desc().enable_print ? "1" : "0");

    auto slang_target_option_entries = target_options.slang_entries();
    target_desc.compilerOptionEntries = slang_target_option_entries.data();
    target_desc.compilerOptionEntryCount = narrow_cast<uint32_t>(slang_target_option_entries.size());

    session_desc.targets = &target_desc;
    session_desc.targetCount = 1;

    auto slang_session_option_entries = session_options.slang_entries();
    session_desc.compilerOptionEntries = slang_session_option_entries.data();
    session_desc.compilerOptionEntryCount = narrow_cast<uint32_t>(slang_session_option_entries.size());

    // Create session uid.
    SHA1 hash;
    hash.update(SLANG_TAG_VERSION);
    session_options.hash(hash);
    target_options.hash(hash);
    m_uid = hash.hex_digest();

    SLANG_CALL(m_device->global_session()->createSession(session_desc, m_slang_session.writeRef()));
}

SlangSession::~SlangSession() { }

ref<SlangModule> SlangSession::load_module(std::string_view module_name)
{
    Slang::ComPtr<ISlangBlob> diagnostics;
    std::string resolved_name = resolve_module_name(module_name);
    slang::IModule* slang_module = m_slang_session->loadModule(resolved_name.c_str(), diagnostics.writeRef());
    if (!slang_module) {
        std::string msg = append_diagnostics(fmt::format("Failed to load module '{}'", module_name), diagnostics);
        throw SlangCompileError(msg);
    }
    report_diagnostics(diagnostics);
    ref<SlangModule> module = make_ref<SlangModule>(ref(this), slang_module);
    register_with_debug_printer(module);
    return module;
}

ref<SlangModule> SlangSession::load_module_from_source(
    std::string_view module_name,
    std::string_view source,
    std::optional<std::filesystem::path> path
)
{
    UnownedSlangBlob source_blob(source.data(), source.size());
    Slang::ComPtr<ISlangBlob> diagnostics;
    slang::IModule* slang_module = m_slang_session->loadModuleFromSource(
        std::string{module_name}.c_str(),
        path ? path->string().c_str() : nullptr,
        &source_blob,
        diagnostics.writeRef()
    );
    if (!slang_module) {
        std::string msg
            = append_diagnostics(fmt::format("Failed to load module '{}' from source'", module_name), diagnostics);
        throw SlangCompileError(msg);
    }
    report_diagnostics(diagnostics);
    ref<SlangModule> module = make_ref<SlangModule>(ref(this), slang_module);
    register_with_debug_printer(module);
    return module;
}

ref<ShaderProgram> SlangSession::link_program(
    std::vector<ref<SlangModule>> modules,
    std::vector<ref<SlangEntryPoint>> entry_points,
    std::optional<SlangLinkOptions> link_options
)
{
    for (const auto& module : modules)
        KALI_CHECK(module->session() == this, "All modules must to belong to this session.");
    for (const auto& entry_point : entry_points)
        KALI_CHECK(entry_point->module()->session() == this, "All entry points must to belong to this session.");

    // Compose the program from it's components.
    Slang::ComPtr<slang::IComponentType> composed_program;
    {
        std::vector<slang::IComponentType*> component_types;
        for (const auto& module : modules)
            component_types.push_back(module->slang_module());
        for (const auto& entry_point : entry_points)
            component_types.push_back(entry_point->slang_entry_point());
        Slang::ComPtr<ISlangBlob> diagnostics;
        m_slang_session->createCompositeComponentType(
            component_types.data(),
            component_types.size(),
            composed_program.writeRef(),
            diagnostics.writeRef()
        );
        if (!composed_program) {
            std::string msg = append_diagnostics("Failed to compose program", diagnostics);
            throw SlangCompileError(msg);
        }
        report_diagnostics(diagnostics);
    }

    // Setup link options.
    CompilerOptionEntries link_option_entries;
    std::string downstream_args;
    if (link_options) {
        if (link_options->floating_point_mode)
            link_option_entries.add(
                slang::CompilerOptionName::FloatingPointMode,
                int(*link_options->floating_point_mode)
            );
        if (link_options->debug_info)
            link_option_entries.add(slang::CompilerOptionName::DebugInformation, int(*link_options->debug_info));
        if (link_options->optimization)
            link_option_entries.add(slang::CompilerOptionName::Optimization, int(*link_options->optimization));
        if (link_options->downstream_args)
            for (const auto& arg : *link_options->downstream_args)
                link_option_entries.add(slang::CompilerOptionName::DownstreamArgs, arg);
        if (link_options->dump_intermediates)
            link_option_entries.add(slang::CompilerOptionName::DumpIntermediates, *link_options->dump_intermediates);
        if (link_options->dump_intermediates_prefix)
            link_option_entries.add(
                slang::CompilerOptionName::DumpIntermediatePrefix,
                *link_options->dump_intermediates_prefix
            );
    }

    // Link the composed program.
    Slang::ComPtr<slang::IComponentType> linked_program;
    {
        Slang::ComPtr<ISlangBlob> diagnostics;
        auto slang_link_option_entries = link_option_entries.slang_entries();
        composed_program->linkWithOptions(
            linked_program.writeRef(),
            narrow_cast<uint32_t>(slang_link_option_entries.size()),
            slang_link_option_entries.data(),
            diagnostics.writeRef()
        );
        if (!composed_program) {
            std::string msg = append_diagnostics("Failed to link program", diagnostics);
            throw SlangCompileError(msg);
        }
        report_diagnostics(diagnostics);
    }

    return make_ref<ShaderProgram>(
        ref(device()),
        ref(this),
        std::move(modules),
        std::move(entry_points),
        std::move(linked_program)
    );
}

ref<ShaderProgram> SlangSession::load_program(
    std::string_view module_name,
    std::vector<std::string_view> entry_point_names,
    std::optional<std::string_view> additional_source,
    std::optional<SlangLinkOptions> link_options
)
{
    ref<SlangModule> module = load_module(module_name);
    std::vector<ref<SlangModule>> modules{module};
    if (additional_source)
        modules.push_back(load_module_from_source("", *additional_source));
    std::vector<ref<SlangEntryPoint>> entry_points;
    for (const auto& entry_point_name : entry_point_names)
        entry_points.push_back(module->entry_point(entry_point_name));
    return link_program(std::move(modules), std::move(entry_points), link_options);
}

std::string SlangSession::to_string() const
{
    return fmt::format(
        "SlangSession(\n"
        "  device = {},\n"
        "  uid = {},\n"
        ")",
        m_device,
        m_uid
    );
}

std::string SlangSession::resolve_module_name(std::string_view module_name)
{
    // Return if module name is an absolute file path.
    std::filesystem::path path = module_name;
    if (path.is_absolute())
        return path.string();

    // Return absolute path if module name is a relative path within the search paths.
    for (const std::filesystem::path& search_path : m_include_paths) {
        std::filesystem::path full_path = search_path / path;
        if (std::filesystem::exists(full_path))
            return full_path.string();
    }

    // Assume we have a module name in the form "a.b.c" which we interpret as "a/b/c.slang".
    std::string str{module_name};
    std::replace(str.begin(), str.end(), '.', '/');
    str += ".slang";
    path = str;
    for (const std::filesystem::path& search_path : m_include_paths) {
        std::filesystem::path full_path = search_path / path;
        if (std::filesystem::exists(full_path))
            return full_path.string();
    }

    // Failed to resolve, return the module name.
    return std::string{module_name};
}

void SlangSession::register_with_debug_printer(SlangModule* module) const
{
    KALI_ASSERT(module);
    if (m_device->debug_printer())
        m_device->debug_printer()->add_hashed_strings(module->layout()->hashed_strings_map());
}


SlangModule::SlangModule(ref<SlangSession> session, slang::IModule* slang_module)
    : m_session(std::move(session))
    , m_slang_module(slang_module)
{
    m_name = m_slang_module->getName();
    m_path = m_slang_module->getFilePath() ? m_slang_module->getFilePath() : "";
}

SlangModule::~SlangModule() { }

std::vector<ref<SlangEntryPoint>> SlangModule::entry_points() const
{
    std::vector<ref<SlangEntryPoint>> entry_points;
    for (SlangInt32 i = 0; i < m_slang_module->getDefinedEntryPointCount(); ++i) {
        Slang::ComPtr<slang::IEntryPoint> slang_entry_point;
        m_slang_module->getDefinedEntryPoint(i, slang_entry_point.writeRef());
        entry_points.push_back(make_ref<SlangEntryPoint>(
            ref(const_cast<SlangModule*>(this)),
            Slang::ComPtr<slang::IComponentType>(slang_entry_point)
        ));
    }
    return entry_points;
}

ref<SlangEntryPoint> SlangModule::entry_point(std::string_view name) const
{
    Slang::ComPtr<slang::IEntryPoint> slang_entry_point;
    m_slang_module->findEntryPointByName(std::string{name}.c_str(), slang_entry_point.writeRef());
    if (!slang_entry_point)
        KALI_THROW("Entry point '{}' not found", name);
    return make_ref<SlangEntryPoint>(
        ref(const_cast<SlangModule*>(this)),
        Slang::ComPtr<slang::IComponentType>(slang_entry_point)
    );
}

bool SlangModule::has_entry_point(std::string_view name) const
{
    Slang::ComPtr<slang::IEntryPoint> slang_entry_point;
    m_slang_module->findEntryPointByName(std::string{name}.c_str(), slang_entry_point.writeRef());
    return slang_entry_point != nullptr;
}

std::string SlangModule::to_string() const
{
    return fmt::format(
        "SlangModule(\n"
        "  name = {},\n"
        "  path = {},\n"
        "  entry_points = {}\n"
        ")",
        m_name,
        m_path,
        string::indent(string::list_to_string(entry_points()))
    );
}

SlangEntryPoint::SlangEntryPoint(ref<SlangModule> module, Slang::ComPtr<slang::IComponentType> slang_entry_point)
    : m_module(std::move(module))
    , m_slang_entry_point(std::move(slang_entry_point))
{
    slang::EntryPointLayout* layout = m_slang_entry_point->getLayout()->getEntryPointByIndex(0);
    m_name = layout->getNameOverride() ? layout->getNameOverride() : layout->getName();
    m_stage = static_cast<ShaderStage>(layout->getStage());
}

ref<SlangEntryPoint> SlangEntryPoint::rename(const std::string& new_name)
{
    Slang::ComPtr<slang::IComponentType> renamed_entry_point;
    SLANG_CALL(m_slang_entry_point->renameEntryPoint(new_name.c_str(), renamed_entry_point.writeRef()));
    return make_ref<SlangEntryPoint>(m_module, renamed_entry_point);
}

ref<SlangEntryPoint> SlangEntryPoint::with_name(const std::string& name) const
{
    Slang::ComPtr<slang::IComponentType> new_entry_point;
    SLANG_CALL(m_slang_entry_point->renameEntryPoint(name.c_str(), new_entry_point.writeRef()));
    return make_ref<SlangEntryPoint>(m_module, new_entry_point);
}

ref<SlangEntryPoint> SlangEntryPoint::with_type_conformances(const TypeConformanceList& type_conformances) const
{
    KALI_UNUSED(type_conformances);
    Slang::ComPtr<slang::IComponentType> new_entry_point;
    // TODO
    // m_module->m_session->get_slang_session()->createTypeConformanceComponentType
    return make_ref<SlangEntryPoint>(m_module, new_entry_point);
}

const EntryPointLayout* SlangEntryPoint::layout() const
{
    return EntryPointLayout::from_slang(m_slang_entry_point->getLayout()->getEntryPointByIndex(0));
}

std::string SlangEntryPoint::to_string() const
{
    return fmt::format("SlangEntryPoint(name=\"{}\", stage={})", m_name, m_stage);
}

ShaderProgram::ShaderProgram(
    ref<Device> device,
    ref<SlangSession> session,
    std::vector<ref<SlangModule>> modules,
    std::vector<ref<SlangEntryPoint>> entry_points,
    Slang::ComPtr<slang::IComponentType> linked_program
)
    : m_device(std::move(device))
    , m_session(std::move(session))
    , m_modules(std::move(modules))
    , m_entry_points(std::move(entry_points))
    , m_linked_program(std::move(linked_program))
{
    gfx::IShaderProgram::Desc gfx_desc{
        .slangGlobalScope = m_linked_program,
    };

    Slang::ComPtr<ISlangBlob> diagnostics;
    if (m_device->gfx_device()->createProgram(gfx_desc, m_gfx_shader_program.writeRef(), diagnostics.writeRef())
        != SLANG_OK) {
        std::string msg = append_diagnostics("Failed to create shader program", diagnostics);
        KALI_THROW(msg);
    }
    report_diagnostics(diagnostics);
}

std::vector<const EntryPointLayout*> ShaderProgram::entry_point_layouts() const
{
    // TODO maybe remove this method
    std::vector<const EntryPointLayout*> layouts;
    for (size_t i = 0; i < m_entry_points.size(); ++i)
        layouts.push_back(m_entry_points[i]->layout());
    return layouts;
}

std::string ShaderProgram::to_string() const
{
    return fmt::format(
        "ShaderProgram(\n"
        "  modules = {},\n"
        "  entry_points = {}\n"
        ")",
        string::indent(string::list_to_string(m_modules)),
        string::indent(string::list_to_string(m_entry_points))
    );
}

} // namespace kali
