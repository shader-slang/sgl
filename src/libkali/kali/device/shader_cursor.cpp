#include "shader_cursor.h"

#include "kali/device/shader_object.h"
#include "kali/device/resource.h"
#include "kali/device/cuda_interop.h"

#include "kali/core/error.h"

#include "kali/math/vector_types.h"
#include "kali/math/matrix_types.h"

namespace kali {

// Helper class for checking if implicit conversion between scalar types is allowed.
// Note that only conversion between types of the same size is allowed.
struct ScalarConversionTable {
    static_assert(size_t(TypeReflection::ScalarType::COUNT) < 32, "Not enough bits to represent all scalar types");
    constexpr ScalarConversionTable()
    {
        for (uint32_t i = 0; i < uint32_t(TypeReflection::ScalarType::COUNT); ++i)
            table[i] = 1 << i;

        auto add_conversion = [&](TypeReflection::ScalarType from, auto... to)
        {
            uint32_t flags{0};
            ((flags |= 1 << uint32_t(to)), ...);
            table[uint32_t(from)] |= flags;
        };

        using ST = TypeReflection::ScalarType;
        add_conversion(ST::int32, ST::uint32, ST::bool_);
        add_conversion(ST::uint32, ST::int32, ST::bool_);
        add_conversion(ST::int64, ST::uint64);
        add_conversion(ST::uint64, ST::int64);
        add_conversion(ST::int8, ST::uint8);
        add_conversion(ST::uint8, ST::int8);
        add_conversion(ST::int16, ST::uint16);
        add_conversion(ST::uint16, ST::int16);
    }

    constexpr bool allow_conversion(TypeReflection::ScalarType from, TypeReflection::ScalarType to) const
    {
        return (table[uint32_t(from)] & (1 << uint32_t(to))) != 0;
    }

