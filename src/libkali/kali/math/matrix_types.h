#pragma once

#include "kali/math/scalar_types.h"
#include "kali/math/vector_types.h"
#include "kali/core/error.h"

#include <limits>

namespace kali::math {

/**
 * Matrix type with row-major storage.
 *
 * The semantics are aligned with Slang:
 * - Row major storage
 * - Math operators are element-wise (e.g. +, -, *, /)
 * - Free standing functions for matrix operations (e.g. mul(), transpose(), etc.)
 *
 * @tparam T Scalar type
 * @tparam RowCount Number of rows (1-4)
 * @tparam ColCount Number of columns (1-4)
 */
template<typename T, int RowCount, int ColCount>
class matrix {
    static_assert(RowCount >= 1 && RowCount <= 4);
    static_assert(ColCount >= 1 && ColCount <= 4);

    // For now, we only support float, but want to have the type T visible
    static_assert(std::is_same_v<float, T>);

private:
    template<typename, int, int>
    friend class matrix;

public:
    using value_type = T;
    using RowType = vector<T, ColCount>;
    using ColType = vector<T, RowCount>;

    static constexpr int get_row_count() { return RowCount; }
    static constexpr int get_col_count() { return ColCount; }

    matrix()
        : matrix(Form::Identity)
    {
    }

    template<typename U>
    matrix(std::initializer_list<U> v)
    {
        T* f = &m_rows[0][0];
        for (auto it = v.begin(); it != v.end(); ++it, ++f)
            *f = static_cast<T>(*it);
    }

    matrix(const matrix&) = default;
    matrix(matrix&&) noexcept = default;

    /// Construct matrix from another matrix with different dimensions.
    /// In HLSL/Slang, destination matrix must be equal or smaller than source matrix.
    /// In Falcor, destination matrix can be larger than source matrix (initialized with identity).
    template<int R, int C>
    matrix(const matrix<T, R, C>& other)
        : matrix(Form::Identity)
    {
        for (int r = 0; r < std::min(RowCount, R); ++r) {
            std::memcpy(&m_rows[r], &other.m_rows[r], std::min(ColCount, C) * sizeof(T));
        }
    }

    matrix& operator=(const matrix&) = default;
    matrix& operator=(matrix&&) = default;

    template<int R, int C>
    matrix& operator=(const matrix<T, R, C>& other)
    {
        for (int r = 0; r < std::min(RowCount, R); ++r) {
            std::memcpy(&m_rows[r], &other.m_rows[r], std::min(ColCount, C) * sizeof(T));
        }

        return *this;
    }

    /// Zero matrix.
    [[nodiscard]] static matrix zeros() { return matrix(Form::Zeros); }

    /// Identity matrix.
    [[nodiscard]] static matrix identity() { return matrix(Form::Identity); }

    T* data() { return &m_rows[0][0]; }
    const T* data() const { return &m_rows[0][0]; }

    RowType& operator[](int r)
    {
        KALI_ASSERT_LT(r, RowCount);
        return m_rows[r];
    }
    const RowType& operator[](int r) const
    {
        KALI_ASSERT_LT(r, RowCount);
        return m_rows[r];
    }

    RowType& get_row(int r)
    {
        KALI_ASSERT_LT(r, RowCount);
        return m_rows[r];
    }
    const RowType& get_row(int r) const
    {
        KALI_ASSERT_LT(r, RowCount);
        return m_rows[r];
    }

    void set_row(int r, const RowType& v)
    {
        KALI_ASSERT_LT(r, RowCount);
        m_rows[r] = v;
    }

    ColType get_col(int col) const
    {
        KALI_ASSERT_LT(col, ColCount);
        ColType result;
        for (int r = 0; r < RowCount; ++r)
            result[r] = m_rows[r][col];
        return result;
    }

    void set_col(int col, const ColType& v)
    {
        KALI_ASSERT_LT(col, ColCount);
        for (int r = 0; r < RowCount; ++r)
            m_rows[r][col] = v[r];
    }

    bool operator==(const matrix& rhs) const { return std::memcmp(this, &rhs, sizeof(*this)) == 0; }
    bool operator!=(const matrix& rhs) const { return !(*this == rhs); }

private:
    enum class Form {
        Undefined,
        Zeros,
        Identity,
    };

    explicit matrix(Form form)
    {
        switch (form) {
        case Form::Undefined:
#ifdef _DEBUG
            for (int i = 0; i < RowCount; ++i)
                m_rows[i] = RowType(std::numeric_limits<T>::quiet_NaN());
#endif
            break;
        case Form::Zeros:
            std::memset(this, 0, sizeof(*this));
            break;
        case Form::Identity:
            std::memset(this, 0, sizeof(*this));
            for (int i = 0; i < std::min(RowCount, ColCount); ++i)
                m_rows[i][i] = T(1);
            break;
        }
    }

    RowType m_rows[RowCount];
};

using float2x2 = matrix<float, 2, 2>;

using float3x3 = matrix<float, 3, 3>;

using float1x4 = matrix<float, 1, 4>;
using float2x4 = matrix<float, 2, 4>;
using float3x4 = matrix<float, 3, 4>;
using float4x4 = matrix<float, 4, 4>;

} // namespace kali::math

namespace kali {

using float2x2 = math::float2x2;

using float3x3 = math::float3x3;

using float1x4 = math::float1x4;
using float2x4 = math::float2x4;
using float3x4 = math::float3x4;
using float4x4 = math::float4x4;

} // namespace kali
