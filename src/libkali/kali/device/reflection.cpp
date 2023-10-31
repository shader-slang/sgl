#include "reflection.h"

#include "kali/core/type_utils.h"
#include "kali/core/format.h"

#include "kali/math/vector_math.h"


template<>
struct fmt::formatter<kali::ShaderOffset> : formatter<std::string> {
    template<typename FormatContext>
    auto format(const kali::ShaderOffset& v, FormatContext& ctx) const
    {
        if (v.is_valid())
            return ::fmt::format_to(
                ctx.out(),
                "{{uniform_offset={}, binding_range_index={}, binding_array_index={}}}",
                v.uniform_offset,
                v.binding_range_index,
                v.binding_array_index
            );
        else
            return ::fmt::format_to(ctx.out(), "{{invalid}}");
    }
};

namespace kali {

namespace {
    std::string indent(std::string_view str)
    {
        std::string result;
        for (auto c : str) {
            result += c;
            if (c == '\n')
                result += "    ";
        }
        return result;
    }

    template<typename T>
    std::string objects_to_string(const std::vector<T>& objects)
    {
        if (objects.empty())
            return "[]";
        std::string result = "[\n";
        for (const auto& object : objects) {
            result += "    " + indent(object->to_string());
            result += ",\n";
        }
        result += "]";
        return result;
    }

} // namespace

// ----------------------------------------------------------------------------
// TypeReflection
// ----------------------------------------------------------------------------

const StructTypeReflection* TypeReflection::as_struct_type() const
{
    return kind() == Kind::struct_ ? static_cast<const StructTypeReflection*>(this) : nullptr;
}

const ArrayTypeReflection* TypeReflection::as_array_type() const
{
    return kind() == Kind::array ? static_cast<const ArrayTypeReflection*>(this) : nullptr;
}

const BasicTypeReflection* TypeReflection::as_basic_type() const
{
    return kind() == Kind::basic ? static_cast<const BasicTypeReflection*>(this) : nullptr;
}

const ResourceTypeReflection* TypeReflection::as_resource_type() const
{
    return kind() == Kind::resource ? static_cast<const ResourceTypeReflection*>(this) : nullptr;
}

const InterfaceTypeReflection* TypeReflection::as_interface_type() const
{
    return kind() == Kind::interface ? static_cast<const InterfaceTypeReflection*>(this) : nullptr;
}

std::string TypeReflection::to_string() const
{
    return fmt::format("TypeReflection(kind={})", kind());
}

// ----------------------------------------------------------------------------
// StructTypeReflection
// ----------------------------------------------------------------------------

VariableReflection* StructTypeReflection::find_member(std::string_view name) const
{
    for (const auto& member : m_members) {
        if (member->name() == name)
            return member;
    }
    return nullptr;
}

std::string StructTypeReflection::to_string() const
{
    return fmt::format(
        "StructTypeReflection(\n"
        "    name=\"{}\",\n"
        "    members={}\n"
        ")",
        name(),
        indent(objects_to_string(members()))
    );
}

// ----------------------------------------------------------------------------
// ArrayTypeReflection
// ----------------------------------------------------------------------------

std::string ArrayTypeReflection::to_string() const
{
    return fmt::format(
        "ArrayTypeReflection(\n"
        "    element_count={},\n",
        "    element_stride={},\n",
        "    element_type={}\n"
        ")",
        element_count(),
        element_stride(),
        indent(element_type()->to_string())
    );
}

// ----------------------------------------------------------------------------
// BasicTypeReflection
// ----------------------------------------------------------------------------

std::string BasicTypeReflection::to_string() const
{
    return fmt::format(
        "BasicTypeReflection(scalar_type={}, row_count={}, col_count={}, is_row_major={})",
        scalar_type(),
        row_count(),
        col_count(),
        is_row_major()
    );
}

// ----------------------------------------------------------------------------
// ResourceTypeReflection
// ----------------------------------------------------------------------------

std::string ResourceTypeReflection::to_string() const
{
    return fmt::format(
        "ResourceTypeReflection(type={}, dimensions={}, structured_type={}, return_type={}, shader_access={})",
        type(),
        dimensions(),
        structured_type(),
        return_type(),
        shader_access()
    );
}

// ----------------------------------------------------------------------------
// InterfaceTypeReflection
// ----------------------------------------------------------------------------

std::string InterfaceTypeReflection::to_string() const
{
    return fmt::format("InterfaceTypeReflection()");
}

// ----------------------------------------------------------------------------
// VariableReflection
// ----------------------------------------------------------------------------

std::string VariableReflection::to_string() const
{
    return fmt::format(
        "VariableReflection(\n"
        "    name=\"{}\",\n"
        "    offset={},\n"
        "    type={}\n"
        ")",
        name(),
        offset(),
        indent(type()->to_string())
    );
}


static ref<TypeReflection> reflect_type(slang::TypeLayoutReflection* type_layout);
static ref<VariableReflection> reflect_variable(slang::VariableLayoutReflection* variable_layout);

static ref<StructTypeReflection> reflect_struct_type(slang::TypeLayoutReflection* type_layout)
{
    // Note: not all types have names. In particular, the "element type" of
    // a `cbuffer` declaration is an anonymous `struct` type, and Slang
    // returns `nullptr` from `getName().
    std::string name = type_layout->getName() ? type_layout->getName() : "";

    std::vector<ref<VariableReflection>> members;

    for (uint32_t i = 0; i < type_layout->getFieldCount(); i++) {
        slang::VariableLayoutReflection* field = type_layout->getFieldByIndex(i);

        ref<VariableReflection> member = reflect_variable(field);
        if (member)
            members.push_back(std::move(member));
    }

    return make_ref<StructTypeReflection>(type_layout, name, members);
}

static ref<ArrayTypeReflection> reflect_array_type(slang::TypeLayoutReflection* type_layout)
{
    ref<TypeReflection> element_type = reflect_type(type_layout->getElementTypeLayout());
    uint32_t element_count = narrow_cast<uint32_t>(type_layout->getElementCount());
    uint32_t element_stride = narrow_cast<uint32_t>(type_layout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM));

