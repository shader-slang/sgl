// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/coopvec.h"
#include "sgl/device/command.h"

#include "sgl/device/resource.h"

#include <iostream>

namespace sgl {

template<typename... Args>
static bool ndarray_is_row_maj(const nb::ndarray<Args...>& array)
{
    return array.ndim() == 2 && (array.stride(0) == int64_t(array.shape(1)) && array.stride(1) == 1);
}
template<typename... Args>
static bool ndarray_is_col_maj(const nb::ndarray<Args...>& array)
{
    return array.ndim() == 2 && (array.stride(0) == 1 && array.stride(1) == int64_t(array.shape(0)));
}

template<typename... Args>
static void check_ndarray(const nb::ndarray<Args...>& array)
{
    if (array.ndim() == 1)
        return; // 1D arrays are always ok
    SGL_CHECK(array.ndim() == 2, "Expected 2-dimensional array (received ndim = %d)", array.ndim());
    SGL_CHECK(ndarray_is_row_maj(array) || ndarray_is_col_maj(array), "2D arrays must be row or column major");
}
template<typename... Args>
static CoopVecMatrixLayout get_ndarray_layout(const nb::ndarray<Args...>& array, std::optional<CoopVecMatrixLayout> hint)
{
    if (hint)
        return hint.value();
    if (ndarray_is_row_maj(array))
        return CoopVecMatrixLayout::row_major;
    if (ndarray_is_col_maj(array))
        return CoopVecMatrixLayout::column_major;
    SGL_THROW("Require a 2D row major/column major array when no explicit matrix layout is given");
}
template<typename... Args>
static DataType get_ndarray_element_type(const nb::ndarray<Args...>& array)
{
    DataType result = dtype_to_data_type(array.dtype());
    SGL_CHECK(result != DataType::void_ && result != DataType::bool_, "Invalid coop_vec element type \"%d\"", result);
    return result;
}
template<typename... Args>
static CoopVecMatrixDesc get_ndarray_matrix_desc(const nb::ndarray<Args...>& array, std::optional<CoopVecMatrixLayout> layout_hint)
{
    CoopVecMatrixDesc desc;
    desc.size = array.nbytes();
    desc.layout = get_ndarray_layout(array, layout_hint);
    desc.element_type = get_ndarray_element_type(array);
    desc.rows = array.ndim() > 0 ? uint32_t(array.shape(0)) : 0;
    desc.cols = array.ndim() > 1 ? uint32_t(array.shape(1)) : 0;
    return desc;
}

static const char* __doc_sgl_CoopVec_query_matrix_size = R"doc()doc";
static const char* __doc_sgl_CoopVec_create_matrix_desc = R"doc()doc";
static const char* __doc_sgl_CoopVec_convert_matrix = R"doc()doc";
static const char* __doc_sgl_CoopVec_convert_matrix_device = R"doc()doc";
static const char* __doc_sgl_CoopVec_matrix_alignment = R"doc()doc";
static const char* __doc_sgl_CoopVec_vector_alignment = R"doc()doc";
static const char* __doc_sgl_CoopVec_align_matrix_offset = R"doc()doc";
static const char* __doc_sgl_CoopVec_align_vector_offset = R"doc()doc";

inline size_t convert_matrix_host_to_host(CoopVec* self,
        nb::ndarray<nb::device::cpu> src, nb::ndarray<nb::device::cpu> dst,
        std::optional<CoopVecMatrixLayout> src_layout, std::optional<CoopVecMatrixLayout> dst_layout)
{
    CoopVecMatrixDesc src_desc = get_ndarray_matrix_desc(src, src_layout);
    CoopVecMatrixDesc dst_desc = get_ndarray_matrix_desc(dst, dst_layout);

    if (src.ndim() == 2 && dst.ndim() == 2) {
        SGL_CHECK(
            src_desc.rows == dst_desc.rows && src_desc.cols == dst_desc.cols,
            "Array shapes of src and dst do not match ((%d, %d) != (%d, %d))",
            src_desc.rows, src_desc.cols, dst_desc.rows, dst_desc.cols
        );
    } else if (src.ndim() == 2) {
        dst_desc.rows = src_desc.rows;
        dst_desc.cols = src_desc.cols;
    } else if (dst.ndim() == 2) {
        src_desc.rows = dst_desc.rows;
        src_desc.cols = dst_desc.cols;
    } else {
        SGL_THROW("At least one of src or dst must be a 2D array");
    }

    return self->convert_matrix_host(src.data(), src_desc, dst.data(), dst_desc);
}

}

