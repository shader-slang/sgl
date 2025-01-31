#pragma once

#include "sgl/device/fwd.h"

#include "sgl/core/fwd.h"
#include "sgl/core/macros.h"
#include "sgl/core/object.h"
#include "sgl/core/enum.h"
#include "sgl/core/data_type.h"

#include <vector>
#include <vulkan/vulkan.h>

namespace sgl {

enum class CoopVecMatrixLayout {
    row_major,
    column_major,
    inferencing_optimal,
    training_optimal,
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
    CoopVec(ref<Device> device);
    ~CoopVec();

    static const size_t k_matrix_alignment = 64; ///< Minimum byte alignment according to spec.
    static const size_t k_vector_alignment = 16; ///< Minimum byte alignment according to spec.

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
        const ref<Buffer>& src,
        CoopVecMatrixDesc src_desc,
        const ref<Buffer>& dst,
        CoopVecMatrixDesc dst_desc,
        CommandBuffer* cmd = nullptr
    );
    // Device-to-device conversion of multiple matrices
    void convert_matrix_device(
        const ref<Buffer>& src,
        const std::vector<CoopVecMatrixDesc>& src_desc,
        const ref<Buffer>& dst,
        const std::vector<CoopVecMatrixDesc>& dst_desc,
        CommandBuffer* cmd = nullptr
    );
    void convert_matrix_device(
        const ref<Buffer>& src,
        const CoopVecMatrixDesc* src_desc,
        const ref<Buffer>& dst,
        const CoopVecMatrixDesc* dst_desc,
        uint32_t matrix_count,
        CommandBuffer* cmd = nullptr
    );

    size_t align_matrix_offset(size_t offset)
    {
        return k_matrix_alignment * ((offset + k_matrix_alignment - 1) / k_matrix_alignment);
    }
    size_t align_vector_offset(size_t offset)
    {
        return k_vector_alignment * ((offset + k_vector_alignment - 1) / k_vector_alignment);
    }

private:
    PFN_vkVoidFunction get_function(const char* name) const;

    Device* m_device;

    void* m_vk_module{nullptr};
    VkDevice m_vk_device{nullptr};
    PFN_vkConvertCooperativeVectorMatrixNV m_VkConvertCooperativeVectorMatrixNV{nullptr};
    PFN_vkCmdConvertCooperativeVectorMatrixNV m_VkCmdConvertCooperativeVectorMatrixNV{nullptr};
};

} // namespace sgl
