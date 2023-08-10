#pragma once

#include "kali/rhi/fwd.h"

#include "kali/core/error.h"
#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/enum.h"
#include "kali/core/type_utils.h"

#include <slang-gfx.h>

#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <map>

namespace kali {

enum class ShaderStage {
    none,
    vertex,
    hull,
    domain,
    geometry,
    fragment,
    compute,
    ray_generation,
    intersection,
    any_hit,
    closest_hit,
    miss,
    callable,
    mesh,
    amplification,
};

KALI_ENUM_INFO(
    ShaderStage,
    {
        {ShaderStage::none, "none"},
        {ShaderStage::vertex, "vertex"},
        {ShaderStage::hull, "hull"},
        {ShaderStage::domain, "domain"},
        {ShaderStage::geometry, "geometry"},
        {ShaderStage::fragment, "fragment"},
        {ShaderStage::compute, "compute"},
        {ShaderStage::ray_generation, "ray_generation"},
        {ShaderStage::intersection, "intersection"},
        {ShaderStage::any_hit, "any_hit"},
        {ShaderStage::closest_hit, "closest_hit"},
        {ShaderStage::miss, "miss"},
        {ShaderStage::callable, "callable"},
        {ShaderStage::mesh, "mesh"},
        {ShaderStage::amplification, "amplification"},
    }
);

KALI_ENUM_REGISTER(ShaderStage);

enum class ShaderModel {
    unknown = 0,
    sm_6_0 = 60,
    sm_6_1 = 61,
    sm_6_2 = 62,
    sm_6_3 = 63,
    sm_6_4 = 64,
    sm_6_5 = 65,
    sm_6_6 = 66,
    sm_6_7 = 67,
};
KALI_ENUM_INFO(
    ShaderModel,
    {
        {ShaderModel::unknown, "unknown"},
        {ShaderModel::sm_6_0, "sm_6_0"},
        {ShaderModel::sm_6_1, "sm_6_1"},
        {ShaderModel::sm_6_2, "sm_6_2"},
        {ShaderModel::sm_6_3, "sm_6_3"},
        {ShaderModel::sm_6_4, "sm_6_4"},
        {ShaderModel::sm_6_5, "sm_6_5"},
        {ShaderModel::sm_6_6, "sm_6_6"},
        {ShaderModel::sm_6_7, "sm_6_7"},
    }
);
KALI_ENUM_REGISTER(ShaderModel);

inline uint32_t get_shader_model_major_version(ShaderModel sm)
{
    return static_cast<uint32_t>(sm) / 10;
}

inline uint32_t get_shader_model_minor_version(ShaderModel sm)
{
    return static_cast<uint32_t>(sm) % 10;
}

enum class ShaderCompilerFlags {
    none = 0x0,
    treat_warnings_as_errors = 0x1,
    dump_intermediates = 0x2,
    floating_point_mode_fast = 0x4,
    floating_point_mode_precise = 0x8,
    generate_debug_info = 0x10,
    matrix_layout_column_major = 0x20,
};
KALI_ENUM_CLASS_OPERATORS(ShaderCompilerFlags)

struct TypeConformance {
    std::string type_name;
    std::string interface_name;
    auto operator<=>(const TypeConformance&) const = default;
};

class TypeConformanceList : public std::map<TypeConformance, uint32_t> {
public:
    /**
     * Adds a type conformance. If the type conformance exists, it will be replaced.
     * @param[in] type_name The name of the implementation type.
     * @param[in] interface_name The name of the interface type.
     * @param[in] id Optional. The id representing the implementation type for this interface. If it is -1, Slang will
     * automatically assign a unique Id for the type.
     * @return The updated list of type conformances.
     */
    TypeConformanceList& add(std::string type_name, std::string interface_name, uint32_t id = -1)
    {
        (*this)[TypeConformance{std::move(type_name), std::move(interface_name)}] = id;
        return *this;
    }

    /**
     * Removes a type conformance. If the type conformance doesn't exist, the call will be silently ignored.
     * @param[in] type_name The name of the implementation type.
     * @param[in] interface_name The name of the interface type.
     * @return The updated list of type conformances.
     */
    TypeConformanceList& remove(std::string type_name, std::string interface_name)
    {
        (*this).erase(TypeConformance{std::move(type_name), std::move(interface_name)});
        return *this;
    }

    /**
     * Add a type conformance list to the current list
     */
    TypeConformanceList& add(const TypeConformanceList& other)
    {
        for (const auto& p : other)
            add(p.first.type_name, p.first.interface_name, p.second);
        return *this;
    }

