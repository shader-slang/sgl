// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "kali/device/fwd.h"
#include "kali/device/types.h"
#include "kali/device/reflection.h"

#include "kali/core/object.h"
#include "kali/core/enum.h"

#include <exception>
#include <map>
#include <vector>
#include <string>

#include <slang.h>

namespace kali {

struct TypeConformance {
    std::string type_name;
    std::string interface_name;
#ifdef KALI_MACOS
    // macOS clang stdc++ doesn't support C++20 <=> operator for standard containers yet.
    bool operator<(const TypeConformance& other) const
    {
        return std::tie(type_name, interface_name) < std::tie(other.type_name, other.interface_name);
    }
#else
    auto operator<=>(const TypeConformance&) const = default;
#endif
};

/// List of type conformances.
class KALI_API TypeConformanceList : public std::map<TypeConformance, uint32_t> {
public:
    TypeConformanceList() = default;
    TypeConformanceList(std::initializer_list<std::pair<const TypeConformance, uint32_t>> init)
        : std::map<TypeConformance, uint32_t>(init)
    {
    }
    TypeConformanceList(const TypeConformanceList& other) = default;
    TypeConformanceList(TypeConformanceList&& other) = default;

    TypeConformanceList& operator=(const TypeConformanceList& other) = default;
    TypeConformanceList& operator=(TypeConformanceList&& other) = default;

    /**
     * Adds a type conformance. If the type conformance exists, it will be replaced.
     * \param type_name The name of the implementation type.
     * \param interface_name The name of the interface type.
     * \param id Optional. The id representing the implementation type for this interface. If it is -1, Slang will
     * automatically assign a unique Id for the type.
     * \return The updated list of type conformances.
     */
    TypeConformanceList& add(std::string type_name, std::string interface_name, uint32_t id = -1);

    /**
     * Removes a type conformance. If the type conformance doesn't exist, the call will be silently ignored.
     * \param type_name The name of the implementation type.
     * \param interface_name The name of the interface type.
     * \return The updated list of type conformances.
     */
    TypeConformanceList& remove(std::string type_name, std::string interface_name);

    /**
     * Add a type conformance list to the current list.
     */
    TypeConformanceList& add(const TypeConformanceList& other);

    /**
     * Remove a type conformance list from the current list.
     */
    TypeConformanceList& remove(const TypeConformanceList& other);
};

/// Exception thrown on compilation errors.
class SlangCompileError : public std::runtime_error {
public:
    SlangCompileError(const std::string& what)
        : std::runtime_error(what)
    {
    }
};

/// Slang matrix layout modes.
enum class SlangMatrixLayout {
    row_major = SLANG_MATRIX_LAYOUT_ROW_MAJOR,
    column_major = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
};
KALI_ENUM_INFO(
    SlangMatrixLayout,
    {
        {SlangMatrixLayout::row_major, "row_major"},
        {SlangMatrixLayout::column_major, "column_major"},
    }
);
KALI_ENUM_REGISTER(SlangMatrixLayout);

/// Slang floating point modes.
enum class SlangFloatingPointMode {
    default_ = SLANG_FLOATING_POINT_MODE_DEFAULT,
    fast = SLANG_FLOATING_POINT_MODE_FAST,
    precise = SLANG_FLOATING_POINT_MODE_PRECISE,
};
KALI_ENUM_INFO(
    SlangFloatingPointMode,
    {
        {SlangFloatingPointMode::default_, "default"},
        {SlangFloatingPointMode::fast, "fast"},
        {SlangFloatingPointMode::precise, "precise"},
    }
);
KALI_ENUM_REGISTER(SlangFloatingPointMode);

/// Slang debug info levels.
enum class SlangDebugInfoLevel {
    /// No debug information.
    none = SLANG_DEBUG_INFO_LEVEL_NONE,
    /// Emit as little debug information as possible, while still supporting stack trackes.
    minimal = SLANG_DEBUG_INFO_LEVEL_MINIMAL,
    /// Emit whatever is the standard level of debug information for each target.
    standard = SLANG_DEBUG_INFO_LEVEL_STANDARD,
    /// Emit as much debug infromation as possible for each target.
    maximal = SLANG_DEBUG_INFO_LEVEL_MAXIMAL,
};
KALI_ENUM_INFO(
    SlangDebugInfoLevel,
    {
        {SlangDebugInfoLevel::none, "none"},
        {SlangDebugInfoLevel::minimal, "minimal"},
        {SlangDebugInfoLevel::standard, "standard"},
        {SlangDebugInfoLevel::maximal, "maximal"},
    }
);
KALI_ENUM_REGISTER(SlangDebugInfoLevel);

/// Slang optimization levels.
enum class SlangOptimizationLevel {
    /// No optimizations.
    none = SLANG_OPTIMIZATION_LEVEL_NONE,
    /// Default optimization level: balance code quality and compilation time.
    default_ = SLANG_OPTIMIZATION_LEVEL_DEFAULT,
    /// Optimize aggressively.
    high = SLANG_OPTIMIZATION_LEVEL_HIGH,
    /// Include optimizations that may take a very long time, or may involve severe space-vs-speed tradeoffs.
    maximal = SLANG_OPTIMIZATION_LEVEL_MAXIMAL,
};
KALI_ENUM_INFO(
    SlangOptimizationLevel,
    {
        {SlangOptimizationLevel::none, "none"},
        {SlangOptimizationLevel::default_, "default"},
        {SlangOptimizationLevel::high, "high"},
        {SlangOptimizationLevel::maximal, "maximal"},
    }
);
KALI_ENUM_REGISTER(SlangOptimizationLevel);

/// Slang compiler options.
/// Can be set when creating a Slang session.
struct SlangCompilerOptions {
    /// Specifies a list of include paths to be used when resolving module/include paths.
    std::vector<std::filesystem::path> include_paths;

