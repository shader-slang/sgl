#include "reflection.h"

#include "kali/core/string.h"

#include "kali/math/vector.h"

#include <span>

namespace kali {

template<typename T>
inline std::string vector_to_string(const std::vector<T>& vector)
{
    if (vector.empty())
        return "[]";
    std::string result = "[\n";
    for (const auto& item : vector) {
        result += "    " + string::indent(item->to_string());
        result += ",\n";
    }
    result += "]";
    return result;
}


std::string c_str_to_string(const char* str)
{
    if (!str)
        return "null";
    return fmt::format("\"{}\"", str);
}

std::string TypeReflection::to_string() const
{
    std::string str;
    str += "TypeReflection(\n";
    // str += fmt::format("    kind={},\n", kind());
    // str += fmt::format("    name={},\n", c_str_to_string(name()));
    // str += fmt::format("    fields={},", vector_to_string(fields()));
    str += ")";
    return str;
}

std::string TypeLayoutReflection::to_string() const
{
    switch (kind()) {
    case TypeReflection::Kind::struct_:
        return fmt::format(
            "TypeLayoutReflection(\n"
            "    name={},\n"
            "    kind={},\n"
            "    fields={}\n"
            ")",
            c_str_to_string(name()),
            kind(),
            string::indent(vector_to_string(fields()))
        );
        break;
    case TypeReflection::Kind::resource:
        return fmt::format(
            "TypeLayoutReflection(\n"
            "    name={},\n"
            "    kind={},\n"
            "    element_type_layout={}\n"
            ")",
            c_str_to_string(name()),
            kind(),
            string::indent(element_type_layout()->to_string())
        );
        break;
    default:
        return fmt::format(
            "TypeLayoutReflection(\n"
            "    kind={},\n"
            ")",
            kind()
        );
    }
}

std::string VariableLayoutReflection::to_string() const
{
    return fmt::format(
        "VariableLayoutReflection(\n"
        "    name={},\n"
        "    type_layout={}\n"
        ")",
        c_str_to_string(name()),
        string::indent(type_layout()->to_string())
    );
}

std::string EntryPointReflection::to_string() const
{
    return fmt::format(
        "EntryPointReflection(\n"
        "    name={},\n"
        "    name_override={},\n"
        "    stage={},\n"
        "    compute_thread_group_size={},\n"
        "    parameters={}\n"
        ")",
        c_str_to_string(name()),
        c_str_to_string(name_override()),
        stage(),
        compute_thread_group_size(),
        string::indent(vector_to_string(parameters()))
    );
}

std::string ProgramReflection::to_string() const
{
    return fmt::format(
        "ProgramReflection(\n"
        "    globals_type={},\n"
        "    parameters={}\n"
        "    entry_points={}\n"
        ")",
        string::indent(globals_type_layout()->to_string()),
        string::indent(vector_to_string(parameters())),
        string::indent(vector_to_string(entry_points()))
    );
}

} // namespace kali
