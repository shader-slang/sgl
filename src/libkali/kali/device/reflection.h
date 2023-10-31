#pragma once

#include "kali/device/shader_offset.h"

#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/enum.h"

#include "kali/math/vector_types.h"

#include <slang.h>
#include <slang-com-ptr.h>

#include <map>
#include <string>

namespace kali {

class TypeReflection;
class ArrayTypeReflection;
class StructTypeReflection;
class BasicTypeReflection;
class ResourceTypeReflection;
class InterfaceTypeReflection;
class VariableReflection;

class KALI_API TypeReflection : public Object {
    KALI_OBJECT(TypeReflection)
public:
    TypeReflection(slang::TypeLayoutReflection* slang_type_layout)
        : m_slang_type_layout(slang_type_layout)
    {
    }

    enum class Kind {
        struct_,   ///< StructTypeReflection
        array,     ///< ArrayTypeReflection
        basic,     ///< BasicTypeReflection
        resource,  ///< ResourceTypeReflection
        interface, ///< InterfaceTypeReflection
    };
    KALI_ENUM_INFO(
        Kind,
        {
            {Kind::struct_, "struct"},
            {Kind::array, "array"},
            {Kind::basic, "basic"},
            {Kind::resource, "resource"},
            {Kind::interface, "interface"},
        }
    );

    /// The kind of type.
    virtual Kind kind() const = 0;

    /// Dynamically cast to \c StructTypeReflection.
    const StructTypeReflection* as_struct_type() const;

    /// Dynamically cast to \c ArrayTypeReflection.
    const ArrayTypeReflection* as_array_type() const;

    /// Dynamically cast to \c BasicTypeReflection.
    const BasicTypeReflection* as_basic_type() const;

    /// Dynamically cast to \c ResourceTypeReflection.
    const ResourceTypeReflection* as_resource_type() const;

    /// Dynamically cast to \c InterfaceTypeReflection.
    const InterfaceTypeReflection* as_interface_type() const;

    slang::TypeLayoutReflection* get_slang_type_layout() const { return m_slang_type_layout; }

    virtual std::string to_string() const override;

protected:
    slang::TypeLayoutReflection* m_slang_type_layout;
};

KALI_ENUM_REGISTER(TypeReflection::Kind);

class KALI_API StructTypeReflection : public TypeReflection {
    KALI_OBJECT(StructTypeReflection)
public:
    StructTypeReflection(
        slang::TypeLayoutReflection* slang_type_layout,
        std::string name,
        std::vector<ref<VariableReflection>> members
    )
        : TypeReflection(slang_type_layout)
        , m_name(std::move(name))
        , m_members(std::move(members))
    {
    }

    Kind kind() const override { return Kind::struct_; }

    /// The struct name.
    const std::string& name() const { return m_name; }

    /// The struct members.
    const std::vector<ref<VariableReflection>>& members() const { return m_members; }

    VariableReflection* find_member(std::string_view name) const;

    void add_member(ref<VariableReflection> member) { m_members.push_back(std::move(member)); }

    virtual std::string to_string() const override;

private:
    std::string m_name;
    std::vector<ref<VariableReflection>> m_members;
};

class KALI_API ArrayTypeReflection : public TypeReflection {
    KALI_OBJECT(ArrayTypeReflection)
public:
    ArrayTypeReflection(
        slang::TypeLayoutReflection* slang_type_layout,
        ref<TypeReflection> element_type,
        uint32_t element_count,
        uint32_t element_stride
    )
        : TypeReflection(slang_type_layout)
        , m_element_type(std::move(element_type))
        , m_element_count(element_count)
        , m_element_stride(element_stride)
    {
    }

    Kind kind() const override { return Kind::array; }

    /// The type of the elements in the array.
    TypeReflection* element_type() const { return m_element_type; }

    /// The number of elements in the array.
    uint32_t element_count() const { return m_element_count; }

