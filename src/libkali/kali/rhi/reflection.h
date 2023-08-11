#pragma once

#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/enum.h"

#include "kali/math/vector_types.h"

#include <slang.h>

#include <map>
#include <string>

namespace kali {

class ProgramVersion;

/**
 * Represents the offset of a uniform shader variable relative to its enclosing type/buffer/block.
 *
 * A `UniformShaderVarOffset` is a simple wrapper around a byte offset for a uniform shader variable.
 * It is used to make API signatures less ambiguous (e.g., about whether an integer represents an
 * index, an offset, a count, etc.
 *
 * A `UniformShaderVarOffset` can also encode an invalid offset (represented as an all-ones bit pattern),
 * to indicate that a particular uniform variable is not present.
 *
 * A `UniformShaderVarOffset` can be obtained from a reflection type or `ParameterBlock` using the
 * `[]` subscript operator:
 *
 * UniformShaderVarOffset aOffset = pSomeType["a"]; // get offset of field `a` inside `pSomeType`
 * UniformShaderVarOffset bOffset = pBlock["b"]; // get offset of parameter `b` inside parameter block
 */
struct UniformShaderVarOffset {
    /// Underlying byte offset type.
    using ByteOffset = uint32_t;

    /// Default constructor. Creates an invalid offset.
    UniformShaderVarOffset() = default;

    /// Constructor.
    explicit UniformShaderVarOffset(ByteOffset offset)
        : m_offset(offset)
    {
    }

    /// Check whether this offset is valid.
    bool is_valid() const { return m_offset != INVALID; }

    /// Get the underlying byte offset.
    ByteOffset get_byte_offset() const { return m_offset; }

    /// Comparison.
    auto operator<=>(const UniformShaderVarOffset&) const = default;

    /// Add an additional byte offset to this offset.
    /// Returns an invalid offset if the current offset is invalid.
    UniformShaderVarOffset operator+(ByteOffset offset) const
    {
        if (!is_valid())
            return {};
        return UniformShaderVarOffset(m_offset + offset);
    }

    /// Adds another offset to this offset.
    /// Returns an invalid offset if either offset is invalid.
    UniformShaderVarOffset operator+(UniformShaderVarOffset other) const
    {
        if (!is_valid() || !other.is_valid())
            return {};
        return UniformShaderVarOffset(m_offset + other.m_offset);
    }

private:
    /// Invalid offset value.
    static constexpr ByteOffset INVALID = ByteOffset(-1);

    ByteOffset m_offset{INVALID};
};

/**
 * Represents the offset of a resource-type shader variable relative to its enclosing type/buffer/block.
 *
 * A `ResourceShaderVarOffset` records the index of a descriptor range and an array index within that range.
 *
 * A `ResourceShaderVarOffset` can also encode an invalid offset (represented as an all-ones bit pattern
 * for both the range and array indices), to indicate that a particular resource variable is not present.
 *
 * A `ResourceShaderVarOffset` can be obtained from a reflection type or `ParameterBlock` using the
 * `[]` subscript operator:
 *
 * ResourceShaderVarOffset texOffset = pSomeType["tex"]; // get offset of texture `tex` inside `pSomeType`
 * ResourceShaderVarOffset sampOffset = pBlock["samp"]; // get offset of sampler `samp` inside block
 *
 * Please note that the concepts of resource "ranges" are largely an implementation detail of
 * the `ParameterBlock` type, and most user code should not attempt to explicitly work with
 * or reason about resource ranges. In particular, there is *no* correspondence between resource
 * range indices and the `register`s or `binding`s assigned to shader parameters.
 */
struct ResourceShaderVarOffset {
public:
    using RangeIndex = uint32_t;
    using ArrayIndex = uint32_t;

    /// Default constructor. Creates an invalid offset.
    ResourceShaderVarOffset() = default;

    /// Constructor.
    explicit ResourceShaderVarOffset(RangeIndex range_index, ArrayIndex array_index = 0)
        : m_range_index(range_index)
        , m_array_index(array_index)
    {
    }

    /// Check whether this offset is valid.
    bool is_valid() const { return m_range_index != INVALID; }

    /// Get the underlying resource/descriptor range index.
    RangeIndex get_range_index() const { return m_range_index; }

    /// Get the underlying array index into the resource/descriptor range.
    ArrayIndex get_array_index() const { return m_array_index; }