    return make_ref<ArrayTypeReflection>(type_layout, element_type, element_count, element_stride);
}

static ref<BasicTypeReflection> reflect_basic_type(slang::TypeLayoutReflection* type_layout)
{
    BasicTypeReflection::ScalarType scalar_type = BasicTypeReflection::ScalarType(type_layout->getScalarType());
    uint8_t row_count = narrow_cast<uint8_t>(type_layout->getRowCount());
    uint8_t col_count = narrow_cast<uint8_t>(type_layout->getColumnCount());
    bool is_row_major = type_layout->getMatrixLayoutMode() == SLANG_MATRIX_LAYOUT_ROW_MAJOR;

    return make_ref<BasicTypeReflection>(type_layout, scalar_type, row_count, col_count, is_row_major);
}

inline ResourceTypeReflection::Type get_resource_type(slang::TypeReflection* type)
{
    switch (type->unwrapArray()->getKind()) {
    case slang::TypeReflection::Kind::ParameterBlock:
    case slang::TypeReflection::Kind::ConstantBuffer:
        return ResourceTypeReflection::Type::constant_buffer;
    case slang::TypeReflection::Kind::SamplerState:
        return ResourceTypeReflection::Type::sampler;
    case slang::TypeReflection::Kind::ShaderStorageBuffer:
        return ResourceTypeReflection::Type::structured_buffer;
    case slang::TypeReflection::Kind::TextureBuffer:
        return ResourceTypeReflection::Type::typed_buffer;
    case slang::TypeReflection::Kind::Resource:
        switch (type->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) {
        case SLANG_STRUCTURED_BUFFER:
            return ResourceTypeReflection::Type::structured_buffer;
        case SLANG_BYTE_ADDRESS_BUFFER:
            return ResourceTypeReflection::Type::raw_buffer;
        case SLANG_TEXTURE_BUFFER:
            return ResourceTypeReflection::Type::typed_buffer;
        case SLANG_ACCELERATION_STRUCTURE:
            return ResourceTypeReflection::Type::acceleration_structure;
        case SLANG_TEXTURE_1D:
        case SLANG_TEXTURE_2D:
        case SLANG_TEXTURE_3D:
        case SLANG_TEXTURE_CUBE:
            return ResourceTypeReflection::Type::texture;
        default:
            KALI_THROW("Unsupported resource type");
        }
    default:
        KALI_THROW("Unsupported resource type");
    }
}

inline ResourceTypeReflection::Dimensions get_resource_dimensions(SlangResourceShape shape)
{
    switch (shape) {
    case SLANG_TEXTURE_1D:
        return ResourceTypeReflection::Dimensions::texture1d;
    case SLANG_TEXTURE_1D_ARRAY:
        return ResourceTypeReflection::Dimensions::texture1d_array;
    case SLANG_TEXTURE_2D:
        return ResourceTypeReflection::Dimensions::texture2d;
    case SLANG_TEXTURE_2D_ARRAY:
        return ResourceTypeReflection::Dimensions::texture2d_array;
    case SLANG_TEXTURE_2D_MULTISAMPLE:
        return ResourceTypeReflection::Dimensions::texture2dms;
    case SLANG_TEXTURE_2D_MULTISAMPLE_ARRAY:
        return ResourceTypeReflection::Dimensions::texture2dms_array;
    case SLANG_TEXTURE_3D:
        return ResourceTypeReflection::Dimensions::texture3d;
    case SLANG_TEXTURE_CUBE:
        return ResourceTypeReflection::Dimensions::texture_cube;
    case SLANG_TEXTURE_CUBE_ARRAY:
        return ResourceTypeReflection::Dimensions::texture_cube_array;
    case SLANG_ACCELERATION_STRUCTURE:
        return ResourceTypeReflection::Dimensions::acceleration_structure;

    case SLANG_TEXTURE_BUFFER:
    case SLANG_STRUCTURED_BUFFER:
    case SLANG_BYTE_ADDRESS_BUFFER:
        return ResourceTypeReflection::Dimensions::buffer;

    default:
        return ResourceTypeReflection::Dimensions::unknown;
    }
}