    /// The stride of the elements in the array (i.e. the number of bytes between elements).
    /// \note This does not necessarily match the size of the element type.
    uint32_t element_stride() const { return m_element_stride; }

    virtual std::string to_string() const override;

private:
    ref<TypeReflection> m_element_type;
    uint32_t m_element_count;
    uint32_t m_element_stride;
};

class KALI_API BasicTypeReflection : public TypeReflection {
    KALI_OBJECT(BasicTypeReflection)
public:
    enum class ScalarType {
        none = SLANG_SCALAR_TYPE_NONE,
        void_ = SLANG_SCALAR_TYPE_VOID,
        bool_ = SLANG_SCALAR_TYPE_BOOL,
        int32 = SLANG_SCALAR_TYPE_INT32,
        uint32 = SLANG_SCALAR_TYPE_UINT32,
        int64 = SLANG_SCALAR_TYPE_INT64,
        uint64 = SLANG_SCALAR_TYPE_UINT64,
        float16 = SLANG_SCALAR_TYPE_FLOAT16,
        float32 = SLANG_SCALAR_TYPE_FLOAT32,
        float64 = SLANG_SCALAR_TYPE_FLOAT64,
        int8 = SLANG_SCALAR_TYPE_INT8,
        uint8 = SLANG_SCALAR_TYPE_UINT8,
        int16 = SLANG_SCALAR_TYPE_INT16,
        uint16 = SLANG_SCALAR_TYPE_UINT16,
    };
    KALI_ENUM_INFO(
        ScalarType,
        {
            {ScalarType::none, "none"},
            {ScalarType::void_, "void"},
            {ScalarType::bool_, "bool"},
            {ScalarType::int32, "int32"},
            {ScalarType::uint32, "uint32"},
            {ScalarType::int64, "int64"},
            {ScalarType::uint64, "uint64"},
            {ScalarType::float16, "float16"},
            {ScalarType::float32, "float32"},
            {ScalarType::float64, "float64"},
            {ScalarType::int8, "int8"},
            {ScalarType::uint8, "uint8"},
            {ScalarType::int16, "int16"},
            {ScalarType::uint16, "uint16"},
        }
    );

    enum class CompositeType {
        // clang-format off
        // Scalar & vector types
        bool_, bool2, bool3, bool4,
        uint8, uint8_2, uint8_3, uint8_4,
        uint16, uint16_2, uint16_3, uint16_4,
        uint, uint2, uint3, uint4,
        uint64, uint64_2, uint64_3, uint64_4,
        int8, int8_2, int8_3, int8_4,
        int16, int16_2, int16_3, int16_4,
        int_, int2, int3, int4,
        int64, int64_2, int64_3, int64_4,
        float16, float16_2, float16_3, float16_4,
        float_, float2, float3, float4,
        float64, float64_2, float64_3, float64_4,
        // Matrix types.
        // clang-format on

        Float16_2x2,
        Float16_2x3,
        Float16_2x4,
        Float16_3x2,
        Float16_3x3,
        Float16_3x4,
        Float16_4x2,
        Float16_4x3,
        Float16_4x4,

        Float2x2,
        Float2x3,
        Float2x4,
        Float3x2,
        Float3x3,
        Float3x4,
        Float4x2,
        Float4x3,
        Float4x4,


        Unknown = -1
    };

    BasicTypeReflection(
        slang::TypeLayoutReflection* slang_type_layout,
        ScalarType scalar_type,
        uint8_t row_count,
        uint8_t col_count,
        bool is_row_major
    )
        : TypeReflection(slang_type_layout)
        , m_scalar_type(scalar_type)
        , m_row_count(row_count)
        , m_col_count(col_count)
        , m_row_major(is_row_major)
    {
    }

    Kind kind() const override { return Kind::basic; }