    /**
     * Remove a type conformance list from the current list
     */
    TypeConformanceList& remove(const TypeConformanceList& other)
    {
        for (const auto& p : other)
            remove(p.first.type_name, p.first.interface_name);
        return *this;
    }

    TypeConformanceList() = default;
    TypeConformanceList(std::initializer_list<std::pair<const TypeConformance, uint32_t>> init)
        : std::map<TypeConformance, uint32_t>(init)
    {
    }
};

class DefineList : public std::map<std::string, std::string, std::less<>> {
public:
    /**
     * Adds a macro definition. If the macro already exists, it will be replaced.
     * @param[in] name The name of macro.
     * @param[in] value Optional. The value of the macro.
     * @return The updated list of macro definitions.
     */
    DefineList& add(std::string_view name, std::string_view value = "")
    {
        emplace(std::string{name}, value);
        return *this;
    }

    /**
     * Removes a macro definition. If the macro doesn't exist, the call will be silently ignored.
     * @param[in] name The name of macro.
     * @return The updated list of macro definitions.
     */
    DefineList& remove(std::string_view name)
    {
        auto it = find(name);
        if (it != end())
            erase(it);
        return *this;
    }

    /**
     * Add a define list to the current list
     */
    DefineList& add(const DefineList& other)
    {
        for (const auto& p : other)
            add(p.first, p.second);
        return *this;
    }

    /**
     * Remove a define list from the current list
     */
    DefineList& remove(const DefineList& other)
    {
        for (const auto& p : other)
            remove(p.first);
        return *this;
    }

    bool has(std::string_view name) const { return find(name) != end(); }

    DefineList() = default;
    DefineList(std::initializer_list<std::pair<const std::string, std::string>> il)
        : std::map<std::string, std::string, std::less<>>(il)
    {
    }
};

enum class ShaderSourceType { file, string };

struct ShaderSourceDesc {
    ShaderSourceType type{ShaderSourceType::file};
    std::filesystem::path file;
    std::string string;
};

struct ShaderModuleDesc {
    std::vector<ShaderSourceDesc> sources;

    void add_file(std::filesystem::path path)
    {
        sources.push_back({
            .type = ShaderSourceType::file,
            .file = std::move(path),
        });
    }

    void add_string(std::string string)
    {
        sources.push_back({
            .type = ShaderSourceType::string,
            .string = std::move(string),
        });
    }
};

struct EntryPointDesc {
    ShaderStage type;
    std::string name;
    std::string export_name;
};

struct EntryPointGroupDesc {
    std::string name;
    uint32_t shader_module_index;
    std::vector<EntryPointDesc> entry_points;

    EntryPointDesc& add_entry_point(ShaderStage type, std::string name_, std::string export_name = "")
    {
        entry_points.push_back({type, std::move(name_), std::move(export_name)});
        return entry_points.back();
    }
};

struct ProgramKernelDesc {
    uint32_t module_index;
    std::vector<EntryPointDesc> entry_points;
};

struct ProgramDesc {
    std::vector<ShaderModuleDesc> shader_modules;
    std::vector<EntryPointGroupDesc> entry_point_groups;

    ShaderModel shader_model{ShaderModel::unknown};
    ShaderCompilerFlags compiler_flags{ShaderCompilerFlags::none};
    std::string prelude;
    std::vector<std::string> downstream_compiler_args;

    static ProgramDesc create() { return {}; }

    static ProgramDesc create_compute(std::filesystem::path path, std::string entry_point)
    {
        ProgramDesc desc;
        desc.add_shader_module().add_file(std::move(path));
        desc.add_entry_point_group().add_entry_point(ShaderStage::compute, std::move(entry_point));
        return desc;
    }

    ShaderModuleDesc& add_shader_module()
    {
        shader_modules.push_back({});
        return shader_modules.back();
    }

    EntryPointGroupDesc& add_entry_point_group(std::string name = "", uint32_t shader_module_index = uint32_t(-1))
    {
        if (name.empty())
            name = "entry_point_group_" + std::to_string(entry_point_groups.size());
        if (shader_module_index == uint32_t(-1))
            shader_module_index = narrow_cast<uint32_t>(shader_modules.size()) - 1;
        entry_point_groups.push_back({
            .name = std::move(name),
            .shader_module_index = shader_module_index,
        });
        return entry_point_groups.back();
    }

    ProgramDesc& set_shader_model(ShaderModel shader_model_)
    {
        shader_model = shader_model_;
        return *this;
    }

    ProgramDesc& set_compiler_flags(ShaderCompilerFlags compiler_flags_)
    {
        compiler_flags = compiler_flags_;
        return *this;
    }