    /// Comparison.
    auto operator<=>(const ResourceShaderVarOffset&) const = default;

    /// Adds another offset to this offset.
    /// Returns an invalid offset if either offset is invalid.
    ResourceShaderVarOffset operator+(ResourceShaderVarOffset other) const
    {
        if (!is_valid() || !other.is_valid())
            return {};
        return ResourceShaderVarOffset(m_range_index + other.m_range_index, m_array_index + other.m_array_index);
    }

private:
    /// Invalid range index value.
    static constexpr RangeIndex INVALID = RangeIndex(-1);

    RangeIndex m_range_index{INVALID};
    ArrayIndex m_array_index{0};
};

/**
 * Represents the offset of a shader variable relative to its enclosing type/buffer/block.
 *
 * A `ShaderVarOffset` can be used to store the offset of a shader variable that might use
 * ordinary/uniform data, resources like textures/buffers/samplers, or some combination.
 * It effectively stores both a `UniformShaderVarOffset` and a `ResourceShaderVarOffset`
 *
 * A `ShaderVarOffset` can also encode an invalid offset, to indicate that a particular
 * shader variable is not present.
 *
 * A `ShaderVarOffset` can be obtained from a reflection type or `ParameterBlock` using the
 * `[]` subscript operator:
 *
 * ShaderVarOffset lightOffset = pSomeType["light"]; // get offset of variable `light` inside `pSomeType`
 * ShaderVarOffset materialOffset = pBlock["material"]; // get offset of variable `material` inside block
 *
 */
struct ShaderVarOffset {
public:
    using ByteOffset = UniformShaderVarOffset::ByteOffset;
    using RangeIndex = ResourceShaderVarOffset::RangeIndex;
    using ArrayIndex = ResourceShaderVarOffset::ArrayIndex;

    /// Default constructor. Creates an invalid offset.
    ShaderVarOffset() = default;

    /// Constructor.
    explicit ShaderVarOffset(UniformShaderVarOffset uniform, ResourceShaderVarOffset resource)
        : m_uniform(uniform)
        , m_resource(resource)
    {
    }

    /// Check whether this offset is valid.
    bool is_valid() const { return m_uniform.is_valid() && m_resource.is_valid(); }

    /// Get the underlying uniform offset.
    UniformShaderVarOffset get_uniform() const { return m_uniform; }

    /// Implicit conversion to the underlying uniform offset.
    operator UniformShaderVarOffset() const { return m_uniform; }

    /// Get the underlying resource offset.
    ResourceShaderVarOffset get_resource() const { return m_resource; }

    /// Implicit conversion to the underlying resource offset.
    operator ResourceShaderVarOffset() const { return m_resource; }

    /// Get the underlying byte offset.
    ByteOffset get_byte_offset() const { return m_uniform.get_byte_offset(); }

    /// Get the underlying resource/descriptor range index.
    RangeIndex get_range_index() const { return m_resource.get_range_index(); }

    /// Get the underlying array index into the resource/descriptor range.
    ArrayIndex get_array_index() const { return m_resource.get_array_index(); }

    /// Comparison.
    auto operator<=>(const ShaderVarOffset&) const = default;

    /// Adds another offset to this offset.
    /// Returns an invalid offset if either offset is invalid.
    ShaderVarOffset operator+(const ShaderVarOffset& other) const
    {
        if (!is_valid() || !other.is_valid())
            return {};
        return ShaderVarOffset(m_uniform + other.m_uniform, m_resource + other.m_resource);
    }

protected:
    UniformShaderVarOffset m_uniform;
    ResourceShaderVarOffset m_resource;
};

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

    virtual Kind get_kind() const = 0;

    /// Dynamic-cast the current object to StructTypeReflection.
    const StructTypeReflection* as_struct_type() const;

    /// Dynamic-cast the current object to ArrayTypeReflection.
    const ArrayTypeReflection* as_array_type() const;

    /// Dynamic-cast the current object to BasicTypeReflection.
    const BasicTypeReflection* as_basic_type() const;

    /// Dynamic-cast the current object to ResourceTypeReflection.
    const ResourceTypeReflection* as_resource_type() const;

    /// Dynamic cast to InterfaceTypeReflection.
    const InterfaceTypeReflection* as_interface_type() const;
};