    ScalarType scalar_type() const { return m_scalar_type; }
    uint8_t row_count() const { return m_row_count; }
    uint8_t col_count() const { return m_col_count; }
    bool is_row_major() const { return m_row_major != 0; }

    bool is_vector() const { return (m_row_count == 1 && m_col_count > 1) || (m_row_count > 1 && m_col_count == 1); }
    bool is_matrix() const { return m_row_count > 1 && m_col_count > 1; }

    virtual std::string to_string() const override;

private:
    ScalarType m_scalar_type;
    uint8_t m_row_count;
    uint8_t m_col_count;
    uint8_t m_row_major;
};

KALI_ENUM_REGISTER(BasicTypeReflection::ScalarType);

class KALI_API ResourceTypeReflection : public TypeReflection {
    KALI_OBJECT(ResourceTypeReflection)
public:
    /// Type of the resource.
    enum class Type {
        texture,
        structured_buffer,
        raw_buffer,
        typed_buffer,
        sampler,
        constant_buffer,
        acceleration_structure,
    };
    KALI_ENUM_INFO(
        Type,
        {
            {Type::texture, "texture"},
            {Type::structured_buffer, "structured_buffer"},
            {Type::raw_buffer, "raw_buffer"},
            {Type::typed_buffer, "typed_buffer"},
            {Type::sampler, "sampler"},
            {Type::constant_buffer, "constant_buffer"},
            {Type::acceleration_structure, "acceleration_structure"},
        }
    );

    /// Dimensions of the resource.
    enum class Dimensions {
        unknown,
        texture1d,
        texture2d,
        texture3d,
        texture_cube,
        texture1d_array,
        texture2d_array,
        texture2dms,
        texture2dms_array,
        texture_cube_array,
        acceleration_structure,
        buffer,
    };
    KALI_ENUM_INFO(
        Dimensions,
        {
            {Dimensions::unknown, "unknown"},
            {Dimensions::texture1d, "texture1d"},
            {Dimensions::texture2d, "texture2d"},
            {Dimensions::texture3d, "texture3d"},
            {Dimensions::texture_cube, "texture_cube"},
            {Dimensions::texture1d_array, "texture1d_array"},
            {Dimensions::texture2d_array, "texture2d_array"},
            {Dimensions::texture2dms, "texture2dms"},
            {Dimensions::texture2dms_array, "texture2dms_array"},
            {Dimensions::texture_cube_array, "texture_cube_array"},
            {Dimensions::acceleration_structure, "acceleration_structure"},
            {Dimensions::buffer, "buffer"},
        }
    );

    /// For Type::structured_buffer, describes the type of the buffer.
    enum class StructuredType {
        none,    ///< Not a structured buffer
        regular, ///< Regular structured buffer
        counter, ///< RWStructuredBuffer with counter
        append,  ///< AppendStructuredBuffer
        consume  ///< ConsumeStructuredBuffer
    };
    KALI_ENUM_INFO(
        StructuredType,
        {
            {StructuredType::none, "none"},
            {StructuredType::regular, "regular"},
            {StructuredType::counter, "counter"},
            {StructuredType::append, "append"},
            {StructuredType::consume, "consume"},
        }
    );

    /// The expected return type.
    enum class ReturnType { unknown, float_, double_, int_, uint };
    KALI_ENUM_INFO(
        ReturnType,
        {
            {ReturnType::unknown, "unknown"},
            {ReturnType::float_, "float"},
            {ReturnType::double_, "double"},
            {ReturnType::int_, "int"},
            {ReturnType::uint, "uint"},
        }
    );

    /// Describes how the shader will access the resource.
    enum class ShaderAccess { undefined, read, read_write };
    KALI_ENUM_INFO(
        ShaderAccess,
        {
            {ShaderAccess::undefined, "undefined"},
            {ShaderAccess::read, "read"},
            {ShaderAccess::read_write, "read_write"},
        }
    );