inline ResourceTypeReflection::StructuredType get_structured_buffer_type(slang::TypeReflection* type)
{
    if (type->getKind() != slang::TypeReflection::Kind::Resource)
        return ResourceTypeReflection::StructuredType::none; // not a structured buffer

    if (type->getResourceShape() != SLANG_STRUCTURED_BUFFER)
        return ResourceTypeReflection::StructuredType::none; // not a structured buffer

    switch (type->getResourceAccess()) {
    case SLANG_RESOURCE_ACCESS_READ:
        return ResourceTypeReflection::StructuredType::regular;
    case SLANG_RESOURCE_ACCESS_READ_WRITE:
    case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
        return ResourceTypeReflection::StructuredType::counter;
    case SLANG_RESOURCE_ACCESS_APPEND:
        return ResourceTypeReflection::StructuredType::append;
    case SLANG_RESOURCE_ACCESS_CONSUME:
        return ResourceTypeReflection::StructuredType::consume;
    default:
        KALI_THROW("Unsupported structured buffer type");
    }
};

inline ResourceTypeReflection::ReturnType get_return_type(slang::TypeReflection* type)
{
    // Could be a resource that doesn't have a specific element type (e.g., a raw buffer)
    if (!type)
        return ResourceTypeReflection::ReturnType::unknown;

    switch (type->getScalarType()) {
    case slang::TypeReflection::ScalarType::Float32:
        return ResourceTypeReflection::ReturnType::float_;
    case slang::TypeReflection::ScalarType::Int32:
        return ResourceTypeReflection::ReturnType::int_;
    case slang::TypeReflection::ScalarType::UInt32:
        return ResourceTypeReflection::ReturnType::uint;
    case slang::TypeReflection::ScalarType::Float64:
        return ResourceTypeReflection::ReturnType::double_;

        // Could be a resource that uses an aggregate element type (e.g., a structured buffer)
    case slang::TypeReflection::ScalarType::None:
        return ResourceTypeReflection::ReturnType::unknown;

    default:
        return ResourceTypeReflection::ReturnType::unknown;
    }
}

inline ResourceTypeReflection::ShaderAccess get_shader_access(slang::TypeReflection* type)
{
    // Compute access for an array using the underlying type...
    type = type->unwrapArray();

    switch (type->getKind()) {
    case slang::TypeReflection::Kind::SamplerState:
    case slang::TypeReflection::Kind::ConstantBuffer:
        return ResourceTypeReflection::ShaderAccess::read;

    case slang::TypeReflection::Kind::Resource:
    case slang::TypeReflection::Kind::ShaderStorageBuffer:
        switch (type->getResourceAccess()) {
        case SLANG_RESOURCE_ACCESS_NONE:
            return ResourceTypeReflection::ShaderAccess::undefined;

        case SLANG_RESOURCE_ACCESS_READ:
            return ResourceTypeReflection::ShaderAccess::read;

        default:
            return ResourceTypeReflection::ShaderAccess::read_write;
        }
        break;

    default:
        return ResourceTypeReflection::ShaderAccess::undefined;
    }
}


static ref<ResourceTypeReflection> reflect_resource_type(slang::TypeLayoutReflection* type_layout)
{
    ResourceTypeReflection::Type type = get_resource_type(type_layout->getType());
    ResourceTypeReflection::Dimensions dimensions = get_resource_dimensions(type_layout->getResourceShape());
    ResourceTypeReflection::StructuredType structured_type = get_structured_buffer_type(type_layout->getType());
    ResourceTypeReflection::ReturnType return_type = get_return_type(type_layout->getType());
    ResourceTypeReflection::ShaderAccess shader_access = get_shader_access(type_layout->getType());
    ref<TypeReflection> element_type = reflect_type(type_layout->getElementTypeLayout());

    return make_ref<ResourceTypeReflection>(
        type_layout,
        type,
        dimensions,
        structured_type,
        return_type,
        shader_access,
        element_type
    );
}

