#include "program.h"
#include "device.h"

#include "core/assert.h"
#include "core/type_utils.h"
#include "core/resolver.h"

namespace kali {

inline SlangStage get_gfx_slang_stage(ShaderType shader_type)
{
    static_assert(uint32_t(ShaderType::none) == SLANG_STAGE_NONE);
    static_assert(uint32_t(ShaderType::none) == SLANG_STAGE_NONE);
    static_assert(uint32_t(ShaderType::vertex) == SLANG_STAGE_VERTEX);
    static_assert(uint32_t(ShaderType::hull) == SLANG_STAGE_HULL);
    static_assert(uint32_t(ShaderType::domain) == SLANG_STAGE_DOMAIN);
    static_assert(uint32_t(ShaderType::geometry) == SLANG_STAGE_GEOMETRY);
    static_assert(uint32_t(ShaderType::fragment) == SLANG_STAGE_FRAGMENT);
    static_assert(uint32_t(ShaderType::compute) == SLANG_STAGE_COMPUTE);
    static_assert(uint32_t(ShaderType::ray_generation) == SLANG_STAGE_RAY_GENERATION);
    static_assert(uint32_t(ShaderType::intersection) == SLANG_STAGE_INTERSECTION);
    static_assert(uint32_t(ShaderType::any_hit) == SLANG_STAGE_ANY_HIT);
    static_assert(uint32_t(ShaderType::closest_hit) == SLANG_STAGE_CLOSEST_HIT);
    static_assert(uint32_t(ShaderType::miss) == SLANG_STAGE_MISS);
    static_assert(uint32_t(ShaderType::callable) == SLANG_STAGE_CALLABLE);
    static_assert(uint32_t(ShaderType::mesh) == SLANG_STAGE_MESH);
    static_assert(uint32_t(ShaderType::amplification) == SLANG_STAGE_AMPLIFICATION);
    KALI_ASSERT(uint32_t(shader_type) <= uint32_t(ShaderType::amplification));
    return SlangStage(shader_type);
}

struct ProgramVersion : public Object { };

Program::Program(const ProgramDesc& desc, ProgramManager* program_manager)
    : m_desc(desc)
    , m_program_manager(program_manager)
{
    KALI_ASSERT(m_program_manager);

    std::string log;
    m_active_version = m_program_manager->create_program_version(*this, log);
}

ProgramManager::ProgramManager(Device* device, slang::IGlobalSession* slang_session)
    : m_device(device)
    , m_slang_session(slang_session)
{
    KALI_ASSERT(device);
    KALI_ASSERT(slang_session);

    m_disabled_warnings.push_back(15602); // #pragma once in modules
    m_disabled_warnings.push_back(30081); // implicit conversion
}

ref<Program> ProgramManager::create_program(const ProgramDesc& desc)
{
    return new Program(desc, this);
}

ref<ProgramVersion> ProgramManager::create_program_version(const Program& program, std::string& out_log) const
{
    Slang::ComPtr<slang::ICompileRequest> compile_request = create_slang_compile_request(program);
    KALI_UNUSED(compile_request);

    // Compile program.
    SlangResult result = compile_request->compile();
    out_log += compile_request->getDiagnosticOutput();
    if (SLANG_FAILED(result))
        return nullptr;

    Slang::ComPtr<slang::IComponentType> program_component;
    compile_request->getProgram(program_component.writeRef());
    // slang::ISession* session = program_component->getSession();

    // Prepare entry points.
    std::vector<Slang::ComPtr<slang::IComponentType>> entry_point_components;
    SlangUInt entry_point_index = 0;
    for (const ProgramKernelDesc& kernel_desc : program.get_desc().kernels) {
        for (const EntryPointDesc& entry_point_desc : kernel_desc.entry_points) {
            Slang::ComPtr<slang::IComponentType> entry_point;
            compile_request->getEntryPoint(entry_point_index++, entry_point.writeRef());
            KALI_ASSERT(entry_point);

            // Rename entry point in the generated code if the exported name differs from the source name.
            // This makes it possible to generate different specializations of the same source entry point,
            // for example by setting different type conformances.
            if (!entry_point_desc.export_name.empty() && entry_point_desc.export_name != entry_point_desc.name) {
                Slang::ComPtr<slang::IComponentType> renamed_entry_point;
                entry_point->renameEntryPoint(entry_point_desc.export_name.c_str(), renamed_entry_point.writeRef());
                entry_point_components.push_back(renamed_entry_point);
            } else {
                entry_point_components.push_back(entry_point);
            }
        }
    }


#if 0
    // Extract list of files referenced, for dependency-tracking purposes.
    int depFileCount = spGetDependencyFileCount(pSlangRequest);
    for (int ii = 0; ii < depFileCount; ++ii)
    {
        std::string depFilePath = spGetDependencyFilePath(pSlangRequest, ii);
        if (std::filesystem::exists(depFilePath))
            program.mFileTimeMap[depFilePath] = getFileModifiedTime(depFilePath);
    }

    // Note: the `ProgramReflection` needs to be able to refer back to the
    // `ProgramVersion`, but the `ProgramVersion` can't be initialized
    // until we have its reflection. We cut that dependency knot by
    // creating an "empty" program first, and then initializing it
    // after the reflection is created.
    //
    // TODO: There is no meaningful semantic difference between `ProgramVersion`
    // and `ProgramReflection`: they are one-to-one. Ideally in a future version
    // of Falcor they could be the same object.
    //
    // TODO @skallweit remove const cast
    ProgramVersion::SharedPtr pVersion = ProgramVersion::createEmpty(const_cast<Program*>(&program), program_component);

    // Note: Because of interactions between how `SV_Target` outputs
    // and `u` register bindings work in Slang today (as a compatibility
    // feature for Shader Model 5.0 and below), we need to make sure
    // that the entry points are included in the component type we use
    // for reflection.
    //
    // TODO: Once Slang drops that behavior for SM 5.1+, we should be able
    // to just use `program_component` for the reflection step instead
    // of `pSlangProgram`.
    //
    ComPtr<slang::IComponentType> pSlangProgram;
    spCompileRequest_getProgram(pSlangRequest, pSlangProgram.writeRef());

    ProgramReflection::SharedPtr pReflector;
    if (!doSlangReflection(*pVersion, program_component, entry_point_components, pReflector, log))
    {
        return nullptr;
    }

    auto descStr = program.getProgramDescString();
    pVersion->init(program.getDefineList(), pReflector, descStr, entry_point_components);

    timer.update();
    double time = timer.delta();
    mCompilationStats.programVersionCount++;
    mCompilationStats.programVersionTotalTime += time;
    mCompilationStats.programVersionMaxTime = std::max(mCompilationStats.programVersionMaxTime, time);
    logDebug("Created program version in {:.3f} s: {}", timer.delta(), descStr);
#endif

    return nullptr;
}


void ProgramManager::add_search_path(std::filesystem::path path)
{
    m_search_paths.push_back(std::move(path));
}

void ProgramManager::remove_search_path(std::filesystem::path path)
{
    m_search_paths.erase(std::remove(m_search_paths.begin(), m_search_paths.end(), path), m_search_paths.end());
}


Slang::ComPtr<slang::ICompileRequest> ProgramManager::create_slang_compile_request(const Program& program) const
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

