#include "sgl/device/cursor_utils.h"
#include "sgl/device/base_cursor.h"

namespace sgl {

BaseCursor::BaseCursor(ref<const TypeLayoutReflection> layout)
    : m_type_layout(std::move(layout))
{
    SGL_ASSERT(m_type_layout);
}

void BaseCursor::check_array(size_t size, TypeReflection::ScalarType scalar_type, size_t element_count) const
{
    ref<const TypeReflection> type = m_type_layout->type();
    ref<const TypeReflection> element_type = m_type_layout->unwrap_array()->type();
    size_t element_size = get_scalar_type_size(element_type->scalar_type());

    SGL_CHECK(type->is_array(), "\"{}\" cannot bind an array", m_type_layout->name());
    SGL_CHECK(
        allow_scalar_conversion(scalar_type, element_type->scalar_type()),
        "\"{}\" expects scalar type {} (no implicit conversion from type {})",
        m_type_layout->name(),
        element_type->scalar_type(),
        scalar_type
    );
    SGL_CHECK(
        element_count <= type->element_count(),
        "\"{}\" expects an array with at most {} elements (got {})",
        m_type_layout->name(),
        type->element_count(),
        element_count
    );
    SGL_ASSERT(element_count * element_size == size);
}

void BaseCursor::check_scalar(size_t size, TypeReflection::ScalarType scalar_type) const
{
    ref<const TypeReflection> type = m_type_layout->unwrap_array()->type();

    SGL_CHECK(type->kind() == TypeReflection::Kind::scalar, "\"{}\" cannot bind a scalar value", m_type_layout->name());
    SGL_CHECK(
        allow_scalar_conversion(scalar_type, type->scalar_type()),
        "\"{}\" expects scalar type {} (no implicit conversion from type {})",
        m_type_layout->name(),
        type->scalar_type(),
        scalar_type
    );
    SGL_UNUSED(size);
}

void BaseCursor::check_vector(size_t size, TypeReflection::ScalarType scalar_type, int dimension) const
{
    ref<const TypeReflection> type = m_type_layout->unwrap_array()->type();

    SGL_CHECK(type->kind() == TypeReflection::Kind::vector, "\"{}\" cannot bind a vector value", m_type_layout->name());
    SGL_CHECK(
        type->col_count() == uint32_t(dimension),
        "\"{}\" expects a vector with dimension {} (got dimension {})",
        m_type_layout->name(),
        type->col_count(),
        dimension
    );
    SGL_CHECK(
        allow_scalar_conversion(scalar_type, type->scalar_type()),
        "\"{}\" expects a vector with scalar type {} (no implicit conversion from type {})",
        m_type_layout->name(),
        type->scalar_type(),
        scalar_type
    );
    SGL_UNUSED(size);
}

void BaseCursor::check_matrix(size_t size, TypeReflection::ScalarType scalar_type, int rows, int cols) const
{
    ref<const TypeReflection> type = m_type_layout->unwrap_array()->type();

    SGL_CHECK(type->kind() == TypeReflection::Kind::matrix, "\"{}\" cannot bind a matrix value", m_type_layout->name());
    SGL_CHECK(
        type->row_count() == uint32_t(rows) && type->col_count() == uint32_t(cols),
        "\"{}\" expects a matrix with dimension {}x{} (got dimension {}x{})",
        m_type_layout->name(),
        type->row_count(),
        type->col_count(),
        rows,
        cols
    );
    SGL_CHECK(
        allow_scalar_conversion(scalar_type, type->scalar_type()),
        "\"{}\" expects a matrix with scalar type {} (no implicit conversion from type {})",
        m_type_layout->name(),
        type->scalar_type(),
        scalar_type
    );
    SGL_UNUSED(size);
}


} // namespace sgl