    /// Specifies a list of preprocessor defines.
    std::map<std::string, std::string> defines;

    /// Specifies the shader model to use.
    /// Defaults to latest available on the device.
    ShaderModel shader_model{ShaderModel::unknown};

    /// Specifies the matrix layout.
    /// Defaults to row-major.
    SlangMatrixLayout matrix_layout{SlangMatrixLayout::row_major};

    /// Specifies a list of warnings to disable (warning codes or names).
    std::vector<std::string> disable_warnings;

    /// Specifies a list of warnings to enable (warning codes or names).
    std::vector<std::string> enable_warnings;

    /// Specifies a list of warnings to be treated as errors
    /// (warning codes or names, or "all" to indicate all warnings).
    std::vector<std::string> warnings_as_errors;

    /// Turn on/off downstream compilation time report.
    bool report_downstream_time{false};

    /// Turn on/off reporting of time spend in different parts of the compiler.
    bool report_perf_benchmark{false};

    /// Specifies whether or not to skip the validation step after emitting SPIRV.
    bool skip_spirv_validation{false};

    //
    // Target options.
    // These can be extended/overriden at link time.
    //

    /// Specifies the floating point mode.
    SlangFloatingPointMode floating_point_mode{SlangFloatingPointMode::default_};

    /// Specifies the level of debug information to include in the generated code.
    SlangDebugInfoLevel debug_info{SlangDebugInfoLevel::standard};

    /// Specifies the optimization level.
    SlangOptimizationLevel optimization{SlangOptimizationLevel::default_};

    /// Specifies a list of additional arguments to be passed to the downstream compiler.
    std::vector<std::string> downstream_args;

    /// When set will dump the intermediate source output.
    bool dump_intermediates{false};

    /// The file name prefix for the intermediate source output.
    std::string dump_intermediates_prefix;
};

/// Slang link options.
/// These can optionally be set when linking a shader program.
struct SlangLinkOptions {
    /// Specifies the floating point mode.
    std::optional<SlangFloatingPointMode> floating_point_mode;

    /// Specifies the level of debug information to include in the generated code.
    std::optional<SlangDebugInfoLevel> debug_info;

    /// Specifies the optimization level.
    std::optional<SlangOptimizationLevel> optimization;

    /// Specifies a list of additional arguments to be passed to the downstream compiler.
    std::optional<std::vector<std::string>> downstream_args;

    /// When set will dump the intermediate source output.
    std::optional<bool> dump_intermediates;

    /// The file name prefix for the intermediate source output.
    std::optional<std::string> dump_intermediates_prefix;
};

struct SlangSessionDesc {
    SlangCompilerOptions compiler_options;
    bool add_default_include_paths{true};
    std::optional<std::filesystem::path> cache_path;
};

class KALI_API SlangSession : public Object {
    KALI_OBJECT(SlangSession)
public:
    SlangSession(ref<Device> device, SlangSessionDesc desc);
    ~SlangSession();

    Device* device() const { return m_device; }
    const SlangSessionDesc& desc() const { return m_desc; }

    ref<SlangModule> load_module(std::string_view module_name);

    ref<SlangModule> load_module_from_source(
        std::string_view module_name,
        std::string_view source,
        std::optional<std::filesystem::path> path = {}
    );