    slang::TargetDesc target_desc;
    target_desc.format = SLANG_TARGET_UNKNOWN;
    target_desc.profile = m_slang_session->findProfile(to_string(program_desc.shader_model).c_str());
    KALI_ASSERT(target_desc.profile != SLANG_PROFILE_UNKNOWN);

    // Get compiler flags and adjust with forced flags.
    ShaderCompilerFlags compiler_flags = program_desc.compiler_flags;
    compiler_flags &= ~m_forced_disabled_compiler_flags;
    compiler_flags |= m_forced_enabled_compiler_flags;

    // Set floating point mode. If no shader compiler flags for this were set, we use Slang's default mode.
    bool flag_fast = is_set(compiler_flags, ShaderCompilerFlags::floating_point_mode_fast);
    bool flag_precise = is_set(compiler_flags, ShaderCompilerFlags::floating_point_mode_precise);
    if (flag_fast && flag_precise) {
        KALI_WARN("Shader compiler flags 'floating_point_mode_fast' and 'floating_point_mode_precise' can't be used "
                  "simultaneously. ignoring 'floating_point_mode_fast'.");
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
        KALI_ASSERT(false);
        break;
    }
    KALI_ASSERT(target_define);

    // Pass any `#define` flags along to Slang, since we aren't doing our
    // own preprocessing any more.
    //
    std::vector<slang::PreprocessorMacroDesc> defines;
    const auto add_define = [&defines](const char* name, const char* value) { defines.push_back({name, value}); };

