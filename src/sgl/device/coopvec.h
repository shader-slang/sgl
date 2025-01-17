#pragma once

#include "sgl/device/fwd.h"

#include "sgl/core/fwd.h"
#include "sgl/core/macros.h"
#include "sgl/core/object.h"
#include "sgl/core/enum.h"

#include "vk_coop_vector.h"

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

struct ConvertCoopVecMatrixOptions {
    uint32_t rows;
    uint32_t cols;
    size_t src_offset;
    size_t src_size;
    CoopVecMatrixLayout src_layout;
    size_t dest_offset;
    size_t dest_size;
    CoopVecMatrixLayout dest_layout;
};

class SGL_API CoopVec : public Object {
    SGL_OBJECT(CoopVec)
public:
    CoopVec(ref<Device> device);
    ~CoopVec();

    size_t query_matrix_size(uint32_t rows, uint32_t cols, const CoopVecMatrixLayout layout);

private:
    PFN_vkVoidFunction get_function(const char* name) const;
    size_t calc_stride(uint32_t rows, uint32_t cols, CoopVecMatrixLayout layout);
    VkCooperativeVectorMatrixLayoutNV get_vk_layout(CoopVecMatrixLayout layout);

    Device* m_device;

    void* m_vk_module{nullptr};
    VkDevice m_vk_device{nullptr};
    PFN_vkConvertCooperativeVectorMatrixNV m_VkConvertCooperativeVectorMatrixNV{nullptr};
    PFN_vkCmdConvertCooperativeVectorMatrixNV m_VkCmdConvertCooperativeVectorMatrixNV{nullptr};
};

} // namespace sgl