    ref<ShaderProgram> link_program(
        std::vector<ref<SlangModule>> modules,
        std::vector<ref<SlangEntryPoint>> entry_points,
        std::optional<SlangLinkOptions> link_options = {}
    );

    ref<ShaderProgram> load_program(
        std::string_view module_name,
        std::vector<std::string_view> entry_point_names,
        std::optional<std::string_view> additional_source = {},
        std::optional<SlangLinkOptions> link_options = {}
    );

    slang::ISession* get_slang_session() const { return m_slang_session; }

    std::string to_string() const override;

    void break_strong_reference_to_device() { m_device.break_strong_reference(); }

private:
    bool write_module_to_cache(SlangModule* module);

    std::string resolve_module_name(std::string_view module_name);

    /// Register a module with the debug printer.
    /// This extracts all the hashed strings of the module.
    void register_with_debug_printer(SlangModule* module) const;

    breakable_ref<Device> m_device;
    SlangSessionDesc m_desc;
    std::string m_uid;
    Slang::ComPtr<slang::ISession> m_slang_session;

    /// List of include paths used for resolving module/include paths.
    std::vector<std::filesystem::path> m_include_paths;
    /// True if session cache is enabled.
    bool m_cache_enabled{false};
    /// Cache root path.
    std::filesystem::path m_cache_path;
    /// One cache path for each include path under the root cache path.
    std::vector<std::filesystem::path> m_cache_include_paths;
};

class KALI_API SlangModule : public Object {
    KALI_OBJECT(SlangModule)
public:
    SlangModule(ref<SlangSession> session, slang::IModule* slang_module);
    ~SlangModule();

    SlangSession* session() const { return m_session; }

    const std::string& name() const { return m_name; }
    const std::filesystem::path& path() const { return m_path; }
    const ProgramLayout* layout() const { return ProgramLayout::from_slang(m_slang_module->getLayout()); }

    std::vector<ref<SlangEntryPoint>> entry_points() const;
    ref<SlangEntryPoint> entry_point(std::string_view name) const;
    bool has_entry_point(std::string_view name) const;

    slang::IModule* slang_module() const { return m_slang_module; }

    std::string to_string() const override;

private:
    ref<SlangSession> m_session;
    /// Slang module (owned by the session).
    slang::IModule* m_slang_module;
    std::string m_name;
    std::filesystem::path m_path;
};

class KALI_API SlangEntryPoint : public Object {
    KALI_OBJECT(SlangEntryPoint)
public:
    SlangEntryPoint(ref<SlangModule> module, Slang::ComPtr<slang::IComponentType> slang_entry_point);
    ~SlangEntryPoint() = default;

    SlangModule* module() const { return m_module; }

    const std::string& name() const { return m_name; }
    ShaderStage stage() const { return m_stage; }
    const EntryPointLayout* layout() const;

    ref<SlangEntryPoint> rename(const std::string& new_name);

    /// Returns a copy of the entry point with a new name.
    ref<SlangEntryPoint> with_name(const std::string& name) const;

    /// Returns a copy of the entry point with a set of type conformances set.
    ref<SlangEntryPoint> with_type_conformances(const TypeConformanceList& type_conformances) const;

    slang::IComponentType* slang_entry_point() const { return m_slang_entry_point.get(); }

    virtual std::string to_string() const override;

private:
    ref<SlangModule> m_module;
    Slang::ComPtr<slang::IComponentType> m_slang_entry_point;
    std::string m_name;
    ShaderStage m_stage;
};

class KALI_API ShaderProgram : public Object {
    KALI_OBJECT(ShaderProgram)
public:
    ShaderProgram(
        ref<Device> device,
        ref<SlangSession> session,
        std::vector<ref<SlangModule>> modules,
        std::vector<ref<SlangEntryPoint>> entry_points,
        Slang::ComPtr<slang::IComponentType> linked_program
    );
    ~ShaderProgram() = default;

    const ref<Device>& device() const { return m_device; }

    const ProgramLayout* program_layout() const { return ProgramLayout::from_slang(m_linked_program->getLayout()); }

    ReflectionCursor reflection() const { return ReflectionCursor(this); }

    gfx::IShaderProgram* gfx_shader_program() const { return m_gfx_shader_program; }

    virtual std::string to_string() const override;

private:
    ref<Device> m_device;
    ref<SlangSession> m_session;
    std::vector<ref<SlangModule>> m_modules;
    std::vector<ref<SlangEntryPoint>> m_entry_points;
    Slang::ComPtr<slang::IComponentType> m_linked_program;
    Slang::ComPtr<gfx::IShaderProgram> m_gfx_shader_program;
};

} // namespace kali
