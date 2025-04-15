#pragma once

#include "sgl/device/fwd.h"

#include "sgl/core/fwd.h"
#include "sgl/core/macros.h"
#include "sgl/core/object.h"
#include "sgl/core/enum.h"
#include "sgl/core/data_type.h"
#include "sgl/core/platform.h"

#include <vector>
#include <slang-rhi.h>

namespace sgl {

enum class CoopVecMatrixLayout {
    row_major = static_cast<uint32_t>(rhi::CooperativeVectorMatrixLayout::RowMajor),
    column_major = static_cast<uint32_t>(rhi::CooperativeVectorMatrixLayout::ColumnMajor),
    inferencing_optimal = static_cast<uint32_t>(rhi::CooperativeVectorMatrixLayout::InferencingOptimal),
    training_optimal = static_cast<uint32_t>(rhi::CooperativeVectorMatrixLayout::TrainingOptimal),
};

SGL_ENUM_INFO(
    CoopVecMatrixLayout,
    {
        {CoopVecMatrixLayout::row_major, "row_major"},
        {CoopVecMatrixLayout::column_major, "column_major"},
        {CoopVecMatrixLayout::inferencing_optimal, "inferencing_optimal"},
        {CoopVecMatrixLayout::training_optimal, "training_optimal"},
    }
);
SGL_ENUM_REGISTER(CoopVecMatrixLayout);

struct CoopVecMatrixDesc {
    uint32_t rows{0};
    uint32_t cols{0};
    DataType element_type{DataType::void_};
    CoopVecMatrixLayout layout{CoopVecMatrixLayout::row_major};
    size_t size{0};   // Size (in bytes) of the matrix
    size_t offset{0}; // Offset (in bytes) from start of buffer
};

class SGL_API CoopVec : public Object {
    SGL_OBJECT(CoopVec)
public:
    CoopVec(Device* device);

    static constexpr size_t MATRIX_ALIGNMENT = 64; ///< Minimum byte alignment according to spec.
    static constexpr size_t VECTOR_ALIGHMENT = 16; ///< Minimum byte alignment according to spec.

    size_t query_matrix_size(uint32_t rows, uint32_t cols, CoopVecMatrixLayout layout, DataType element_type);

    // Convenience function for building a matrix desc; calls query_matrix_size internally
    CoopVecMatrixDesc create_matrix_desc(
        uint32_t rows,
        uint32_t cols,
        CoopVecMatrixLayout layout,
        DataType element_type,
        size_t offset = 0
    );

    // Host-to-host conversion
    size_t convert_matrix_host(const void* src, CoopVecMatrixDesc src_desc, void* dst, CoopVecMatrixDesc dst_desc);
    // Device-to-device conversion of single matrix
    void convert_matrix_device(
        const Buffer* src,
        CoopVecMatrixDesc src_desc,
        const Buffer* dst,
        CoopVecMatrixDesc dst_desc,
        CommandEncoder* encoder = nullptr
    );
    // Device-to-device conversion of multiple matrices
    void convert_matrix_device(
        const Buffer* src,
        const std::vector<CoopVecMatrixDesc>& src_desc,
        const Buffer* dst,
        const std::vector<CoopVecMatrixDesc>& dst_desc,
        CommandEncoder* encoder = nullptr
    );
    void convert_matrix_device(
        const Buffer* src,
        const CoopVecMatrixDesc* src_desc,
        const Buffer* dst,
        const CoopVecMatrixDesc* dst_desc,
        uint32_t matrix_count,
        CommandEncoder* encoder = nullptr
    );

    size_t align_matrix_offset(size_t offset)
    {
        return MATRIX_ALIGNMENT * ((offset + MATRIX_ALIGNMENT - 1) / MATRIX_ALIGNMENT);
    }
    size_t align_vector_offset(size_t offset)
    {
        return VECTOR_ALIGHMENT * ((offset + VECTOR_ALIGHMENT - 1) / VECTOR_ALIGHMENT);
    }

private:
    Device* m_device;
};

} // namespace sgl
