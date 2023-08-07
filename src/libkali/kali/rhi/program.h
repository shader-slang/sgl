#pragma once

#include "kali/rhi/fwd.h"

#include "kali/core/error.h"
#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/enum.h"

#include <slang-gfx.h>

#include <string>
#include <vector>
#include <filesystem>
#include <functional>
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
    automatic = 0,
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
        {ShaderModel::automatic, "automatic"},
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

    bool operator<(TypeConformance const& other) const
    {
        return type_name < other.type_name || (type_name == other.type_name && interface_name < other.interface_name);
    }

    bool operator==(TypeConformance const& other) const
    {
        return type_name == other.type_name && interface_name == other.interface_name;
    }
    struct HashFunction {
        size_t operator()(const TypeConformance& conformance) const
        {
            size_t hash = std::hash<std::string>()(conformance.type_name);
            hash = hash ^ std::hash<std::string>()(conformance.interface_name);
            return hash;
        }
    };
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

class DefineList : public std::map<std::string, std::string> {
public:
    /**
     * Adds a macro definition. If the macro already exists, it will be replaced.
     * @param[in] name The name of macro.
     * @param[in] value Optional. The value of the macro.
     * @return The updated list of macro definitions.
     */
    DefineList& add(const std::string& name, std::string value = "")
    {
        (*this)[name] = std::move(value);
        return *this;
    }

    /**
     * Removes a macro definition. If the macro doesn't exist, the call will be silently ignored.
     * @param[in] name The name of macro.
     * @return The updated list of macro definitions.
     */
    DefineList& remove(const std::string& name)
    {
        (*this).erase(name);
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

    DefineList() = default;
    DefineList(std::initializer_list<std::pair<const std::string, std::string>> il)
        : std::map<std::string, std::string>(il)
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
};

struct EntryPointDesc {
    ShaderStage type;
    std::string name;
    std::string export_name;
};

struct ProgramKernelDesc {
    uint32_t module_index;
    std::vector<EntryPointDesc> entry_points;
};

struct ProgramDesc {
    std::vector<ShaderModuleDesc> modules;
    std::vector<ProgramKernelDesc> kernels;

    DefineList defines;
    ShaderModel shader_model{ShaderModel::automatic};
    ShaderCompilerFlags compiler_flags;
    std::string prelude;
    std::vector<std::string> downstream_compiler_args;

    static ProgramDesc create() { return {}; }

    static ProgramDesc create_compute(std::filesystem::path path, std::string entry_point)
    {
        ProgramDesc desc{
            .modules = {
                ShaderModuleDesc{
                    .sources = {
                        ShaderSourceDesc{
                            .type = ShaderSourceType::file,
                            .file = path,
                        },
                    },
                },
            },
            .kernels = {
                ProgramKernelDesc{
                    .module_index = 0,
                    .entry_points = {
                        EntryPointDesc{
                            .type = ShaderStage::compute,
                            .name = entry_point
                        },
                    },
                },
            },
        };
        return desc;
    }

    // clang-format off
    ProgramDesc& set_defines(DefineList defines_) { defines = std::move(defines_); return *this; }
    ProgramDesc& set_shader_model(ShaderModel shader_model_) { shader_model = shader_model_; return *this; }
    ProgramDesc& set_compiler_flags(ShaderCompilerFlags compiler_flags_) { compiler_flags = compiler_flags_; return *this; }
    ProgramDesc& set_prelude(std::string prelude_) { prelude = std::move(prelude_); return *this; }
    ProgramDesc& set_downstream_compiler_args(std::vector<std::string> downstream_compiler_args_) { downstream_compiler_args = std::move(downstream_compiler_args_); return *this; }
    // clang-format on

    // ProgramDesc& add_file(std::filesystem::path path)
    // {
    //     shader_modules.emplace_back(ShaderModuleDesc{
    //         .type = ShaderModuleType::file,
    //         .file = std::move(path),
    //     });
    //     return *this;
    // }

    // ProgramDesc& add_string(std::string string)
    // {
    //     shader_modules.emplace_back(ShaderModuleDesc{
    //         .type = ShaderModuleType::string,
    //         .string = std::move(string),
    //     });
    //     return *this;
    // }

    // ProgramDesc& add_entrypoint(std::string name)
    // {
    //     entrypoints.emplace_back(std::move(name));
    //     return *this;
    // }
};

struct ProgramVersion;

struct ProgramKernels;

class KALI_API Program : public Object {
    KALI_OBJECT(Program)
public:
    Program(const ProgramDesc& desc, ProgramManager* program_manager);

    const ProgramDesc& get_desc() const { return m_desc; }

    gfx::IShaderProgram* get_gfx_shader_program() const { return m_gfx_shader_program; }

private:
    ProgramDesc m_desc;
    ProgramManager* m_program_manager;

    ProgramVersion* m_active_version{nullptr};

    Slang::ComPtr<gfx::IShaderProgram> m_gfx_shader_program;
};

class KALI_API ProgramManager : public Object {
    KALI_OBJECT(ProgramManager)
public:
    ProgramManager(Device* device, slang::IGlobalSession* slang_session);

    ref<Program> create_program(const ProgramDesc& desc);

    ref<ProgramVersion> create_program_version(const Program& program, std::string& out_log) const;

    const std::vector<std::filesystem::path>& get_search_paths() const { return m_search_paths; }
    void add_search_path(std::filesystem::path path);
    void remove_search_path(std::filesystem::path path);

private:
    Slang::ComPtr<slang::ICompileRequest> create_slang_compile_request(const Program& program) const;

    std::filesystem::path resolve_path(const std::filesystem::path& path) const;

    Device* m_device;
    slang::IGlobalSession* m_slang_session;

    std::vector<std::filesystem::path> m_search_paths;
    DefineList m_global_defines;
    ShaderCompilerFlags m_forced_enabled_compiler_flags{ShaderCompilerFlags::none};
    ShaderCompilerFlags m_forced_disabled_compiler_flags{ShaderCompilerFlags::none};
    std::vector<int> m_disabled_warnings;
};


}; // namespace kali