KALI_ENUM_REGISTER(TypeReflection::Kind);

class StructTypeReflection : public TypeReflection {
    KALI_OBJECT(StructTypeReflection)
public:
    StructTypeReflection(std::string name, std::vector<ref<VariableReflection>> members)
        : m_name(std::move(name))
        , m_members(std::move(members))
    {
    }

    Kind get_kind() const override { return Kind::struct_; }

    const std::string& get_name() const { return m_name; }

    const std::vector<ref<VariableReflection>> get_members() const { return m_members; }

    VariableReflection* find_member(std::string_view name) const;

    void add_member(ref<VariableReflection> member) { m_members.push_back(std::move(member)); }

private:
    std::string m_name;
    std::vector<ref<VariableReflection>> m_members;
};

class ArrayTypeReflection : public TypeReflection {
    KALI_OBJECT(ArrayTypeReflection)
public:
    ArrayTypeReflection(ref<TypeReflection> element_type, uint32_t element_count, uint32_t element_stride)
        : m_element_type(std::move(element_type))
        , m_element_count(element_count)
        , m_element_stride(element_stride)
    {
    }

    Kind get_kind() const override { return Kind::array; }

    /// Get the type of the elements in the array.
    TypeReflection* get_element_type() const { return m_element_type; }

    /// Get the number of elements in the array.
    uint32_t get_element_count() const { return m_element_count; }

    /// Get the stride of the elements in the array (i.e. the number of bytes between elements).
    /// Note: This does not necessarily match the size of the element type.
    uint32_t get_element_stride() const { return m_element_stride; }

private:
    ref<TypeReflection> m_element_type;
    uint32_t m_element_count;
    uint32_t m_element_stride;
};

class BasicTypeReflection : public TypeReflection {
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

    BasicTypeReflection(ScalarType scalar_type, uint8_t row_count, uint8_t col_count, bool is_row_major)
        : m_scalar_type(scalar_type)
        , m_row_count(row_count)
        , m_col_count(col_count)
        , m_row_major(is_row_major)
    {
    }

    Kind get_kind() const override { return Kind::basic; }

    ScalarType get_scalar_type() const { return m_scalar_type; }
    uint8_t get_row_count() const { return m_row_count; }
    uint8_t get_col_count() const { return m_col_count; }
    bool is_row_major() const { return m_row_major != 0; }

    bool is_vector() const { return (m_row_count == 1 && m_col_count > 1) || (m_row_count > 1 && m_col_count == 1); }
    bool is_matrix() const { return m_row_count > 1 && m_col_count > 1; }

private:
    ScalarType m_scalar_type;
    uint8_t m_row_count;
    uint8_t m_col_count;
    uint8_t m_row_major;
};

KALI_ENUM_REGISTER(BasicTypeReflection::ScalarType);

class ResourceTypeReflection : public TypeReflection {
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
        Type type,
        Dimensions dimensions,
        StructuredType structured_type,
        ReturnType return_type,
        ShaderAccess shader_access,
        ref<TypeReflection> element_type
    )
        : m_type(type)
        , m_dimensions(dimensions)
        , m_structured_type(structured_type)
        , m_return_type(return_type)
        , m_shader_access(shader_access)
        , m_element_type(std::move(element_type))
    {
    }

    Kind get_kind() const override { return Kind::resource; }

    Type get_type() const { return m_type; }
    Dimensions get_dimensions() const { return m_dimensions; }
    StructuredType get_structured_type() const { return m_structured_type; }
    ReturnType get_return_type() const { return m_return_type; }
    ShaderAccess get_shader_access() const { return m_shader_access; }
    TypeReflection* get_element_type() const { return m_element_type; }

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

class InterfaceTypeReflection : public TypeReflection {
    KALI_OBJECT(InterfaceTypeReflection)
public:
    InterfaceTypeReflection() { }

    Kind get_kind() const override { return Kind::interface; }
};

class KALI_API VariableReflection : public Object {
    KALI_OBJECT(VariableReflection)
public:
    VariableReflection(std::string name, ref<TypeReflection> type, const ShaderVarOffset& offset)
        : m_name(std::move(name))
        , m_type(std::move(type))
        , m_offset(offset)
    {
    }

    const std::string& get_name() const { return m_name; }

    TypeReflection* get_type() const { return m_type; }