    uint32_t table[size_t(TypeReflection::ScalarType::COUNT)]{};
};

inline bool allow_scalar_conversion(TypeReflection::ScalarType from, TypeReflection::ScalarType to)
{
    static constexpr ScalarConversionTable table;
    return table.allow_conversion(from, to);
}

ShaderCursor::ShaderCursor(ShaderObject* shader_object)
    : m_shader_object(shader_object)
    , m_type_layout(shader_object->element_type_layout())
    , m_offset(ShaderOffset::zero())
{
    KALI_ASSERT(m_shader_object);
    KALI_ASSERT(m_type_layout);
    KALI_ASSERT(m_offset.is_valid());
}

std::string ShaderCursor::to_string() const
{
    return "ShaderCursor()";
}

ShaderCursor ShaderCursor::dereference() const
{
    KALI_CHECK(is_valid(), "Invalid cursor");
    switch (m_type_layout->kind()) {
    case TypeReflection::Kind::constant_buffer:
    case TypeReflection::Kind::parameter_block:
        return ShaderCursor(m_shader_object->get_object(m_offset));
    default:
        return {};
    }
}

//
// Navigation
//

ShaderCursor ShaderCursor::operator[](std::string_view name) const
{
    KALI_CHECK(is_valid(), "Invalid cursor");
    ShaderCursor result = find_field(name);
    KALI_CHECK(result.is_valid(), "Field '{}' not found.", name);
    return result;
}

ShaderCursor ShaderCursor::operator[](uint32_t index) const
{
    KALI_CHECK(is_valid(), "Invalid cursor");
    ShaderCursor result = find_element(index);
    KALI_CHECK(result.is_valid(), "Element '{}' not found.", index);
    return result;
}

ShaderCursor ShaderCursor::find_field(std::string_view name) const
{
    if (!is_valid())
        return *this;

    // If the cursor is valid, we want to consider the type of data
    // it is referencing.
    //
    switch (m_type_layout->kind()) {
        // The easy/expected case is when the value has a structure type.
        //
    case TypeReflection::Kind::struct_: {
        // We start by looking up the index of a field matching `name`.
        //
        // If there is no such field, we have an error.
        //
        int32_t field_index = m_type_layout->find_field_index_by_name(name.data(), name.data() + name.size());
        if (field_index < 0)
            break;

        // Once we know the index of the field being referenced,
        // we create a cursor to point at the field, based on
        // the offset information already in this cursor, plus
        // offsets derived from the field's layout.
        //
        const VariableLayoutReflection* field_layout = m_type_layout->get_field_by_index(field_index);
        ShaderCursor field_cursor;

        // The field cursorwill point into the same parent object.
        //
        field_cursor.m_shader_object = m_shader_object;

        // The type being pointed to is the tyep of the field.
        //
        field_cursor.m_type_layout = field_layout->type_layout();

        // The byte offset is the current offset plus the relative offset of the field.
        // The offset in binding ranges is computed similarly.
        //
        field_cursor.m_offset.uniform_offset = m_offset.uniform_offset + narrow_cast<uint32_t>(field_layout->offset());
        field_cursor.m_offset.binding_range_index
            = m_offset.binding_range_index + m_type_layout->get_field_binding_range_offset(field_index);

        // The index of the field within any binding ranges will be the same
        // as the index computed for the parent structure.
        //
        // Note: this case would arise for an array of structures with texture-type
        // fields. Suppose we have:
        //
        //      struct S { Texture2D t; Texture2D u; }
        //      S g[4];
        //
        // In this scenario, `g` holds two binding ranges:
        //
        // * Range #0 comprises 4 textures, representing `g[...].t`
        // * Range #1 comprises 4 textures, representing `g[...].u`
        //
        // A cursor for `g[2]` would have a `binding_range_index` of zero but
        // a `binding_array_index` of 2, iindicating that we could end up
        // referencing either range, but no matter what we know the index
        // is 2. Thus when we form a cursor for `g[2].u` we want to
        // apply the binding range offset to get a `binding_range_index` of
        // 1, while the `binding_array_index` is unmodified.
        //
        // The result is that `g[2].u` is stored in range #1 at array index 2.
        //
        field_cursor.m_offset.binding_array_index = m_offset.binding_array_index;

        return field_cursor;
    }

    // In some cases the user might be trying to acess a field by name
    // from a cursor that references a constant buffer or parameter block,
    // and in these cases we want the access to Just Work.
    //
    case TypeReflection::Kind::constant_buffer:
    case TypeReflection::Kind::parameter_block: {
        // We basically need to "dereference" the current cursor
        // to go from a pointer to a constant buffer to a pointer
        // to the *contents* of the constant buffer.
        //
        ShaderCursor d = dereference();
        return d.find_field(name);
    }

    default:
        break;
    }

#if 0
    // If a cursor is pointing at a root shader object (created for a
    // program), then we will also iterate over the entry point shader
    // objects attached to it and look for a matching parameter name
    // on them.
    //
    // This is a bit of "do what I mean" logic and could potentially
    // lead to problems if there could be multiple entry points with
    // the same parameter name.
    //
    // TODO: figure out whether we should support this long-term.
    //
    auto entryPointCount = (GfxIndex)m_shader_object->getEntryPointCount();
    for (GfxIndex e = 0; e < entryPointCount; ++e) {
        ComPtr<IShaderObject> entryPoint;
        m_shader_object->getEntryPoint(e, entryPoint.writeRef());

        ShaderCursor entryPointCursor(entryPoint);

        auto result = entryPointCursor.getField(name, nameEnd, outCursor);
        if (SLANG_SUCCEEDED(result))
            return result;
    }
#endif
    return {};
}

ShaderCursor ShaderCursor::find_element(uint32_t index) const
{
    if (!is_valid())
        return *this;

#if 0
    if (m_containerType != ShaderObjectContainerType::None) {
        ShaderCursor element_cursor;
        element_cursor.m_shader_object = m_shader_object;
        element_cursor.m_type_layout = m_type_layout->getElementTypeLayout();
        element_cursor.m_containerType = m_containerType;
        element_cursor.m_offset.uniform_offset = index * m_type_layout->getStride();
        element_cursor.m_offset.binding_range_index = 0;
        element_cursor.m_offset.binding_array_index = index;
        return element_cursor;
    }
#endif

    switch (m_type_layout->kind()) {
    case TypeReflection::Kind::array: {
        ShaderCursor element_cursor;
        element_cursor.m_shader_object = m_shader_object;
        element_cursor.m_type_layout = m_type_layout->element_type_layout();
        element_cursor.m_offset.uniform_offset
            = m_offset.uniform_offset + index * narrow_cast<uint32_t>(m_type_layout->element_stride());
        element_cursor.m_offset.binding_range_index = m_offset.binding_range_index;
        element_cursor.m_offset.binding_array_index
            = m_offset.binding_array_index * narrow_cast<uint32_t>(m_type_layout->element_count()) + index;
        return element_cursor;
    } break;

    case TypeReflection::Kind::vector:
    case TypeReflection::Kind::matrix: {
        ShaderCursor field_cursor;
        field_cursor.m_shader_object = m_shader_object;
        field_cursor.m_type_layout = m_type_layout->element_type_layout();
        field_cursor.m_offset.uniform_offset
            = m_offset.uniform_offset + narrow_cast<uint32_t>(m_type_layout->element_stride()) * index;
        field_cursor.m_offset.binding_range_index = m_offset.binding_range_index;
        field_cursor.m_offset.binding_array_index = m_offset.binding_array_index;
        return field_cursor;
    } break;

    default:
        break;
    }

    return {};
}

ShaderCursor ShaderCursor::find_entry_point(uint32_t index) const
{
    if (!is_valid())
        return *this;

    // TODO check index
    // uint32_t count = m_shader_object->get_entry_point_count();
    return ShaderCursor(m_shader_object->get_entry_point(index));
}

//
// Resource binding
//

inline bool is_parameter_block(const TypeReflection* type)
{
    return type->kind() == TypeReflection::Kind::parameter_block;
}

inline bool is_resource_type(const TypeReflection* type)
{
    switch (type->kind()) {
    case TypeReflection::Kind::constant_buffer:
    case TypeReflection::Kind::resource:
    case TypeReflection::Kind::sampler_state:
    case TypeReflection::Kind::texture_buffer:
    case TypeReflection::Kind::shader_storage_buffer:
    case TypeReflection::Kind::parameter_block:
        return true;
    default:
        return false;
    }
}

inline bool is_buffer_resource_type(const TypeReflection* type)
{
    switch (type->kind()) {
    case TypeReflection::Kind::constant_buffer:
    case TypeReflection::Kind::resource:
        switch (type->resource_shape() & TypeReflection::ResourceShape::base_shape_mask) {
        case TypeReflection::ResourceShape::texture_buffer:
        case TypeReflection::ResourceShape::structured_buffer:
        case TypeReflection::ResourceShape::byte_address_buffer:
            return true;
        default:
            return false;
        }
        break;
    case TypeReflection::Kind::texture_buffer:
    case TypeReflection::Kind::shader_storage_buffer:
    case TypeReflection::Kind::parameter_block:
        return true;
    default:
        return false;
    }
}

inline bool is_texture_resource_type(const TypeReflection* type)
{
    switch (type->kind()) {
    case TypeReflection::Kind::resource:
        switch (type->resource_shape() & TypeReflection::ResourceShape::base_shape_mask) {
        case TypeReflection::ResourceShape::texture_1d:
        case TypeReflection::ResourceShape::texture_2d:
        case TypeReflection::ResourceShape::texture_3d:
        case TypeReflection::ResourceShape::texture_cube:
            return true;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
}

inline bool is_sampler_type(const TypeReflection* type)
{
    return type->kind() == TypeReflection::Kind::sampler_state;
}

inline bool is_shader_resource_type(const TypeReflection* type)
{
    return type->resource_access() == TypeReflection::ResourceAccess::read;
}

inline bool is_unordered_access_type(const TypeReflection* type)
{
    return type->resource_access() == TypeReflection::ResourceAccess::read_write;
}

inline bool is_acceleration_structure_resource_type(const TypeReflection* type)
{
    return type->kind() == TypeReflection::Kind::resource
        && type->resource_shape() == TypeReflection::ResourceShape::acceleration_structure;
}

void ShaderCursor::set_resource(const ref<ResourceView>& resource_view) const
{
    const TypeReflection* type = m_type_layout->unwrap_array()->type();

    KALI_CHECK(is_resource_type(type), "'{}' cannot bind a resource", m_type_layout->name());

    if (resource_view) {
        if (is_shader_resource_type(type)) {
            KALI_CHECK(
                resource_view->type() == ResourceViewType::shader_resource,
                "'{}' expects a shader resource view",
                m_type_layout->name()
            );
        } else if (is_unordered_access_type(type)) {
            KALI_CHECK(
                resource_view->type() == ResourceViewType::unordered_access,
                "'{}' expects an unordered access view",
                m_type_layout->name()
            );
        } else {
            KALI_THROW("'{}' expects a valid resource view", m_type_layout->name());
        }
    }

    m_shader_object->set_resource(m_offset, resource_view);
}

void ShaderCursor::set_buffer(const ref<Buffer>& buffer) const
{
    const TypeReflection* type = m_type_layout->unwrap_array()->type();

    KALI_CHECK(is_buffer_resource_type(type), "'{}' cannot bind a buffer", m_type_layout->name());

    if (buffer) {
        if (is_shader_resource_type(type)) {
            set_resource(buffer->get_srv());
        } else if (is_unordered_access_type(type)) {
            set_resource(buffer->get_uav());
        } else {
            KALI_THROW("'{}' expects a valid buffer", m_type_layout->name());
        }
    } else {
        set_resource(nullptr);
    }
}

void ShaderCursor::set_texture(const ref<Texture>& texture) const
{
    const TypeReflection* type = m_type_layout->unwrap_array()->type();

    KALI_CHECK(is_texture_resource_type(type), "'{}' cannot bind a texture", m_type_layout->name());

    if (texture) {
        if (is_shader_resource_type(type)) {
            set_resource(texture->get_srv());
        } else if (is_unordered_access_type(type)) {
            set_resource(texture->get_uav());
        } else {
            KALI_THROW("'{}' expects a valid texture", m_type_layout->name());
        }
    } else {
        set_resource(nullptr);
    }
}

void ShaderCursor::set_sampler(const ref<Sampler>& sampler) const
{
    const TypeReflection* type = m_type_layout->unwrap_array()->type();

    KALI_CHECK(is_sampler_type(type), "'{}' cannot bind a sampler", m_type_layout->name());

    m_shader_object->set_sampler(m_offset, sampler);
}

void ShaderCursor::set_acceleration_structure(const ref<AccelerationStructure>& acceleration_structure) const
{
    const TypeReflection* type = m_type_layout->type();

    KALI_CHECK(
        is_acceleration_structure_resource_type(type),
        "'{}' cannot bind an acceleration structure",
        m_type_layout->name()
    );

    m_shader_object->set_acceleration_structure(m_offset, acceleration_structure);
}

void ShaderCursor::set_data(const void* data, size_t size) const
{
    if (m_type_layout->parameter_category() != TypeReflection::ParameterCategory::uniform)
        KALI_THROW("'{}' cannot bind data", m_type_layout->name());
    m_shader_object->set_data(m_offset, data, size);
}

void ShaderCursor::set_object(const ref<MutableShaderObject>& object) const
{
    const TypeReflection* type = m_type_layout->type();

    KALI_CHECK(is_parameter_block(type), "'{}' cannot bind an object", m_type_layout->name());

    m_shader_object->set_object(m_offset, object);
}

#if KALI_HAS_CUDA
void ShaderCursor::set_cuda_tensor_view(const cuda::TensorView& tensor_view) const
{
    const TypeReflection* type = m_type_layout->unwrap_array()->type();

    KALI_CHECK(is_buffer_resource_type(type), "'{}' cannot bind a CUDA tensor view", m_type_layout->name());

    if (is_shader_resource_type(type)) {
        m_shader_object->set_cuda_tensor_view(m_offset, tensor_view, false);
    } else if (is_unordered_access_type(type)) {
        m_shader_object->set_cuda_tensor_view(m_offset, tensor_view, true);
    } else {
        KALI_THROW("'{}' expects a valid buffer", m_type_layout->name());
    }
}
#endif

void ShaderCursor::set_scalar(const void* data, size_t size, TypeReflection::ScalarType scalar_type) const
{
    const TypeReflection* type = m_type_layout->unwrap_array()->type();

    KALI_CHECK(type->kind() == TypeReflection::Kind::scalar, "'{}' cannot bind a scalar value", m_type_layout->name());
    KALI_CHECK(
        allow_scalar_conversion(scalar_type, type->scalar_type()),
        "'{}' expects scalar type {} (no implicit conversion from type {})",
        m_type_layout->name(),
        type->scalar_type(),
        scalar_type
    );

    m_shader_object->set_data(m_offset, data, size);
}

void ShaderCursor::set_vector(const void* data, size_t size, TypeReflection::ScalarType scalar_type, int dimension)
    const
{
    const TypeReflection* type = m_type_layout->unwrap_array()->type();

    KALI_CHECK(type->kind() == TypeReflection::Kind::vector, "'{}' cannot bind a vector value", m_type_layout->name());
    KALI_CHECK(
        type->col_count() == uint32_t(dimension),
        "'{}' expects a vector with dimension {} (got dimension {})",
        m_type_layout->name(),
        type->col_count(),
        dimension
    );
    KALI_CHECK(
        allow_scalar_conversion(scalar_type, type->scalar_type()),
        "'{}' expects a vector with scalar type {} (no implicit conversion from type {})",
        m_type_layout->name(),
        type->scalar_type(),
        scalar_type
    );

    m_shader_object->set_data(m_offset, data, size);
}

void ShaderCursor::set_matrix(const void* data, size_t size, TypeReflection::ScalarType scalar_type, int rows, int cols)
    const
{
    const TypeReflection* type = m_type_layout->unwrap_array()->type();

    KALI_CHECK(type->kind() == TypeReflection::Kind::matrix, "'{}' cannot bind a matrix value", m_type_layout->name());
    KALI_CHECK(
        type->row_count() == uint32_t(rows) && type->col_count() == uint32_t(cols),
        "'{}' expects a matrix with dimension {}x{} (got dimension {}x{})",
        m_type_layout->name(),
        type->row_count(),
        type->col_count(),
        rows,
        cols
    );
    KALI_CHECK(
        allow_scalar_conversion(scalar_type, type->scalar_type()),
        "'{}' expects a matrix with scalar type {} (no implicit conversion from type {})",
        m_type_layout->name(),
        type->scalar_type(),
        scalar_type
    );

    if (rows > 1) {
        // each row is aligned to 16 bytes
        size_t row_size = size / rows;
        ShaderOffset offset = m_offset;
        for (int row = 0; row < rows; ++row) {
            m_shader_object->set_data(offset, reinterpret_cast<const uint8_t*>(data) + row * row_size, row_size);
            offset.uniform_offset += 16;
        }
    } else {
        m_shader_object->set_data(m_offset, data, size);
    }
}

//
// Setter specializations
//

template<>
KALI_API void ShaderCursor::set(const ref<MutableShaderObject>& value) const
{
    set_object(value);
}

template<>
KALI_API void ShaderCursor::set(const ref<Buffer>& value) const
{
    set_buffer(value);
}

template<>
KALI_API void ShaderCursor::set(const ref<Texture>& value) const
{
    set_texture(value);
}

template<>
KALI_API void ShaderCursor::set(const ref<ResourceView>& value) const
{
    set_resource(value);
}

template<>
KALI_API void ShaderCursor::set(const ref<Sampler>& value) const
{
    set_sampler(value);
}

template<>
KALI_API void ShaderCursor::set(const ref<AccelerationStructure>& value) const
{
    set_acceleration_structure(value);
}

#define SET_SCALAR(type, scalar_type)                                                                                  \
    template<>                                                                                                         \
    KALI_API void ShaderCursor::set(const type& value) const                                                           \
    {                                                                                                                  \
        set_scalar(&value, sizeof(value), TypeReflection::ScalarType::scalar_type);                                    \
    }

#define SET_VECTOR(type, scalar_type)                                                                                  \
    template<>                                                                                                         \
    KALI_API void ShaderCursor::set(const type& value) const                                                           \
    {                                                                                                                  \
        set_vector(&value, sizeof(value), TypeReflection::ScalarType::scalar_type, type::dimension);                   \
    }

#define SET_MATRIX(type, scalar_type)                                                                                  \
    template<>                                                                                                         \
    KALI_API void ShaderCursor::set(const type& value) const                                                           \
    {                                                                                                                  \
        set_matrix(&value, sizeof(value), TypeReflection::ScalarType::scalar_type, type::rows, type::cols);            \
    }

SET_SCALAR(int, int32);
SET_VECTOR(int2, int32);
SET_VECTOR(int3, int32);
SET_VECTOR(int4, int32);

SET_SCALAR(uint, uint32);
SET_VECTOR(uint2, uint32);
SET_VECTOR(uint3, uint32);
SET_VECTOR(uint4, uint32);

SET_SCALAR(int64_t, int64);
SET_SCALAR(uint64_t, uint64);

SET_SCALAR(float16_t, float16);
SET_VECTOR(float16_t2, float16);
SET_VECTOR(float16_t3, float16);
SET_VECTOR(float16_t4, float16);

SET_SCALAR(float, float32);
SET_VECTOR(float2, float32);
SET_VECTOR(float3, float32);
SET_VECTOR(float4, float32);

SET_MATRIX(float2x2, float32);
SET_MATRIX(float3x3, float32);
SET_MATRIX(float2x4, float32);
SET_MATRIX(float3x4, float32);
SET_MATRIX(float4x4, float32);

SET_SCALAR(double, float64);

#undef SET_SCALAR
#undef SET_VECTOR
#undef SET_MATRIX

// Template specialization to allow setting booleans on a parameter block.
// On the host side a bool is 1B and the device 4B. We cast bools to 32-bit integers here.
// Note that this applies to our boolN vectors as well, which are currently 1B per element.

template<>
KALI_API void ShaderCursor::set(const bool& value) const
{
    uint v = value ? 1 : 0;
    set_scalar(&v, sizeof(v), TypeReflection::ScalarType::bool_);
}

template<>
KALI_API void ShaderCursor::set(const bool2& value) const
{
    uint2 v = {value.x ? 1 : 0, value.y ? 1 : 0};
    set_vector(&v, sizeof(v), TypeReflection::ScalarType::bool_, 2);
}

template<>
KALI_API void ShaderCursor::set(const bool3& value) const
{
    uint3 v = {value.x ? 1 : 0, value.y ? 1 : 0, value.z ? 1 : 0};
    set_vector(&v, sizeof(v), TypeReflection::ScalarType::bool_, 3);
}

template<>
KALI_API void ShaderCursor::set(const bool4& value) const
{
    uint4 v = {value.x ? 1 : 0, value.y ? 1 : 0, value.z ? 1 : 0, value.w ? 1 : 0};
    set_vector(&v, sizeof(v), TypeReflection::ScalarType::bool_, 4);
}

} // namespace kali