    ResourceTypeReflection(
        slang::TypeLayoutReflection* slang_type_layout,
        Type type,
        Dimensions dimensions,
        StructuredType structured_type,
        ReturnType return_type,
        ShaderAccess shader_access,
        ref<TypeReflection> element_type
    )
        : TypeReflection(slang_type_layout)
        , m_type(type)
        , m_dimensions(dimensions)
        , m_structured_type(structured_type)
        , m_return_type(return_type)
        , m_shader_access(shader_access)
        , m_element_type(std::move(element_type))
    {
    }

    Kind kind() const override { return Kind::resource; }

    Type type() const { return m_type; }
    Dimensions dimensions() const { return m_dimensions; }
    StructuredType structured_type() const { return m_structured_type; }
    ReturnType return_type() const { return m_return_type; }
    ShaderAccess shader_access() const { return m_shader_access; }
    TypeReflection* element_type() const { return m_element_type; }

    virtual std::string to_string() const override;

private:
    Type m_type;
    Dimensions m_dimensions;
    StructuredType m_structured_type;
    ReturnType m_return_type;
    ShaderAccess m_shader_access;
    ref<TypeReflection> m_element_type;
};

KALI_ENUM_REGISTER(ResourceTypeReflection::Type);
KALI_ENUM_REGISTER(ResourceTypeReflection::Dimensions);
KALI_ENUM_REGISTER(ResourceTypeReflection::StructuredType);
KALI_ENUM_REGISTER(ResourceTypeReflection::ReturnType);
KALI_ENUM_REGISTER(ResourceTypeReflection::ShaderAccess);

class KALI_API InterfaceTypeReflection : public TypeReflection {
    KALI_OBJECT(InterfaceTypeReflection)
public:
    InterfaceTypeReflection(slang::TypeLayoutReflection* slang_type_layout)
        : TypeReflection(slang_type_layout)
    {
    }

    Kind kind() const override { return Kind::interface; }

    virtual std::string to_string() const override;
};

class KALI_API VariableReflection : public Object {
    KALI_OBJECT(VariableReflection)
public:
    VariableReflection(std::string name, ref<TypeReflection> type, const ShaderOffset& offset)
        : m_name(std::move(name))
        , m_type(std::move(type))
        , m_offset(offset)
    {
    }

    const std::string& name() const { return m_name; }

    TypeReflection* type() const { return m_type; }

    const ShaderOffset& offset() const { return m_offset; }

    virtual std::string to_string() const override;

private:
    std::string m_name;
    ref<TypeReflection> m_type;
    ShaderOffset m_offset;
};

class KALI_API ProgramLayout : public Object {
    KALI_OBJECT(ProgramLayout)
public:
    ProgramLayout(slang::ProgramLayout* layout);

    // TypeLayoutReflection get_globals_type_layout() const
    // {
    //     return TypeLayoutReflection(m_layout->getGlobalParamsTypeLayout());
    // }

    const ref<StructTypeReflection>& globals_type() const { return m_globals_type; }

    /// Return a hash to string map of all hashed strings in the program.
    const std::map<uint32_t, std::string>& hashed_strings() const;

    virtual std::string to_string() const override;

private:
    slang::ProgramLayout* m_layout;
    ref<StructTypeReflection> m_globals_type;
    mutable std::map<uint32_t, std::string> m_hashed_strings;
};

class KALI_API EntryPointLayout : public Object {
    KALI_OBJECT(EntryPointLayout)
public:
    EntryPointLayout(slang::EntryPointLayout* layout);

    /// The name of the entry point.
    std::string name() const { return m_layout->getName(); }

    /// The thread group size if this is a compute stage entry point.
    uint3 compute_thread_group_size() const;

    virtual std::string to_string() const override;

private:
    slang::EntryPointLayout* m_layout;
};

// TODO temporary utilities

KALI_API ref<TypeReflection> get_type_reflection(slang::TypeLayoutReflection* slang_type_layout);

} // namespace kali
