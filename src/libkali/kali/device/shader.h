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

namespace kali {

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

KALI_ENUM_INFO(
    ShaderCompilerFlags,
    {
        {ShaderCompilerFlags::none, "none"},
        {ShaderCompilerFlags::treat_warnings_as_errors, "treat_warnings_as_errors"},
        {ShaderCompilerFlags::dump_intermediates, "dump_intermediates"},
        {ShaderCompilerFlags::floating_point_mode_fast, "floating_point_mode_fast"},
        {ShaderCompilerFlags::floating_point_mode_precise, "floating_point_mode_precise"},
        {ShaderCompilerFlags::generate_debug_info, "generate_debug_info"},
        {ShaderCompilerFlags::matrix_layout_column_major, "matrix_layout_column_major"},
    }
);
KALI_ENUM_REGISTER(ShaderCompilerFlags);

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
class SlangCompileError : public std::runtime_error {
public:
    SlangCompileError(const std::string& what)
        : std::runtime_error(what)
    {
    }
};

struct SlangSessionDesc {
    ShaderModel shader_model{ShaderModel::unknown};
    ShaderCompilerFlags compiler_flags{ShaderCompilerFlags::none};
    std::vector<std::string> compiler_args;
    std::vector<std::filesystem::path> search_paths;
    DefineList defines;
};

class KALI_API SlangSession : public Object {
    KALI_OBJECT(SlangSession)
public:
    SlangSession(ref<Device> device, SlangSessionDesc desc);
    virtual ~SlangSession();

    const ref<Device> device() const { return m_device; }
    const SlangSessionDesc& desc() const { return m_desc; }

    ref<SlangModule> load_module(const std::filesystem::path& path);
    ref<SlangModule> load_module_from_source(
        const std::string& source,
        const std::filesystem::path& path = {},
        const std::string& name = {}
    );

    std::filesystem::path resolve_path(const std::filesystem::path& path);

    slang::ISession* get_slang_session() const { return m_slang_session; }

private:
    ref<Device> m_device;
    SlangSessionDesc m_desc;
    Slang::ComPtr<slang::ISession> m_slang_session;
};

struct SlangModuleDesc {
    enum class Type {
        file,
        source,
    };
    Type type;
    std::filesystem::path path;
    std::string source;
};

class KALI_API SlangModule : public Object {
    KALI_OBJECT(SlangModule)
public:
    SlangModule(ref<SlangSession> session, SlangModuleDesc desc);
    virtual ~SlangModule();

    const SlangModuleDesc& desc() const { return m_desc; }

    ref<SlangGlobalScope> global_scope();
    ref<SlangEntryPoint> entry_point(std::string_view name);
    std::vector<ref<SlangEntryPoint>> entry_points();

    ref<ShaderProgram> create_program(std::string_view entry_point_name);
    ref<ShaderProgram> create_program(ref<SlangGlobalScope> global_scope, ref<SlangEntryPoint> entry_point);
    ref<ShaderProgram>
    create_program(ref<SlangGlobalScope> global_scope, std::vector<ref<SlangEntryPoint>> entry_points);

private:
    ref<SlangSession> m_session;
    SlangModuleDesc m_desc;
    Slang::ComPtr<slang::ICompileRequest> m_compile_request;
    size_t m_entry_point_count;
};

class KALI_API SlangComponentType : public Object {
    KALI_OBJECT(SlangComponentType)
public:
    SlangComponentType(ref<SlangModule> module, Slang::ComPtr<slang::IComponentType> component_type);
    virtual ~SlangComponentType() = default;

    ref<SlangModule> module() const { return m_module; }

protected:
    ref<SlangModule> m_module;
    Slang::ComPtr<slang::IComponentType> m_component_type;

    friend class ShaderProgram;
};

class KALI_API SlangGlobalScope : public SlangComponentType {
    KALI_OBJECT(SlangGlobalScope)
public:
    SlangGlobalScope(ref<SlangModule> module, Slang::ComPtr<slang::IComponentType> component_type);
    virtual ~SlangGlobalScope() = default;

    const ProgramReflection* layout() const;

    virtual std::string to_string() const override;
};

class KALI_API SlangEntryPoint : public SlangComponentType {
    KALI_OBJECT(SlangEntryPoint)
public:
    SlangEntryPoint(ref<SlangModule> module, Slang::ComPtr<slang::IComponentType> component_type);
    virtual ~SlangEntryPoint() = default;

    const std::string& name() const { return m_name; }
    ShaderStage stage() const { return m_stage; }
    const EntryPointReflection* layout() const;

    ref<SlangEntryPoint> rename(const std::string& new_name);

    virtual std::string to_string() const override;

private:
    std::string m_name;
    ShaderStage m_stage;
};

class KALI_API ShaderProgram : public Object {
    KALI_OBJECT(ShaderProgram)
public:
    ShaderProgram(
        ref<Device> device,
        ref<SlangGlobalScope> global_scope,
        std::vector<ref<SlangEntryPoint>> entry_points
    );
    virtual ~ShaderProgram() = default;

    const ref<Device>& device() const { return m_device; }

    const ProgramReflection* program_layout() const { return m_global_scope->layout(); }
    std::vector<const EntryPointReflection*> entry_point_layouts() const;
    const EntryPointReflection* entry_point_layout(uint32_t index) const { return m_entry_points[index]->layout(); }

    gfx::IShaderProgram* get_gfx_shader_program() const { return m_gfx_shader_program; }

    virtual std::string to_string() const override;

private:
    ref<Device> m_device;
    ref<SlangGlobalScope> m_global_scope;
    std::vector<ref<SlangEntryPoint>> m_entry_points;
    Slang::ComPtr<gfx::IShaderProgram> m_gfx_shader_program;
};

} // namespace kali
