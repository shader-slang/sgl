// SPDX-License-Identifier: Apache-2.0

#include "shader_cursor.h"

#include "sgl/device/shader_object.h"
#include "sgl/device/resource.h"
#include "sgl/device/cuda_interop.h"
#include "sgl/device/cursor_utils.h"

#include "sgl/core/error.h"

#include "sgl/math/vector_types.h"
#include "sgl/math/matrix_types.h"

// TODO: Decide if we want to disable / optimize type checks
// currently can represent 50% of the cost of writes in
// certain situations.
#define SGL_ENABLE_CURSOR_TYPE_CHECKS

namespace sgl {

ShaderCursor::ShaderCursor(ShaderObject* shader_object)
    : m_type_layout(shader_object->slang_element_type_layout())
    , m_shader_object(shader_object)
    , m_offset(ShaderOffset::zero())
{
    SGL_ASSERT(m_type_layout);
    SGL_ASSERT(m_shader_object);
    SGL_ASSERT(m_offset.is_valid());
}

std::string ShaderCursor::to_string() const
{
    return "ShaderCursor()";
}

bool ShaderCursor::is_reference() const
{
    switch ((TypeReflection::Kind)m_type_layout->getKind()) {
    case TypeReflection::Kind::constant_buffer:
    case TypeReflection::Kind::parameter_block:
        return true;
    default:
        return false;
    }
}

ShaderCursor ShaderCursor::dereference() const
{
    SGL_CHECK(is_valid(), "Invalid cursor");
    switch ((TypeReflection::Kind)m_type_layout->getKind()) {
    case TypeReflection::Kind::constant_buffer:
    case TypeReflection::Kind::parameter_block:
    {
        ShaderCursor d = ShaderCursor(m_shader_object->get_object(m_offset));
#if SGL_MACOS
        d.m_type_layout = m_shader_object->get_slang_session()->getTypeLayout(
                m_type_layout->getElementTypeLayout()->getType(),
                0,
                slang::LayoutRules::MetalArgumentBufferTier2);
#endif
        return d;
    }
    default:
        return {};
    }
}

//
// Navigation
//

ShaderCursor ShaderCursor::operator[](std::string_view name) const
{
    SGL_CHECK(is_valid(), "Invalid cursor");
    ShaderCursor result = find_field(name);
    SGL_CHECK(result.is_valid(), "Field \"{}\" not found.", name);
    return result;
}

ShaderCursor ShaderCursor::operator[](uint32_t index) const
{
    SGL_CHECK(is_valid(), "Invalid cursor");
    ShaderCursor result = find_element(index);
    SGL_CHECK(result.is_valid(), "Element {} not found.", index);
    return result;
}

ShaderCursor ShaderCursor::find_field(std::string_view name) const
{
    if (!is_valid())
        return *this;

    // If the cursor is valid, we want to consider the type of data
    // it is referencing.
    //
    switch ((TypeReflection::Kind)m_type_layout->getKind()) {
        // The easy/expected case is when the value has a structure type.
        //
    case TypeReflection::Kind::struct_: {
        // We start by looking up the index of a field matching `name`.
        //
        // If there is no such field, we have an error.
        //
        int32_t field_index = (int32_t)m_type_layout->findFieldIndexByName(name.data(), name.data() + name.size());
        if (field_index < 0)
            break;

        // Once we know the index of the field being referenced,
        // we create a cursor to point at the field, based on
        // the offset information already in this cursor, plus
        // offsets derived from the field's layout.
        //
        slang::VariableLayoutReflection* field_layout = m_type_layout->getFieldByIndex(field_index);
        ShaderCursor field_cursor;

        // The field cursor will point into the same parent object.
        //
        field_cursor.m_shader_object = m_shader_object;

        // The type being pointed to is the type of the field.
        //
        field_cursor.m_type_layout = field_layout->getTypeLayout();

        // The byte offset is the current offset plus the relative offset of the field.
        // The offset in binding ranges is computed similarly.
        //
        field_cursor.m_offset.uniform_offset
            = m_offset.uniform_offset + narrow_cast<uint32_t>(field_layout->getOffset());
        field_cursor.m_offset.binding_range_index = m_offset.binding_range_index
            + narrow_cast<int32_t>(m_type_layout->getFieldBindingRangeOffset(field_index));

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

    switch ((TypeReflection::Kind)m_type_layout->getKind()) {
    case TypeReflection::Kind::array: {
        ShaderCursor element_cursor;
        element_cursor.m_shader_object = m_shader_object;
        element_cursor.m_type_layout = m_type_layout->getElementTypeLayout();
        element_cursor.m_offset.uniform_offset = m_offset.uniform_offset
            + index
                * narrow_cast<uint32_t>(
                    m_type_layout->getElementStride(SlangParameterCategory::SLANG_PARAMETER_CATEGORY_UNIFORM)
                );
        element_cursor.m_offset.binding_range_index = m_offset.binding_range_index;
        element_cursor.m_offset.binding_array_index
            = m_offset.binding_array_index * narrow_cast<uint32_t>(m_type_layout->getElementCount()) + index;
        return element_cursor;
    } break;

    case TypeReflection::Kind::vector:
    case TypeReflection::Kind::matrix: {
        ShaderCursor field_cursor;
        field_cursor.m_shader_object = m_shader_object;
        field_cursor.m_type_layout = m_type_layout->getElementTypeLayout();
        field_cursor.m_offset.uniform_offset = m_offset.uniform_offset
            + narrow_cast<uint32_t>(m_type_layout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM)) * index;
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

inline bool is_parameter_block(slang::TypeReflection* type)
{
    return (TypeReflection::Kind)type->getKind() == TypeReflection::Kind::parameter_block;
}

inline bool is_resource_type(slang::TypeReflection* type)
{
    switch ((TypeReflection::Kind)type->getKind()) {
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

inline bool is_buffer_resource_type(slang::TypeReflection* type)
{
    switch ((TypeReflection::Kind)type->getKind()) {
    case TypeReflection::Kind::constant_buffer:
    case TypeReflection::Kind::resource: {
        auto shape = (TypeReflection::ResourceShape)(
            type->getResourceShape() & SlangResourceShape::SLANG_RESOURCE_BASE_SHAPE_MASK
        );
        switch (shape) {
        case TypeReflection::ResourceShape::texture_buffer:
        case TypeReflection::ResourceShape::structured_buffer:
        case TypeReflection::ResourceShape::byte_address_buffer:
            return true;
        default:
            return false;
        }
    }
    case TypeReflection::Kind::texture_buffer:
    case TypeReflection::Kind::shader_storage_buffer:
    case TypeReflection::Kind::parameter_block:
        return true;
    default:
        return false;
    }
}

inline bool is_texture_resource_type(slang::TypeReflection* type)
{
    switch ((TypeReflection::Kind)type->getKind()) {
    case TypeReflection::Kind::resource: {
        auto shape = (TypeReflection::ResourceShape)(
            type->getResourceShape() & SlangResourceShape::SLANG_RESOURCE_BASE_SHAPE_MASK
        );
        switch (shape) {
        case TypeReflection::ResourceShape::texture_1d:
        case TypeReflection::ResourceShape::texture_2d:
        case TypeReflection::ResourceShape::texture_3d:
        case TypeReflection::ResourceShape::texture_cube:
            return true;
        default:
            return false;
        }
        break;
    }
    default:
        return false;
    }
}

inline bool is_sampler_type(slang::TypeReflection* type)
{
    return (TypeReflection::Kind)type->getKind() == TypeReflection::Kind::sampler_state;
}

inline bool is_shader_resource_type(slang::TypeReflection* type)
{
    return (TypeReflection::ResourceAccess)type->getResourceAccess() == TypeReflection::ResourceAccess::read;
}

inline bool is_unordered_access_type(slang::TypeReflection* type)
{
    return (TypeReflection::ResourceAccess)type->getResourceAccess() == TypeReflection::ResourceAccess::read_write;
}

inline bool is_acceleration_structure_resource_type(slang::TypeReflection* type)
{
    return (TypeReflection::Kind)type->getKind() == TypeReflection::Kind::resource
        && (TypeReflection::ResourceShape)type->getResourceShape()
        == TypeReflection::ResourceShape::acceleration_structure;
}

void ShaderCursor::set_resource(const ref<ResourceView>& resource_view) const
{
    slang::TypeReflection* type = cursor_utils::unwrap_array(m_type_layout)->getType();

    SGL_CHECK(is_resource_type(type), "\"{}\" cannot bind a resource", m_type_layout->getName());

    if (resource_view) {
        if (is_shader_resource_type(type)) {
            SGL_CHECK(
                resource_view->type() == ResourceViewType::shader_resource,
                "\"{}\" expects a shader resource view",
                m_type_layout->getName()
            );
        } else if (is_unordered_access_type(type)) {
            SGL_CHECK(
                resource_view->type() == ResourceViewType::unordered_access,
                "\"{}\" expects an unordered access view",
                m_type_layout->getName()
            );
        } else {
            SGL_THROW("\"{}\" expects a valid resource view", m_type_layout->getName());
        }
    }

    m_shader_object->set_resource(m_offset, resource_view);
}

void ShaderCursor::set_buffer(const ref<Buffer>& buffer) const
{
    slang::TypeReflection* type = cursor_utils::unwrap_array(m_type_layout)->getType();

    SGL_CHECK(is_buffer_resource_type(type), "\"{}\" cannot bind a buffer", m_type_layout->getName());

    if (buffer) {
        if (is_shader_resource_type(type)) {
            set_resource(buffer->get_srv());
        } else if (is_unordered_access_type(type)) {
            set_resource(buffer->get_uav());
        } else {
            SGL_THROW("\"{}\" expects a valid buffer", m_type_layout->getName());
        }
    } else {
        set_resource(nullptr);
    }
}

void ShaderCursor::set_texture(const ref<Texture>& texture) const
{
    slang::TypeReflection* type = cursor_utils::unwrap_array(m_type_layout)->getType();

    SGL_CHECK(is_texture_resource_type(type), "\"{}\" cannot bind a texture", m_type_layout->getName());

    if (texture) {
        if (is_shader_resource_type(type)) {
            set_resource(texture->get_srv());
        } else if (is_unordered_access_type(type)) {
            set_resource(texture->get_uav());
        } else {
            SGL_THROW("\"{}\" expects a valid texture", m_type_layout->getName());
        }
    } else {
        set_resource(nullptr);
    }
}

void ShaderCursor::set_sampler(const ref<Sampler>& sampler) const
{
    slang::TypeReflection* type = cursor_utils::unwrap_array(m_type_layout)->getType();

    SGL_CHECK(is_sampler_type(type), "\"{}\" cannot bind a sampler", m_type_layout->getName());

    m_shader_object->set_sampler(m_offset, sampler);
}

void ShaderCursor::set_acceleration_structure(const ref<AccelerationStructure>& acceleration_structure) const
{
    slang::TypeReflection* type = m_type_layout->getType();

    SGL_CHECK(
        is_acceleration_structure_resource_type(type),
        "\"{}\" cannot bind an acceleration structure",
        m_type_layout->getName()
    );

    m_shader_object->set_acceleration_structure(m_offset, acceleration_structure);
}

void ShaderCursor::set_data(const void* data, size_t size) const
{
    if ((TypeReflection::ParameterCategory)m_type_layout->getParameterCategory()
        != TypeReflection::ParameterCategory::uniform)
        SGL_THROW("\"{}\" cannot bind data", m_type_layout->getName());
    m_shader_object->set_data(m_offset, data, size);
}

void ShaderCursor::set_object(const ref<MutableShaderObject>& object) const
{
    slang::TypeReflection* type = m_type_layout->getType();

    SGL_CHECK(is_parameter_block(type), "\"{}\" cannot bind an object", m_type_layout->getName());

    m_shader_object->set_object(m_offset, object);
}

void ShaderCursor::set_cuda_tensor_view(const cuda::TensorView& tensor_view) const
{
    slang::TypeReflection* type = cursor_utils::unwrap_array(m_type_layout)->getType();

    SGL_CHECK(is_buffer_resource_type(type), "\"{}\" cannot bind a CUDA tensor view", m_type_layout->getName());

    if (is_shader_resource_type(type)) {
        m_shader_object->set_cuda_tensor_view(m_offset, tensor_view, false);
    } else if (is_unordered_access_type(type)) {
        m_shader_object->set_cuda_tensor_view(m_offset, tensor_view, true);
    } else {
        SGL_THROW("\"{}\" expects a valid buffer", m_type_layout->getName());
    }
}

void ShaderCursor::_set_array(
    const void* data,
    size_t size,
    TypeReflection::ScalarType scalar_type,
    size_t element_count
) const
{
    slang::TypeReflection* element_type = cursor_utils::unwrap_array(m_type_layout)->getType();
    size_t element_size = cursor_utils::get_scalar_type_size((TypeReflection::ScalarType)element_type->getScalarType());

#ifdef SGL_ENABLE_CURSOR_TYPE_CHECKS
    cursor_utils::check_array(m_type_layout, size, scalar_type, element_count);
#else
    SGL_UNUSED(scalar_type);
    SGL_UNUSED(element_count);
#endif

    size_t stride = m_type_layout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
    if (element_size == stride) {
        m_shader_object->set_data(m_offset, data, size);
    } else {
        ShaderOffset offset = m_offset;
        for (size_t i = 0; i < element_count; ++i) {
            m_shader_object->set_data(offset, reinterpret_cast<const uint8_t*>(data) + i * element_size, element_size);
            offset.uniform_offset += narrow_cast<uint32_t>(stride);
        }
    }
}

void ShaderCursor::_set_array_unsafe(const void* data, size_t size, size_t element_count) const
{
    slang::TypeReflection* element_type = cursor_utils::unwrap_array(m_type_layout)->getType();
    size_t element_size = cursor_utils::get_scalar_type_size((TypeReflection::ScalarType)element_type->getScalarType());

    size_t stride = m_type_layout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
    if (element_size == stride) {
        m_shader_object->set_data(m_offset, data, size);
    } else {
        ShaderOffset offset = m_offset;
        for (size_t i = 0; i < element_count; ++i) {
            m_shader_object->set_data(offset, reinterpret_cast<const uint8_t*>(data) + i * element_size, element_size);
            offset.uniform_offset += narrow_cast<uint32_t>(stride);
        }
    }
}

void ShaderCursor::_set_scalar(const void* data, size_t size, TypeReflection::ScalarType scalar_type) const
{
#ifdef SGL_ENABLE_CURSOR_TYPE_CHECKS
    cursor_utils::check_scalar(m_type_layout, size, scalar_type);
#else
    SGL_UNUSED(scalar_type);
#endif
    m_shader_object->set_data(m_offset, data, size);
}

void ShaderCursor::_set_vector(const void* data, size_t size, TypeReflection::ScalarType scalar_type, int dimension)
    const
{
#ifdef SGL_ENABLE_CURSOR_TYPE_CHECKS
    cursor_utils::check_vector(m_type_layout, size, scalar_type, dimension);
#else
    SGL_UNUSED(scalar_type);
    SGL_UNUSED(dimension);
#endif
    m_shader_object->set_data(m_offset, data, size);
}

void ShaderCursor::_set_matrix(
    const void* data,
    size_t size,
    TypeReflection::ScalarType scalar_type,
    int rows,
    int cols
) const
{
#ifdef SGL_ENABLE_CURSOR_TYPE_CHECKS
    cursor_utils::check_matrix(m_type_layout, size, scalar_type, rows, cols);
#else
    SGL_UNUSED(scalar_type);
    SGL_UNUSED(cols);
#endif

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
SGL_API void ShaderCursor::set(const ref<MutableShaderObject>& value) const
{
    set_object(value);
}

template<>
SGL_API void ShaderCursor::set(const ref<Buffer>& value) const
{
    set_buffer(value);
}

template<>
SGL_API void ShaderCursor::set(const ref<Texture>& value) const
{
    set_texture(value);
}

template<>
SGL_API void ShaderCursor::set(const ref<ResourceView>& value) const
{
    set_resource(value);
}

template<>
SGL_API void ShaderCursor::set(const ref<Sampler>& value) const
{
    set_sampler(value);
}

template<>
SGL_API void ShaderCursor::set(const ref<AccelerationStructure>& value) const
{
    set_acceleration_structure(value);
}

#define SET_SCALAR(type, scalar_type)                                                                                  \
    template<>                                                                                                         \
    SGL_API void ShaderCursor::set(const type& value) const                                                            \
    {                                                                                                                  \
        _set_scalar(&value, sizeof(value), TypeReflection::ScalarType::scalar_type);                                   \
    }

#define SET_VECTOR(type, scalar_type)                                                                                  \
    template<>                                                                                                         \
    SGL_API void ShaderCursor::set(const type& value) const                                                            \
    {                                                                                                                  \
        _set_vector(&value, sizeof(value), TypeReflection::ScalarType::scalar_type, type::dimension);                  \
    }

#define SET_MATRIX(type, scalar_type)                                                                                  \
    template<>                                                                                                         \
    SGL_API void ShaderCursor::set(const type& value) const                                                            \
    {                                                                                                                  \
        _set_matrix(&value, sizeof(value), TypeReflection::ScalarType::scalar_type, type::rows, type::cols);           \
    }

SET_SCALAR(int8_t, int8);
SET_SCALAR(uint8_t, uint8);
SET_SCALAR(int16_t, int16);
SET_SCALAR(uint16_t, uint16);

SET_SCALAR(int, int32);
SET_VECTOR(int1, int32);
SET_VECTOR(int2, int32);
SET_VECTOR(int3, int32);
SET_VECTOR(int4, int32);

SET_SCALAR(uint, uint32);
SET_VECTOR(uint1, uint32);
SET_VECTOR(uint2, uint32);
SET_VECTOR(uint3, uint32);
SET_VECTOR(uint4, uint32);

// MACOS treats these as separate types to int/uint, so they need to be
// provided as extra overloads for linking to succeed.
#if SGL_MACOS
SET_SCALAR(long, int32);
SET_SCALAR(unsigned long, uint32);
#endif

SET_SCALAR(int64_t, int64);
SET_SCALAR(uint64_t, uint64);

SET_SCALAR(float16_t, float16);
SET_VECTOR(float16_t1, float16);
SET_VECTOR(float16_t2, float16);
SET_VECTOR(float16_t3, float16);
SET_VECTOR(float16_t4, float16);

SET_SCALAR(float, float32);
SET_VECTOR(float1, float32);
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
SGL_API void ShaderCursor::set(const bool& value) const
{
#if SGL_MACOS
    bool v = value;
#else
    uint v = value ? 1 : 0;
#endif
    _set_scalar(&v, sizeof(v), TypeReflection::ScalarType::bool_);
}

template<>
SGL_API void ShaderCursor::set(const bool1& value) const
{
#if SGL_MACOS
    bool1 v = value;
#else
    uint1 v(value.x ? 1 : 0);
#endif
    _set_vector(&v, sizeof(v), TypeReflection::ScalarType::bool_, 1);
}

template<>
SGL_API void ShaderCursor::set(const bool2& value) const
{
#if SGL_MACOS
    bool2 v = value;
#else
    uint2 v = {value.x ? 1 : 0, value.y ? 1 : 0};
#endif
    _set_vector(&v, sizeof(v), TypeReflection::ScalarType::bool_, 2);
}

template<>
SGL_API void ShaderCursor::set(const bool3& value) const
{
#if SGL_MACOS
    bool3 v = value;
#else
    uint3 v = {value.x ? 1 : 0, value.y ? 1 : 0, value.z ? 1 : 0};
#endif
    _set_vector(&v, sizeof(v), TypeReflection::ScalarType::bool_, 3);
}

template<>
SGL_API void ShaderCursor::set(const bool4& value) const
{
#if SGL_MACOS
    bool4 v = value;
#else
    uint4 v = {value.x ? 1 : 0, value.y ? 1 : 0, value.z ? 1 : 0, value.w ? 1 : 0};
#endif
    _set_vector(&v, sizeof(v), TypeReflection::ScalarType::bool_, 4);
}

} // namespace sgl