    const ShaderVarOffset& get_offset() const { return m_offset; }

private:
    std::string m_name;
    ref<TypeReflection> m_type;
    ShaderVarOffset m_offset;
};

#if 0
class TypeReflection {
public:
    TypeReflection(slang::TypeReflection* type)
        : m_type(type)
    {
    }

    enum class Kind {
        none = SLANG_TYPE_KIND_NONE,
        struct_ = SLANG_TYPE_KIND_STRUCT,
        array = SLANG_TYPE_KIND_ARRAY,
        matrix = SLANG_TYPE_KIND_MATRIX,
        vector = SLANG_TYPE_KIND_VECTOR,
        scalar = SLANG_TYPE_KIND_SCALAR,
        constant_buffer = SLANG_TYPE_KIND_CONSTANT_BUFFER,
        resource = SLANG_TYPE_KIND_RESOURCE,
        sampler_state = SLANG_TYPE_KIND_SAMPLER_STATE,
        texture_buffer = SLANG_TYPE_KIND_TEXTURE_BUFFER,
        shader_storage_buffer = SLANG_TYPE_KIND_SHADER_STORAGE_BUFFER,
        parameter_block = SLANG_TYPE_KIND_PARAMETER_BLOCK,
        generic_type_parameter = SLANG_TYPE_KIND_GENERIC_TYPE_PARAMETER,
        interface = SLANG_TYPE_KIND_INTERFACE,
        output_stream = SLANG_TYPE_KIND_OUTPUT_STREAM,
        specialized = SLANG_TYPE_KIND_SPECIALIZED,
        feedback = SLANG_TYPE_KIND_FEEDBACK,
        pointer = SLANG_TYPE_KIND_POINTER,
    };

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

    Kind get_kind() const { return Kind(m_type->getKind()); }

    bool is_struct() const { return get_kind() == Kind::struct_; }

    bool is_array() const { return get_kind() == Kind::array; }

    TypeReflection unwrap_array() const
    {
        if (!is_array())
            KALI_THROW("Type is not array");
        TypeReflection type = *this;
        // while (type.is_array())
        //     type = type.get_element_type();
        return type;
    }

    bool is_scalar() const { return get_kind() == Kind::scalar; }

    ScalarType get_scalar_type() const
    {
        if (!is_scalar())
            KALI_THROW("Type is not scalar");

        return ScalarType(m_type->getScalarType());
    }


private:
    slang::TypeReflection* m_type;
};

class TypeLayoutReflection {
public:
    TypeLayoutReflection(slang::TypeLayoutReflection* type_layout)
        : m_type_layout(type_layout)
    {
    }

    TypeReflection get_type() const { return TypeReflection(m_type_layout->getType()); }

private:
    slang::TypeLayoutReflection* m_type_layout;
};

class VariableLayoutReflection {
public:
    VariableLayoutReflection(slang::VariableLayoutReflection* layout)
        : m_layout(layout)
    {
    }

    const char* get_name() const { return m_layout->getName(); }

    TypeLayoutReflection get_type_layout() const { return TypeLayoutReflection(m_layout->getTypeLayout()); }

private:
    slang::VariableLayoutReflection* m_layout;
};
#endif

class KALI_API ProgramLayout {
public:
    ProgramLayout(const ProgramVersion* program_version, slang::ProgramLayout* layout);

    // TypeLayoutReflection get_globals_type_layout() const
    // {
    //     return TypeLayoutReflection(m_layout->getGlobalParamsTypeLayout());
    // }

    /// Return a hash to string map of all hashed strings in the program.
    std::map<uint32_t, std::string> get_hashed_strings() const;

    void dump();

private:
    const ProgramVersion* m_program_version;
    slang::ProgramLayout* m_layout;
    ref<StructTypeReflection> m_globals_type;
};

class KALI_API EntryPointLayout {
public:
    EntryPointLayout(const ProgramVersion* program_version, uint32_t entry_point, slang::EntryPointLayout* layout)
        : m_program_version(program_version)
        , m_entry_point(entry_point)
        , m_layout(layout)
    {
    }

    uint3 get_compute_thread_group_size() const;

private:
    const ProgramVersion* m_program_version;
    uint32_t m_entry_point;
    slang::EntryPointLayout* m_layout;
};

} // namespace kali
