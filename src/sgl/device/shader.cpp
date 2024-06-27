// SPDX-License-Identifier: Apache-2.0

#include "shader.h"

#include "sgl/device/device.h"
#include "sgl/device/helpers.h"
#include "sgl/device/reflection.h"
#include "sgl/device/kernel.h"
#include "sgl/device/print.h"
#include "sgl/device/slang_utils.h"
#include "sgl/device/pipeline.h"

#include "sgl/core/type_utils.h"
#include "sgl/core/platform.h"
#include "sgl/core/string.h"
#include "sgl/core/crypto.h"
#include "sgl/core/timer.h"
#include "sgl/core/file_stream.h"

#include <slang.h>

#include <random>

namespace sgl {

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

    void insert_cache_include(const std::filesystem::path& path, size_t index)
    {
        SGL_ASSERT(index <= m_entries.size());
        m_entries.insert(m_entries.begin() + index,
            {
            .name = slang::CompilerOptionName::Include,
            .value = {
                .kind = slang::CompilerOptionValueKind::String,
                .string0 = path.string(),
            },
        });
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
    m_device->_register_slang_session(this);
    recreate_session();
}

SlangSession::~SlangSession()
{
    m_device->_unregister_slang_session(this);
}

void SlangSession::recreate_session()
{
    SGL_CHECK_NOT_NULL(m_device);

    ConstructorRefGuard ref_guard(this);

    DeviceType device_type = m_device->type();

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
    SGL_CHECK(
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

    // Set optimization and debug options.
    session_options.add(slang::CompilerOptionName::DebugInformation, int(options.debug_info));
    session_options.add(slang::CompilerOptionName::Optimization, int(options.optimization));

    // Set downstream arguments.
    if (device_type == DeviceType::d3d12) {
        for (const auto& arg : options.downstream_args)
            session_options.add(slang::CompilerOptionName::DownstreamArgs, "dxc", arg);
    }

    // Set intermediate dump options.
    session_options.add(slang::CompilerOptionName::DumpIntermediates, options.dump_intermediates);
    session_options.add(slang::CompilerOptionName::DumpIntermediatePrefix, options.dump_intermediates_prefix);

    // TODO: We enable loop inversion as it was the default in older versions of Slang,
    //       and leads to artifacts in one project using sgl.
    session_options.add(slang::CompilerOptionName::LoopInversion, true);

    // Only use up to date binary modules (required for caching to work properly).
    session_options.add(slang::CompilerOptionName::UseUpToDateBinaryModule, true);

    slang::SessionDesc session_desc{};

    // Set include paths.
    // Slang will search for files in the order they are specified.
    // Use provided include paths first, followed by default include paths.
    // Keep a copy of the include paths for local resolution of module names.
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
    SGL_CHECK(target_desc.profile != SLANG_PROFILE_UNKNOWN, "Unsupported target profile: {}", profile_str);

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
        SGL_UNREACHABLE();
    }
    SGL_ASSERT(target_define);

    // Add target define.
    session_options.add_macro_define(target_define, "1");

    // Add shader model defines.
    session_options.add_macro_define("__SHADER_TARGET_MAJOR", fmt::format("{}", shader_model_major));
    session_options.add_macro_define("__SHADER_TARGET_MINOR", fmt::format("{}", shader_model_minor));

    // Add NVAPI defines.
    session_options.add_macro_define(
        "SGL_ENABLE_NVAPI",
        (SGL_HAS_NVAPI && m_device->type() == DeviceType::d3d12) ? "1" : "0"
    );
#if SGL_HAS_NVAPI
    session_options.add_macro_define("NV_SHADER_EXTN_SLOT", "u999");
    session_options.add(
        slang::CompilerOptionName::DownstreamArgs,
        "dxc",
        fmt::format("-I{}", (platform::runtime_directory() / "shaders/nvapi").string())
    );
#endif

    // Add device print enable flag.
    session_options.add_macro_define("SGL_ENABLE_PRINT", m_device->desc().enable_print ? "1" : "0");

    auto slang_target_option_entries = target_options.slang_entries();
    target_desc.compilerOptionEntries = slang_target_option_entries.data();
    target_desc.compilerOptionEntryCount = narrow_cast<uint32_t>(slang_target_option_entries.size());

    session_desc.targets = &target_desc;
    session_desc.targetCount = 1;

    auto slang_session_option_entries = session_options.slang_entries();
    session_desc.compilerOptionEntries = slang_session_option_entries.data();
    session_desc.compilerOptionEntryCount = narrow_cast<uint32_t>(slang_session_option_entries.size());

    // Use the session descriptor to generate a hash that uniquely identifies the session.
    Slang::ComPtr<ISlangBlob> session_digest;
    SLANG_CALL(m_device->global_session()->getSessionDescDigest(&session_desc, session_digest.writeRef()));
    m_uid = string::hexlify(session_digest->getBufferPointer(), session_digest->getBufferSize());

    // Setup session cache.
    // The session cache relies on Slang's ability to serialize shader modules.
    // We use the session digest to create a unique cache directory for the session.
    // When loading a shader module, we store a serialized version ('.slang-module' file) to the cache.
    // Next time the module is loaded, Slang can detect the cached file and load it directly.
    // Cached modules are stored in a directory structure that mirrors the include paths.
    if (m_desc.cache_path) {
        m_cache_enabled = true;
        m_cache_path = *m_desc.cache_path;
        SGL_CHECK(m_cache_path.is_absolute(), "Cache path must be an absolute path");
        m_cache_path /= m_uid;
        m_cache_include_paths.resize(m_include_paths.size());
        for (size_t i = 0; i < m_include_paths.size(); ++i) {
            m_cache_include_paths[i] = m_cache_path / fmt::format("{}", i);
            std::filesystem::create_directories(m_cache_include_paths[i]);
            session_options.insert_cache_include(m_cache_include_paths[i], i);
        }

        // Update session descriptor with the patched include paths.
        slang_session_option_entries = session_options.slang_entries();
        session_desc.compilerOptionEntries = slang_session_option_entries.data();
        session_desc.compilerOptionEntryCount = narrow_cast<uint32_t>(slang_session_option_entries.size());
    }

    SLANG_CALL(m_device->global_session()->createSession(session_desc, m_slang_session.writeRef()));

    // Load NVAPI module if available.
    // We link this to all programs because slang uses NVAPI features while not including NVAPI itself.
    if (SGL_HAS_NVAPI && m_device->type() == DeviceType::d3d12) {
        m_nvapi_module = load_module("sgl/device/nvapi.slang");
        m_nvapi_module->break_strong_reference_to_session();
    }

    recreate_modules();
    recreate_programs();
}

ref<SlangModule> SlangSession::load_module(std::string_view module_name)
{
    SlangModuleDesc desc;
    desc.module_name = module_name;

    ref<SlangModule> module = make_ref<SlangModule>(ref(this), desc);
    recreate_module(module.get());
    return module;
}

ref<SlangModule> SlangSession::load_module_from_source(
    std::string_view module_name,
    std::string_view source,
    std::optional<std::filesystem::path> path
)
{
    SlangModuleDesc desc;
    desc.module_name = module_name;
    desc.source = source;
    desc.path = path;

    ref<SlangModule> module = make_ref<SlangModule>(ref(this), desc);
    recreate_module(module.get());
    return module;
}

ref<ShaderProgram> SlangSession::link_program(
    std::vector<ref<SlangModule>> modules,
    std::vector<ref<SlangEntryPoint>> entry_points,
    std::optional<SlangLinkOptions> link_options
)
{
    for (const auto& module : modules)
        SGL_CHECK(module->session() == this, "All modules must belong to this session.");
    for (const auto& entry_point : entry_points)
        SGL_CHECK(entry_point->module()->session() == this, "All entry points must belong to this session.");

    // Link NVAPI module if available.
    if (SGL_HAS_NVAPI && m_device->type() == DeviceType::d3d12)
        modules.push_back(m_nvapi_module);

    ShaderProgramDesc desc;
    desc.modules = modules;
    desc.entry_points = entry_points;
    desc.link_options = link_options;

    auto program = make_ref<ShaderProgram>(ref(device()), ref(this), desc);

    recreate_program(program.get());

    return program;
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
    // TODO improve the way we generate unique names for additional sources
    static uint32_t id = 0;
    if (additional_source)
        modules.push_back(load_module_from_source(fmt::format("additional_source_{}", id++), *additional_source));
    std::vector<ref<SlangEntryPoint>> entry_points;
    for (const auto& entry_point_name : entry_point_names)
        entry_points.push_back(module->entry_point(entry_point_name));
    return link_program(std::move(modules), std::move(entry_points), link_options);
}

std::string SlangSession::load_source(std::string_view module_name)
{
    std::string resolved_name = resolve_module_name(module_name);

    for (const std::filesystem::path& include_path : m_include_paths) {
        std::filesystem::path full_path = include_path / resolved_name;
        if (std::filesystem::exists(full_path)) {
            FileStream stream(full_path, FileStream::Mode::read);
            std::string result;
            result.resize(stream.size());
            stream.read(result.data(), result.size());
            return result;
        }
    }
    SGL_THROW("Failed to load source for module \"{}\"", module_name);
}

void SlangSession::_register_program(ShaderProgram* program)
{
    m_registered_programs.insert(program);
}

void SlangSession::_unregister_program(ShaderProgram* program)
{
    m_registered_programs.erase(program);
}

void SlangSession::_register_module(SlangModule* module)
{
    m_registered_modules.insert(module);
}

void SlangSession::_unregister_module(SlangModule* module)
{
    m_registered_modules.erase(module);
}

void SlangSession::recreate_modules()
{
    for (SlangModule* module : m_registered_modules)
        recreate_module(module);
}

void SlangSession::recreate_modules_referencing(const std::set<std::filesystem::path>& paths)
{
    int dependencies = 0;
    for (int i = 0; i < m_slang_session->getLoadedModuleCount(); ++i) {
        slang::IModule* slang_module = m_slang_session->getLoadedModule(i);
        std::filesystem::path module_path = slang_module->getFilePath();
        if (paths.contains(module_path)) {
            log_info("Hot reload, module {} changed", module_path);
            dependencies++;
        }
    }
    if (dependencies > 0)
        recreate_session();
}

void SlangSession::recreate_module(SlangModule* module)
{
    module->load();
    update_module_cache();
    module->recreate_entry_points();
}

void SlangSession::recreate_programs()
{
    for (ShaderProgram* program : m_registered_programs)
        recreate_program(program);
}

void SlangSession::recreate_program(ShaderProgram* program)
{
    program->link();
}

std::string SlangSession::to_string() const
{
    return fmt::format(
        "SlangSession(\n"
        "  device = {},\n"
        "  uid = {}\n"
        ")",
        m_device,
        m_uid
    );
}

void SlangSession::update_module_cache()
{
    if (!m_cache_enabled)
        return;

    // Cache newly loaded modules.
    for (int i = 0; i < m_slang_session->getLoadedModuleCount(); ++i) {
        slang::IModule* slang_module = m_slang_session->getLoadedModule(i);
        if (m_loaded_modules.contains(slang_module))
            continue;
        write_module_to_cache(slang_module);
        m_loaded_modules.insert(slang_module);
    }
}

bool SlangSession::write_module_to_cache(slang::IModule* module)
{
    std::filesystem::path path{module->getFilePath()};

    // Do not cache module if it was loaded from the cache.
    if (path.extension() == ".slang-module")
        return false;

    // Check if target path is within the specified root path.
    auto is_sub_path = [](const std::filesystem::path root, const std::filesystem::path& target)
    {
        std::string relative = std::filesystem::relative(target, root).string();
        return relative.size() == 1 || (relative.size() > 1 && relative[0] != '.' && relative[1] != '.');
    };

    // Find the include path that contains the module.
    SGL_ASSERT(m_include_paths.size() == m_cache_include_paths.size());
    size_t include_index = size_t(-1);
    for (size_t i = 0; i < m_include_paths.size(); ++i) {
        if (is_sub_path(m_include_paths[i], path)) {
            include_index = i;
            break;
        }
    }
    if (include_index == size_t(-1))
        return false;

    // Determine path of the cached module.
    std::filesystem::path relative = std::filesystem::relative(path, m_include_paths[include_index]);
    std::filesystem::path cache_path = m_cache_include_paths[include_index] / relative;
    cache_path.replace_extension(".slang-module");

    // Create directories to cache path.
    std::error_code ec;
    std::filesystem::create_directories(cache_path.parent_path(), ec);
    if (ec) {
        log_warn(
            "Failed to create directory \"{}\" for slang module cache ({})",
            cache_path.parent_path(),
            ec.message()
        );
        return false;
    }

    // Write module to a temporary file.
    std::filesystem::path tmp_path = cache_path;
    std::random_device rd;
    uint64_t uid = rd();
    tmp_path.replace_extension(".slang-module-" + string::hexlify(&uid, sizeof(uid)));
    if (std::filesystem::exists(tmp_path))
        return false;
    if (!SLANG_SUCCEEDED(module->writeToFile(tmp_path.string().c_str()))) {
        log_warn("Failed to write cached slang module \"{}\" to \"{}\"", module->getName(), cache_path);
        return false;
    }

    // Rename temporary file to cache path.
    std::filesystem::rename(tmp_path, cache_path, ec);
    if (ec) {
        log_warn("Failed to rename cached slang module \"{}\" to \"{}\" ({})", tmp_path, cache_path, ec.message());
        std::filesystem::remove(tmp_path, ec);
        return false;
    }

    log_debug("Cached slang module \"{}\" to \"{}\"", module->getName(), cache_path);

    return true;
}

std::string SlangSession::resolve_module_name(std::string_view module_name)
{
    // Return if module name is an absolute file path.
    std::filesystem::path path = module_name;
    if (path.is_absolute())
        return path.string();

    // Return relative path if module name is a relative path within the include paths.
    for (const std::filesystem::path& include_path : m_include_paths) {
        std::filesystem::path full_path = include_path / path;
        if (std::filesystem::exists(full_path))
            return path.string();
    }

    // Assume we have a module name in the form "a.b.c" which we interpret as "a/b/c.slang".
    std::string str{module_name};
    std::replace(str.begin(), str.end(), '.', '/');
    str += ".slang";
    path = str;
    for (const std::filesystem::path& include_path : m_include_paths) {
        std::filesystem::path full_path = include_path / path;
        if (std::filesystem::exists(full_path))
            return path.string();
    }

    // Failed to resolve, return the module name as is.
    return std::string{module_name};
}

void SlangSession::register_with_debug_printer(SlangModule* module) const
{
    SGL_ASSERT(module);
    if (m_device->debug_printer())
        m_device->debug_printer()->add_hashed_strings(module->layout()->hashed_strings_map());
}

SlangModule::SlangModule(ref<SlangSession> session, const SlangModuleDesc& desc)
    : m_session(std::move(session))
    , m_desc(desc)
{
    m_session->_register_module(this);
}

SlangModule::~SlangModule()
{
    m_session->_unregister_module(this);
}

void SlangModule::load()
{
    Timer timer;
    Slang::ComPtr<ISlangBlob> diagnostics;
    slang::IModule* slang_module;

    // Load module either from resolved name or source depending on whether source specified
    if (!m_desc.source.has_value()) {
        std::string resolved_name = m_session->resolve_module_name(m_desc.module_name);
        slang_module = m_session->get_slang_session()->loadModule(resolved_name.c_str(), diagnostics.writeRef());
        if (!slang_module) {
            std::string msg = append_diagnostics(
                fmt::format("Failed to load slang module \"{}\"", m_desc.module_name),
                diagnostics
            );
            throw SlangCompileError(msg);
        }
    } else {
        // TODO workaround: slang doesn't like loading the same source twice
        static uint32_t id = 0;
        std::string source_str = fmt::format("// {}\n{}", id++, m_desc.source);

        slang_module = m_session->get_slang_session()->loadModuleFromSourceString(
            std::string{m_desc.module_name}.c_str(),
            m_desc.path ? m_desc.path->string().c_str() : nullptr,
            source_str.c_str(),
            diagnostics.writeRef()
        );
        if (!slang_module) {
            std::string msg = append_diagnostics(
                fmt::format("Failed to load slang module \"{}\" from source", m_desc.module_name),
                diagnostics
            );
            throw SlangCompileError(msg);
        }
    }

    report_diagnostics(diagnostics);
    log_debug("Loading slang module \"{}\" took {}", m_desc.module_name, string::format_duration(timer.elapsed_s()));
    register_with_debug_printer();

    // Store initialized module info
    m_slang_module = slang_module;
    m_name = m_slang_module->getName();
    m_path = m_slang_module->getFilePath() ? m_slang_module->getFilePath() : "";

    // In case this was a re-load, tell any existing entry points to recreate
    recreate_entry_points();
}

void SlangModule::register_with_debug_printer() const
{
    if (m_session->device()->debug_printer())
        m_session->device()->debug_printer()->add_hashed_strings(layout()->hashed_strings_map());
}

std::vector<ref<SlangEntryPoint>> SlangModule::entry_points() const
{
    std::vector<ref<SlangEntryPoint>> entry_points;
    for (SlangInt32 i = 0; i < m_slang_module->getDefinedEntryPointCount(); ++i) {
        Slang::ComPtr<slang::IEntryPoint> slang_entry_point;
        m_slang_module->getDefinedEntryPoint(i, slang_entry_point.writeRef());

        slang::EntryPointLayout* layout = slang_entry_point->getLayout()->getEntryPointByIndex(0);
        std::string name = layout->getNameOverride() ? layout->getNameOverride() : layout->getName();

        entry_points.push_back(entry_point(name));
    }
    return entry_points;
}

ref<SlangEntryPoint> SlangModule::entry_point(std::string_view name) const
{
    SlangEntryPointDesc desc;
    desc.name = name;

    auto entry_point = make_ref<SlangEntryPoint>(ref(const_cast<SlangModule*>(this)), desc);

    recreate_entry_point(entry_point.get());

    return entry_point;
}

bool SlangModule::has_entry_point(std::string_view name) const
{
    Slang::ComPtr<slang::IEntryPoint> slang_entry_point;
    m_slang_module->findEntryPointByName(std::string{name}.c_str(), slang_entry_point.writeRef());
    return slang_entry_point != nullptr;
}

void SlangModule::recreate_entry_points()
{
    for (SlangEntryPoint* entry_point : m_registered_entry_points)
        recreate_entry_point(entry_point);
}

void SlangModule::recreate_entry_point(SlangEntryPoint* entry_point) const
{
    const SlangEntryPointDesc& desc = entry_point->desc();
    Slang::ComPtr<slang::IEntryPoint> slang_entry_point;
    m_slang_module->findEntryPointByName(std::string{desc.name}.c_str(), slang_entry_point.writeRef());
    if (!slang_entry_point)
        SGL_THROW("Entry point \"{}\" not found", desc.name);
    entry_point->init(Slang::ComPtr<slang::IComponentType>(slang_entry_point));
}

void SlangModule::_register_entry_point(SlangEntryPoint* entry_point) const
{
    m_registered_entry_points.insert(entry_point);
}

void SlangModule::_unregister_entry_point(SlangEntryPoint* entry_point) const
{
    m_registered_entry_points.erase(entry_point);
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

SlangEntryPoint::SlangEntryPoint(ref<SlangModule> module, const SlangEntryPointDesc& desc)
    : m_module(module)
    , m_desc(desc)
    , m_stage(ShaderStage::none)
{
    m_module->_register_entry_point(this);
}

SlangEntryPoint::~SlangEntryPoint()
{
    m_module->_unregister_entry_point(this);
}

void SlangEntryPoint::init(Slang::ComPtr<slang::IComponentType> slang_entry_point)
{
    m_slang_entry_point = std::move(slang_entry_point);
    slang::EntryPointLayout* layout = m_slang_entry_point->getLayout()->getEntryPointByIndex(0);
    m_name = layout->getNameOverride() ? layout->getNameOverride() : layout->getName();
    m_stage = static_cast<ShaderStage>(layout->getStage());
}

ref<SlangEntryPoint> SlangEntryPoint::rename(const std::string& new_name)
{
    Slang::ComPtr<slang::IComponentType> renamed_entry_point;
    SLANG_CALL(m_slang_entry_point->renameEntryPoint(new_name.c_str(), renamed_entry_point.writeRef()));

    SlangEntryPointDesc desc;
    desc.name = new_name;
    auto ep = make_ref<SlangEntryPoint>(m_module, desc);
    ep->init(renamed_entry_point);

    return ep;
}

ref<SlangEntryPoint> SlangEntryPoint::with_name(const std::string& name) const
{
    Slang::ComPtr<slang::IComponentType> new_entry_point;
    SLANG_CALL(m_slang_entry_point->renameEntryPoint(name.c_str(), new_entry_point.writeRef()));

    SlangEntryPointDesc desc;
    desc.name = name;
    auto ep = make_ref<SlangEntryPoint>(m_module, desc);
    ep->init(new_entry_point);

    return ep;
}

ref<SlangEntryPoint> SlangEntryPoint::with_type_conformances(const TypeConformanceList& type_conformances) const
{
    SGL_UNUSED(type_conformances);
    // TODO
    // m_module->m_session->get_slang_session()->createTypeConformanceComponentType

    SlangEntryPointDesc desc;
    return make_ref<SlangEntryPoint>(m_module, desc);
}

const EntryPointLayout* SlangEntryPoint::layout() const
{
    return EntryPointLayout::from_slang(m_slang_entry_point->getLayout()->getEntryPointByIndex(0));
}

std::string SlangEntryPoint::to_string() const
{
    return fmt::format("SlangEntryPoint(name=\"{}\", stage={})", m_name, m_stage);
}

ShaderProgram::ShaderProgram(ref<Device> device, ref<SlangSession> session, const ShaderProgramDesc& desc)
    : DeviceResource(std::move(device))
    , m_session(std::move(session))
    , m_desc(desc)
{
    m_session->_register_program(this);
}

ShaderProgram::~ShaderProgram()
{
    m_session->_unregister_program(this);
}


void ShaderProgram::link()
{
    const ShaderProgramDesc& desc = m_desc;
    slang::ISession* session = m_session->get_slang_session();

    Timer timer;

    // Compose the program from it's components.
    Slang::ComPtr<slang::IComponentType> composed_program;
    {
        std::vector<slang::IComponentType*> component_types;
        for (const auto& module : desc.modules)
            component_types.push_back(module->slang_module());
        for (const auto& entry_point : desc.entry_points)
            component_types.push_back(entry_point->slang_entry_point());
        Slang::ComPtr<ISlangBlob> diagnostics;
        session->createCompositeComponentType(
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
    if (desc.link_options) {
        const SlangLinkOptions& link_options = desc.link_options.value();

        if (link_options.floating_point_mode)
            link_option_entries.add(
                slang::CompilerOptionName::FloatingPointMode,
                int(*link_options.floating_point_mode)
            );
        if (link_options.debug_info)
            link_option_entries.add(slang::CompilerOptionName::DebugInformation, int(*link_options.debug_info));
        if (link_options.optimization)
            link_option_entries.add(slang::CompilerOptionName::Optimization, int(*link_options.optimization));
        if (link_options.downstream_args) {
            if (m_device->type() == DeviceType::d3d12) {
                for (const auto& arg : *link_options.downstream_args)
                    link_option_entries.add(slang::CompilerOptionName::DownstreamArgs, "dxc", arg);
            }
        }
        if (link_options.dump_intermediates)
            link_option_entries.add(slang::CompilerOptionName::DumpIntermediates, *link_options.dump_intermediates);
        if (link_options.dump_intermediates_prefix)
            link_option_entries.add(
                slang::CompilerOptionName::DumpIntermediatePrefix,
                *link_options.dump_intermediates_prefix
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

    // Create shader program.
    Slang::ComPtr<gfx::IShaderProgram> gfx_shader_program;
    {
        gfx::IShaderProgram::Desc gfx_desc{
            .slangGlobalScope = linked_program,
        };

        Slang::ComPtr<ISlangBlob> diagnostics;
        if (m_device->gfx_device()->createProgram(gfx_desc, gfx_shader_program.writeRef(), diagnostics.writeRef())
            != SLANG_OK) {
            std::string msg = append_diagnostics("Failed to create shader program", diagnostics);
            SGL_THROW(msg);
        }
        report_diagnostics(diagnostics);
    }

    // Report link time.
    std::string name;
    for (const auto& entry_point : desc.entry_points)
        name += (name.empty() ? "" : ", ") + entry_point->module()->name() + ":" + entry_point->name();
    log_debug("Linking shader program \"{}\" took {}", name, string::format_duration(timer.elapsed_s()));

    // Store program info
    m_linked_program = linked_program;
    m_gfx_shader_program = gfx_shader_program;

    // If this is a hot reload, notify pipelines that the program has refreshed so they may need to rebuild
    for (Pipeline* pipeline : m_registered_pipelines)
        pipeline->notify_program_reloaded();
}

void ShaderProgram::_register_pipeline(Pipeline* pipeline)
{
    m_registered_pipelines.insert(pipeline);
}

void ShaderProgram::_unregister_pipeline(Pipeline* pipeline)
{
    m_registered_pipelines.erase(pipeline);
}

std::string ShaderProgram::to_string() const
{
    return fmt::format(
        "ShaderProgram(\n"
        "  modules = {},\n"
        "  entry_points = {}\n"
        ")",
        string::indent(string::list_to_string(m_desc.modules)),
        string::indent(string::list_to_string(m_desc.entry_points))
    );
}

} // namespace sgl