static ref<InterfaceTypeReflection> reflect_interface_type(slang::TypeLayoutReflection* type_layout)
{
    return make_ref<InterfaceTypeReflection>(type_layout);
}

static ref<TypeReflection> reflect_type(slang::TypeLayoutReflection* type_layout)
{
    KALI_ASSERT(type_layout);
    switch (type_layout->getKind()) {
    case slang::TypeReflection::Kind::None:
        return nullptr;
    case slang::TypeReflection::Kind::Struct:
        return reflect_struct_type(type_layout);
    case slang::TypeReflection::Kind::Array:
        return reflect_array_type(type_layout);
    case slang::TypeReflection::Kind::Matrix:
    case slang::TypeReflection::Kind::Vector:
    case slang::TypeReflection::Kind::Scalar:
        return reflect_basic_type(type_layout);
    case slang::TypeReflection::Kind::ConstantBuffer:
    case slang::TypeReflection::Kind::Resource:
    case slang::TypeReflection::Kind::SamplerState:
    case slang::TypeReflection::Kind::TextureBuffer:
    case slang::TypeReflection::Kind::ShaderStorageBuffer:
    case slang::TypeReflection::Kind::ParameterBlock:
        return reflect_resource_type(type_layout);
    case slang::TypeReflection::Kind::GenericTypeParameter:
        KALI_THROW("Generic type parameters are not supported");
    case slang::TypeReflection::Kind::Interface:
        return reflect_interface_type(type_layout);
    case slang::TypeReflection::Kind::Specialized:
        // return reflect_specialized_type(type_layout);
        KALI_THROW("Specialized types are not supported");
    case slang::TypeReflection::Kind::OutputStream:
    case slang::TypeReflection::Kind::Feedback:
    case slang::TypeReflection::Kind::Pointer:
        KALI_THROW("Unsupported type");
    }
    KALI_UNREACHABLE();
}

static ref<VariableReflection> reflect_variable(slang::VariableLayoutReflection* variable_layout)
{
    KALI_ASSERT(variable_layout);

    std::string name{variable_layout->getName()};

    ref<TypeReflection> type = reflect_type(variable_layout->getTypeLayout());

    // auto byteOffset = (ShaderVarOffset::ByteOffset)variable_layout->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM);

    return make_ref<VariableReflection>(name, type, ShaderOffset());
}

ProgramLayout::ProgramLayout(slang::ProgramLayout* layout)
    : m_layout(layout)
{
    // m_globals_type = reflect_struct_type(m_layout->getGlobalParamsTypeLayout());

    std::vector<ref<VariableReflection>> globals;
    for (uint32_t i = 0; i < m_layout->getParameterCount(); ++i) {
        ref<VariableReflection> global = reflect_variable(m_layout->getParameterByIndex(i));
        if (global)
            globals.push_back(std::move(global));
    }
    m_globals_type = make_ref<StructTypeReflection>(m_layout->getGlobalParamsTypeLayout(), "", globals);
}

const std::map<uint32_t, std::string>& ProgramLayout::hashed_strings() const
{
    if (m_hashed_strings.empty() && m_layout->getHashedStringCount() > 0) {
        for (uint32_t i = 0; i < m_layout->getHashedStringCount(); ++i) {
            size_t size;
            const char* str = m_layout->getHashedString(i, &size);
            uint32_t hash = spComputeStringHash(str, size);
            m_hashed_strings.emplace(hash, std::string(str, str + size));
        }
    }
    return m_hashed_strings;
}

std::string ProgramLayout::to_string() const
{
    return fmt::format(
        "ProgramLayout(\n"
        "    globals_type={},\n"
        "    hashed_strings={}\n"
        ")",
        indent(globals_type()->to_string()),
        hashed_strings().size()
    );
}

EntryPointLayout::EntryPointLayout(slang::EntryPointLayout* layout)
    : m_layout(layout)
{
}

uint3 EntryPointLayout::compute_thread_group_size() const
{
    SlangUInt size[3];
    m_layout->getComputeThreadGroupSize(3, size);
    return uint3(narrow_cast<uint32_t>(size[0]), narrow_cast<uint32_t>(size[1]), narrow_cast<uint32_t>(size[2]));
}

std::string EntryPointLayout::to_string() const
{
    return fmt::format(
        "EntryPointLayout(\n"
        "    name=\"{}\",\n"
        "    compute_thread_group_size={}\n"
        ")",
        name(),
        compute_thread_group_size()
    );
}

ref<TypeReflection> get_type_reflection(slang::TypeLayoutReflection* slang_type_layout)
{
    return reflect_type(slang_type_layout);
}

} // namespace kali
