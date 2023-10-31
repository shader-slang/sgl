#include "reflection.h"

#include "kali/core/type_utils.h"
#include "kali/core/format.h"

namespace kali {

class StringWriter {
public:
    void indent(int offset)
    {
        m_indent = std::max(0, m_indent + offset);
        m_padding = std::string(m_indent, ' ');
    }

    void print(std::string_view str) { m_str += str; }

    void println(std::string_view str)
    {
        m_str += m_padding;
        m_str += str;
        m_str += '\n';
    }

    template<typename... Args>
    void print(fmt::format_string<Args...> fmt, Args&&... args)
    {
        print(fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void println(fmt::format_string<Args...> fmt, Args&&... args)
    {
        println(fmt::format(fmt, std::forward<Args>(args)...));
    }

    StringWriter& operator()(std::string_view str)
    {
        println(str);
        return *this;
    }

    template<typename... Args>
    StringWriter& operator()(fmt::format_string<Args...> fmt, Args&&... args)
    {
        println(fmt::format(fmt, std::forward<Args>(args)...));
        return *this;
    }

    std::string str() const { return m_str; }

private:
    std::string m_str;
    int m_indent{0};
    std::string m_padding;
};

// ----------------------------------------------------------------------------
// TypeReflection
// ----------------------------------------------------------------------------

const StructTypeReflection* TypeReflection::as_struct_type() const
{
    return get_kind() == Kind::struct_ ? static_cast<const StructTypeReflection*>(this) : nullptr;
}

const ArrayTypeReflection* TypeReflection::as_array_type() const
{
    return get_kind() == Kind::array ? static_cast<const ArrayTypeReflection*>(this) : nullptr;
}

const BasicTypeReflection* TypeReflection::as_basic_type() const
{
    return get_kind() == Kind::basic ? static_cast<const BasicTypeReflection*>(this) : nullptr;
}

const ResourceTypeReflection* TypeReflection::as_resource_type() const
{
    return get_kind() == Kind::resource ? static_cast<const ResourceTypeReflection*>(this) : nullptr;
}

const InterfaceTypeReflection* TypeReflection::as_interface_type() const
{
    return get_kind() == Kind::interface ? static_cast<const InterfaceTypeReflection*>(this) : nullptr;
}

// ----------------------------------------------------------------------------
// StructTypeReflection
// ----------------------------------------------------------------------------

VariableReflection* StructTypeReflection::find_member(std::string_view name) const
{
    for (const auto& member : m_members) {
        if (member->get_name() == name)
            return member;
    }
    return nullptr;
}

// ----------------------------------------------------------------------------
// ArrayTypeReflection
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// BasicTypeReflection
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// ResourceTypeReflection
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// InterfaceReflectionType
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// VariableReflection
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// InterfaceReflectionType
// ----------------------------------------------------------------------------

class ReflectionWriter : public StringWriter {
public:
    void write(const TypeReflection* type)
    {
        if (!type)
            return;
        if (type->as_struct_type()) {
            write(type->as_struct_type());
        } else if (type->as_array_type()) {
            write(type->as_array_type());
        } else if (type->as_basic_type()) {
            write(type->as_basic_type());
        } else if (type->as_resource_type()) {
            write(type->as_resource_type());
        } else if (type->as_interface_type()) {
            write(type->as_interface_type());
        }
    }

    void write(const StructTypeReflection* type)
    {
        println("StructTypeReflection(name=\"{}\")", type->get_name());
        indent(4);
        for (auto&& member : type->get_members()) {
            write(member.get());
        }
        indent(-4);
    }

    void write(const ArrayTypeReflection* type)
    {
        println(
            "ArrayTypeReflection(element_count={}, element_stride={})",
            type->get_element_count(),
            type->get_element_stride()
        );
        indent(4);
        write(type->get_element_type());
        indent(-4);
    }

    void write(const BasicTypeReflection* type)
    {
        KALI_UNUSED(type);
        println(
            "BasicTypeReflection(scalar_type={}, row_count={}, col_count={}, is_row_major={})",
            enum_to_string(type->get_scalar_type()),
            type->get_row_count(),
            type->get_col_count(),
            type->is_row_major()
        );
    }

    void write(const ResourceTypeReflection* type)
    {
        KALI_UNUSED(type);
        println(
            "ResourceTypeReflection(type={}, dimensions={}, structured_type={}, return_type={}, access={})",
            enum_to_string(type->get_type()),
            enum_to_string(type->get_dimensions()),
            enum_to_string(type->get_structured_type()),
            enum_to_string(type->get_return_type()),
            enum_to_string(type->get_shader_access())
        );
        indent(4);
        write(type->get_element_type());
        indent(-4);
    }

    void write(const InterfaceTypeReflection* type)
    {
        KALI_UNUSED(type);
        println("InterfaceTypeReflection()");
    }

    void write(const VariableReflection* var)
    {
        println("VariableReflection(name=\"{}\")", var->get_name());
        indent(4);
        write(var->get_type());
        indent(-4);
    }
};


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

        ref<VariableReflection> member = reflect_variable(field); //, pType->getResourceRangeCount(), pBlock,
                                                                  //&fieldPath, pProgramVersion);
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

static ref<TypeReflection> reflect_type(slang::TypeLayoutReflection* type_layout
                                        // ParameterBlockReflection* pBlock,
                                        // ReflectionPath* pPath,
                                        // ProgramVersion const* pProgramVersion
)
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

std::map<uint32_t, std::string> ProgramLayout::get_hashed_strings() const
{
    std::map<uint32_t, std::string> result;
    for (uint32_t i = 0; i < m_layout->getHashedStringCount(); ++i) {
        size_t size;
        const char* str = m_layout->getHashedString(i, &size);
        uint32_t hash = spComputeStringHash(str, size);
        result.emplace(hash, std::string(str, str + size));
    }
    return result;
}

void ProgramLayout::dump()
{
    ReflectionWriter writer;
    writer.write(m_globals_type);
    log_info("\n\n{}\n\n", writer.str());
}

EntryPointLayout::EntryPointLayout(slang::EntryPointLayout* layout)
    : m_layout(layout)
{
}

uint3 EntryPointLayout::get_compute_thread_group_size() const
{
    SlangUInt size[3];
    m_layout->getComputeThreadGroupSize(3, size);
    return uint3(narrow_cast<uint32_t>(size[0]), narrow_cast<uint32_t>(size[1]), narrow_cast<uint32_t>(size[2]));
}

ref<TypeReflection> get_type_reflection(slang::TypeLayoutReflection* slang_type_layout)
{
    return reflect_type(slang_type_layout);
}

std::string dump_type_reflection(const TypeReflection* type_reflection)
{
    ReflectionWriter writer;
    writer.write(type_reflection);
    return writer.str();
}

} // namespace kali