SGL_PY_EXPORT(device_coopvec)
{
    using namespace sgl;

    nb::class_<CoopVec> coop_vec(m, "CoopVec", D_NA(CoopVec));

    nb::sgl_enum<CoopVecMatrixLayout>(coop_vec, "MatrixLayout", D_NA(CoopVec, MatrixLayout));

    nb::class_<CoopVecMatrixDesc>(coop_vec, "MatrixDesc", D_NA(CoopVec, MatrixDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](CoopVecMatrixDesc* self, uint32_t rows, uint32_t cols, DataType element_type, CoopVecMatrixLayout layout, size_t size, size_t offset) {
                self->rows = rows;
                self->cols = cols;
                self->element_type = element_type;
                self->layout = layout;
                self->size = size;
                self->offset = offset;
            },
            "rows"_a,
            "cols"_a,
            "element_type"_a,
            "layout"_a,
            "size"_a,
            "offset"_a
        )
        .def_rw("rows", &CoopVecMatrixDesc::rows, D_NA(CoopVec, MatrixDesc, rows))
        .def_rw("cols", &CoopVecMatrixDesc::cols, D_NA(CoopVec, MatrixDesc, cols))
        .def_rw("element_type", &CoopVecMatrixDesc::element_type, D_NA(CoopVec, MatrixDesc, element_type))
        .def_rw("layout", &CoopVecMatrixDesc::layout, D_NA(CoopVec, MatrixDesc, layout))
        .def_rw("size", &CoopVecMatrixDesc::size, D_NA(CoopVec, MatrixDesc, size))
        .def_rw("offset", &CoopVecMatrixDesc::offset, D_NA(CoopVec, MatrixDesc, offset));

    coop_vec
        .def("query_matrix_size", &CoopVec::query_matrix_size, D(CoopVec, query_matrix_size))
        .def("create_matrix_desc", &CoopVec::create_matrix_desc, "rows"_a, "cols"_a, "layout"_a, "element_type"_a, "offset"_a = 0, D(CoopVec, create_matrix_desc))
        .def("convert_matrix_host", &convert_matrix_host_to_host, "src"_a, "dst"_a, "src_layout"_a = nb::none(), "dst_layout"_a = nb::none(), D(CoopVec, convert_matrix))
        .def("convert_matrix_device", static_cast<void (CoopVec::*)(const ref<Buffer>&, CoopVecMatrixDesc, const ref<Buffer>&, CoopVecMatrixDesc, CommandBuffer*)>(&CoopVec::convert_matrix_device), "src"_a, "src_desc"_a, "dst"_a, "dst_desc"_a, "cmd"_a = nullptr, D(CoopVec, convert_matrix_device))
        .def("convert_matrix_device", static_cast<void (CoopVec::*)(const ref<Buffer>&, const std::vector<CoopVecMatrixDesc>&, const ref<Buffer>&, const std::vector<CoopVecMatrixDesc>&, CommandBuffer*)>(&CoopVec::convert_matrix_device), "src"_a, "src_desc"_a, "dst"_a, "dst_desc"_a, "cmd"_a = nullptr, D(CoopVec, convert_matrix_device))
        .def("align_matrix_offset", &CoopVec::align_matrix_offset, "offset"_a, D(CoopVec, align_matrix_offset))
        .def("align_vector_offset", &CoopVec::align_vector_offset, "offset"_a, D(CoopVec, align_vector_offset))
        .def_ro_static("matrix_alignment", &CoopVec::k_matrix_alignment, D(CoopVec, matrix_alignment))
        .def_ro_static("vector_alignment", &CoopVec::k_vector_alignment, D(CoopVec, vector_alignment));
}
