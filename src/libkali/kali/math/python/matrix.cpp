#include "nanobind.h"

#include "kali/math/matrix.h"
#include "kali/core/traits.h"

#include <array>
#include <type_traits>

namespace kali::math {

template<typename T>
void bind_matrix_type(nb::module_& m, const char* name)
{
    nb::class_<T> mat(m, name);

    auto constexpr row_count = T::row_count();
    auto constexpr col_count = T::col_count();
    using value_type = typename T::value_type;
    using row_type = typename T::row_type;

    mat.def(nb::init<>());

    // Initialization from array (to allow implicit conversion from python lists).
    mat.def(nb::init_implicit<std::array<value_type, row_count * col_count>>());

    mat.def("__getitem__", [](const T& self, int i) { return self[i]; });
    mat.def("__setitem__", [](T& self, int i, const row_type& v) { self[i] = v; });

    mat.def("__getitem__", [](const T& self, std::array<int, 2> ij) { return self[ij[0]][ij[1]]; });
    mat.def("__setitem__", [](T& self, std::array<int, 2> ij, const value_type& v) { self[ij[0]][ij[1]] = v; });

    mat.def("get_row", nb::overload_cast<int>(&T::get_row, nb::const_), "row"_a);
    mat.def("set_row", &T::set_row, "row"_a, "value"_a);
    mat.def("get_col", &T::get_col, "col"_a);
    mat.def("set_col", &T::set_col, "col"_a, "value"_a);

    auto to_string_ = [](const T& self) { return to_string(self); };
    mat.def("__repr__", to_string_);
    mat.def("__str__", to_string_);

    mat.def_static("zeros", &T::zeros);
    mat.def_static("identity", &T::identity);

    mat.def(nb::self == nb::self);
    mat.def(nb::self != nb::self);

    m.def("transpose", [](const T& m) { return transpose(m); });

    if constexpr (row_count == col_count) {
        m.def("determinant", [](const T& m) { return determinant(m); });
        m.def("inverse", [](const T& m) { return inverse(m); });
    }
}

inline void bind_matrix(nb::module_& m)
{
    bind_matrix_type<float2x2>(m, "float2x2");
    bind_matrix_type<float3x3>(m, "float3x3");
    bind_matrix_type<float1x4>(m, "float1x4");
    bind_matrix_type<float2x4>(m, "float2x4");
    bind_matrix_type<float3x4>(m, "float3x4");
    bind_matrix_type<float4x4>(m, "float4x4");

    m.def(
        "perspective",
        [](float fovy, float aspect, float z_near, float z_far) { return perspective(fovy, aspect, z_near, z_far); }
    );
    m.def(
        "ortho",
        [](float left, float right, float bottom, float top, float z_near, float z_far)
        { return ortho(left, right, bottom, top, z_near, z_far); }
    );
}

} // namespace kali::math

KALI_PY_EXPORT(math_matrix)
{
    nb::module_ math = m.attr("math");

    kali::math::bind_matrix(math);

    m.attr("float2x2") = math.attr("float2x2");
    m.attr("float3x3") = math.attr("float3x3");
    m.attr("float1x4") = math.attr("float1x4");
    m.attr("float2x4") = math.attr("float2x4");
    m.attr("float3x4") = math.attr("float3x4");
    m.attr("float4x4") = math.attr("float4x4");
}