    ProgramDesc& set_prelude(std::string prelude_)
    {
        prelude = std::move(prelude_);
        return *this;
    }

    ProgramDesc& set_downstream_compiler_args(std::vector<std::string> downstream_compiler_args_)
    {
        downstream_compiler_args = std::move(downstream_compiler_args_);
        return *this;
    }
};

class ProgramVersion;

class KALI_API Program : public Object {
    KALI_OBJECT(Program)
public:
    Program(ProgramDesc desc, ProgramManager* program_manager);
    virtual ~Program();

    const ProgramDesc& get_desc() const { return m_desc; }

    ProgramManager* get_program_manager() const { return m_program_manager; }

    const DefineList& get_defines() const { return m_defines; }

    void set_defines(const DefineList& defines);

    void add_define(std::string_view name, std::string_view value = "");
    void add_defines(const DefineList& defines);
    void remove_define(std::string_view name);

    const ProgramVersion* get_active_version();

    bool reload(bool force);

private:
    void mark_dirty() { m_active_version = nullptr; }

    struct ProgramVersionKey {
        DefineList defines;
        TypeConformanceList type_conformances;
        auto operator<=>(const ProgramVersionKey&) const = default;
    };

    ProgramDesc m_desc;
    ProgramManager* m_program_manager;

    DefineList m_defines;
    TypeConformanceList m_type_conformances;

    std::map<ProgramVersionKey, ref<const ProgramVersion>> m_versions;
    const ProgramVersion* m_active_version{nullptr};

    /// Map to keep track of all source files contributing to this program and their timestamps.
    /// This is used to detect if a source file has changed and the program needs to be recompiled.
    mutable std::map<std::filesystem::path, std::filesystem::file_time_type> m_source_file_timestamps;

    friend class ProgramManager;
};

class ProgramLayout;
class EntryPointLayout;

class KALI_API ProgramVersion : public Object {
    KALI_OBJECT(ProgramVersion)
public:
    ProgramVersion(
        const Program* program,
        Slang::ComPtr<slang::ICompileRequest> compile_request,
        Slang::ComPtr<slang::IComponentType> program_component,
        std::vector<Slang::ComPtr<slang::IComponentType>> entry_point_components,
        std::vector<Slang::ComPtr<slang::IComponentType>> linked_entry_point_components
    );

    ProgramLayout get_program_layout() const;
    EntryPointLayout get_entry_point_layout(uint32_t entry_point) const;

    gfx::IShaderProgram* get_gfx_shader_program() const;

private:
    const Program* m_program;
    Slang::ComPtr<slang::ICompileRequest> m_compile_request;
    Slang::ComPtr<slang::IComponentType> m_program_component;
    std::vector<Slang::ComPtr<slang::IComponentType>> m_entry_point_components;
    std::vector<Slang::ComPtr<slang::IComponentType>> m_linked_entry_point_components;
    mutable Slang::ComPtr<gfx::IShaderProgram> m_gfx_shader_program;
};

class KALI_API ProgramManager : public Object {
    KALI_OBJECT(ProgramManager)
public:
    ProgramManager(Device* device, slang::IGlobalSession* slang_session);

    Device* get_device() const { return m_device; }

    ref<Program> create_program(ProgramDesc desc);

    /// Create a new program version (i.e. a compiled program).
    /// @param program Program to compile.
    /// @param out_log String to write compiler output to.
    /// @return A new program version, or null if compilation failed.
    ref<ProgramVersion> create_program_version(const Program& program, std::string& out_log) const;

    /// Reload all programs.
    /// @param force If true, all programs will be reloaded, even if they haven't changed.
    /// @return The number of programs that were reloaded.
    size_t reload_programs(bool force = false);

    const std::vector<Program*>& get_loaded_programs() const { return m_loaded_programs; }

    void add_loaded_program(Program* program);
    void remove_loaded_program(Program* program);

    const std::vector<std::filesystem::path>& get_search_paths() const { return m_search_paths; }
    void add_search_path(std::filesystem::path path);
    void remove_search_path(std::filesystem::path path);

private:
    std::filesystem::path resolve_path(const std::filesystem::path& path) const;

    Device* m_device;
    slang::IGlobalSession* m_slang_session;

    std::vector<std::filesystem::path> m_search_paths;
    DefineList m_global_defines;
    ShaderCompilerFlags m_forced_enabled_compiler_flags{ShaderCompilerFlags::none};
    ShaderCompilerFlags m_forced_disabled_compiler_flags{ShaderCompilerFlags::none};
    std::vector<int> m_disabled_warnings;

    std::vector<Program*> m_loaded_programs;
};


}; // namespace kali
