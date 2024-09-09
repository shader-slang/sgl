// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/reflection.h"

#include "sgl/core/macros.h"

namespace sgl {

class SGL_API BaseCursor {
public:
    BaseCursor() = default;

    BaseCursor(ref<const TypeLayoutReflection> layout);

    ref<const TypeLayoutReflection> type_layout() const { return m_type_layout; }
    ref<const TypeReflection> type() const { return m_type_layout->type(); }

protected:
    ref<const TypeLayoutReflection> m_type_layout{nullptr};

    void check_array(size_t size, TypeReflection::ScalarType scalar_type, size_t element_count) const;
    void check_scalar(size_t size, TypeReflection::ScalarType scalar_type) const;
    void check_vector(size_t size, TypeReflection::ScalarType scalar_type, int dimension) const;
    void check_matrix(size_t size, TypeReflection::ScalarType scalar_type, int rows, int cols) const;
};

} // namespace sgl