    // Add global followed by program specific defines.
    for (const auto& d : m_global_defines)
        add_define(d.first.c_str(), d.second.c_str());
    for (const auto& d : program_desc.defines)
        add_define(d.first.c_str(), d.second.c_str());

    // Add a `#define`s based on the target and shader model.
    add_define(target_define, "1");
    std::string sm = "__SM_" + to_string(program_desc.shader_model).substr(3) + "__";
    add_define(sm.c_str(), "1");

    session_desc.preprocessorMacros = defines.data();
    session_desc.preprocessorMacroCount = narrow_cast<SlangInt>(defines.size());

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
    m_slang_session->createSession(session_desc, session.writeRef());
    KALI_ASSERT(session);

#if 0
    program.mFileTimeMap.clear(); // TODO @skallweit
#endif

#if 0
    if (!program.mDesc.mLanguagePrelude.empty()) {
        if (target_desc.format == SLANG_DXIL) {
            m_slang_session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_HLSL, program.mDesc.mLanguagePrelude.c_str());
        } else {
            reportError("Language prelude set for unsupported target " + std::string(targetMacroName));
            return nullptr;
        }
    }
#endif

    // Create compile request.
    Slang::ComPtr<slang::ICompileRequest> compile_request;
    session->createCompileRequest(compile_request.writeRef());
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
            compile_request->processCommandLineArguments(args.data(), narrow_cast<int>(args.size()));
    }

    for (size_t i = 0; i < program_desc.modules.size(); ++i) {
        const ShaderModuleDesc& module_desc = program_desc.modules[i];
        int translation_unit_index = compile_request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
        KALI_ASSERT(translation_unit_index == i);
        for (const ShaderSourceDesc& source_desc : module_desc.sources) {
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
    for (const ProgramKernelDesc& kernel_desc : program_desc.kernels) {
        for (const EntryPointDesc& entry_point_desc : kernel_desc.entry_points) {
            compile_request->addEntryPoint(
                narrow_cast<int>(kernel_desc.module_index),
                entry_point_desc.name.c_str(),
                get_gfx_slang_stage(entry_point_desc.type)
            );
        }
    }


#if 0
    // Now lets add all our input shader code, one-by-one
    int translationUnitsAdded = 0;
    int translationUnitIndex = -1;

    for (const auto& src : program.mDesc.mSources) {
        // Register new translation unit with Slang if needed.
        if (translationUnitIndex < 0 || src.source.createTranslationUnit) {
            // If module name is empty, pass in nullptr to let Slang generate a name internally.
            const char* name = !src.source.moduleName.empty() ? src.source.moduleName.c_str() : nullptr;
            translationUnitIndex = spAddTranslationUnit(compile_request, SLANG_SOURCE_LANGUAGE_SLANG, name);
            FALCOR_ASSERT(translationUnitIndex == translationUnitsAdded);
            translationUnitsAdded++;
        }
        FALCOR_ASSERT(translationUnitIndex >= 0);

        // Add source code to the translation unit
        if (src.getType() == Program::ShaderModule::Type::File) {
            // If this is not an HLSL or a SLANG file, display a warning
            const auto& path = src.source.filePath;
            if (!(hasExtension(path, "hlsl") || hasExtension(path, "slang"))) {
                logWarning("Compiling a shader file which is not a SLANG file or an HLSL file. This is not an error, "
                           "but make sure that the file "
                           "contains valid shaders");
            }
            std::filesystem::path fullPath;
            if (!findFileInShaderDirectories(path, fullPath)) {
                reportError("Can't find file " + path.string());
                spDestroyCompileRequest(compile_request);
                return nullptr;
            }
            spAddTranslationUnitSourceFile(compile_request, translationUnitIndex, fullPath.string().c_str());
        } else {
            FALCOR_ASSERT(src.getType() == Program::ShaderModule::Type::String);
            spAddTranslationUnitSourceString(
                compile_request,
                translationUnitIndex,
                src.source.modulePath.c_str(),
                src.source.str.c_str()
            );
        }
    }

    // Now we make a separate pass and add the entry points.
    // Each entry point references the index of the source
    // it uses, and luckily, the Slang API can use these
    // indices directly.
    for (auto& entryPoint : program.mDesc.mEntryPoints) {
        spAddEntryPoint(
            compile_request,
            entryPoint.sourceIndex,
            entryPoint.name.c_str(),
            getSlangStage(entryPoint.stage)
        );
    }
#endif
    return compile_request;
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
