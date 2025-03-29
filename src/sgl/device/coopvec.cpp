#include "coopvec.h"

#include "sgl/device/device.h"
#include "sgl/device/native_handle_traits.h"
#include "sgl/device/command.h"
#include "sgl/device/helpers.h"

#include <vector>

namespace sgl {

CoopVec::CoopVec(Device* device)
    : m_device(device)
{
    SGL_CHECK(m_device->has_feature("cooperative-vector"), "Device does not support cooperative vectors.");
}

static uint32_t calc_element_stride(uint32_t rows, uint32_t cols, CoopVecMatrixLayout layout)
{
    if (layout == CoopVecMatrixLayout::row_major)
        return cols;
    else if (layout == CoopVecMatrixLayout::column_major)
        return rows;
    return 0ull;
};

static size_t get_element_size(DataType dtype)
{
    switch (dtype) {
    case DataType::int8:
    case DataType::uint8:
        return 1;
    case DataType::float16:
    case DataType::int16:
    case DataType::uint16:
        return 2;
    case DataType::float32:
    case DataType::int32:
    case DataType::uint32:
        return 4;
    case DataType::float64:
    case DataType::uint64:
        return 8;
    default:
        SGL_THROW("\"%s\" is not a valid component type for cooperative vector matrix", dtype);
    }
}

static rhi::CooperativeVectorComponentType get_rhi_component_type(DataType dtype)
{
    switch (dtype) {
    case DataType::float16:
        return rhi::CooperativeVectorComponentType::Float16;
    case DataType::float32:
        return rhi::CooperativeVectorComponentType::Float32;
    case DataType::float64:
        return rhi::CooperativeVectorComponentType::Float64;
    case DataType::int8:
        return rhi::CooperativeVectorComponentType::Sint8;
    case DataType::int16:
        return rhi::CooperativeVectorComponentType::Sint16;
    case DataType::int32:
        return rhi::CooperativeVectorComponentType::Sint32;
    case DataType::uint8:
        return rhi::CooperativeVectorComponentType::Uint8;
    case DataType::uint16:
        return rhi::CooperativeVectorComponentType::Uint16;
    case DataType::uint32:
        return rhi::CooperativeVectorComponentType::Uint32;
    case DataType::uint64:
        return rhi::CooperativeVectorComponentType::Uint64;
    default:
        SGL_THROW("\"%s\" is not a valid component type for cooperative vector matrix", dtype);
    }
}

size_t CoopVec::query_matrix_size(uint32_t rows, uint32_t cols, CoopVecMatrixLayout layout, DataType element_type)
{
    SGL_CHECK(rows > 0 && rows <= 128, "Number of rows must be 1..128.");
    SGL_CHECK(cols > 0 && cols <= 128, "Number of columns must be 1..128.");

    size_t required_size = 0;

    rhi::ConvertCooperativeVectorMatrixDesc desc = {};
    desc.srcSize = 0;
    desc.srcData.hostAddress = nullptr;
    desc.dstSize = &required_size;
    desc.dstData.hostAddress = nullptr;
    desc.srcComponentType = get_rhi_component_type(element_type);
    desc.dstComponentType = get_rhi_component_type(element_type);
    desc.rowCount = rows;
    desc.colCount = cols;
    desc.srcLayout = static_cast<rhi::CooperativeVectorMatrixLayout>(layout);
    desc.srcStride = calc_element_stride(rows, cols, layout) * get_element_size(element_type);
    desc.dstLayout = static_cast<rhi::CooperativeVectorMatrixLayout>(layout);
    desc.dstStride = calc_element_stride(rows, cols, layout) * get_element_size(element_type);

    SLANG_CALL(m_device->rhi_device()->convertCooperativeVectorMatrix(&desc, 1));
    SGL_CHECK(required_size > 0, "Expected matrix size to be larger than zero.");

    return required_size;
}

CoopVecMatrixDesc CoopVec::create_matrix_desc(
    uint32_t rows,
    uint32_t cols,
    CoopVecMatrixLayout layout,
    DataType element_type,
    size_t offset
)
{
    SGL_CHECK(
        (offset % MATRIX_ALIGNMENT) == 0,
        "Matrix offset %d does not conform to required matrix alignment of %d",
        offset,
        MATRIX_ALIGNMENT
    );
    CoopVecMatrixDesc result;
    result.rows = rows;
    result.cols = cols;
    result.layout = layout;
    result.element_type = element_type;
    result.size = query_matrix_size(rows, cols, layout, element_type);
    result.offset = offset;
    return result;
}

static rhi::ConvertCooperativeVectorMatrixDesc build_rhi_matrix_desc(
    rhi::DeviceOrHostAddressConst src,
    CoopVecMatrixDesc src_desc,
    rhi::DeviceOrHostAddress dst,
    CoopVecMatrixDesc dst_desc,
    size_t* dst_size
)
{
    SGL_ASSERT(dst_size);
    SGL_CHECK(
        src_desc.rows == dst_desc.rows && src_desc.cols == dst_desc.cols,
        "Source and destination shapes don't match ((%d, %d) != (%d, %d))",
        src_desc.rows,
        src_desc.cols,
        dst_desc.rows,
        dst_desc.cols
    );
    SGL_CHECK(src_desc.rows > 0 && src_desc.rows <= 128, "Number of rows must be 1..128.");
    SGL_CHECK(src_desc.cols > 0 && src_desc.cols <= 128, "Number of columns must be 1..128.");

    rhi::ConvertCooperativeVectorMatrixDesc desc = {};
    desc.srcSize = src_desc.size;
    desc.srcData = src;
    desc.dstData = dst;
    *dst_size = dst_desc.size;
    desc.dstSize = dst_size;
    desc.srcComponentType = get_rhi_component_type(src_desc.element_type);
    desc.dstComponentType = get_rhi_component_type(dst_desc.element_type);
    desc.rowCount = src_desc.rows;
    desc.colCount = src_desc.cols;
    desc.srcLayout = static_cast<rhi::CooperativeVectorMatrixLayout>(src_desc.layout);
    // Bytes between a consecutive row or column (if row/column-major layout).
    desc.srcStride
        = calc_element_stride(src_desc.rows, src_desc.cols, src_desc.layout) * get_element_size(src_desc.element_type);
    desc.dstLayout = static_cast<rhi::CooperativeVectorMatrixLayout>(dst_desc.layout);
    // Bytes between a consecutive row or column (if row/column-major layout).
    desc.dstStride
        = calc_element_stride(dst_desc.rows, dst_desc.cols, dst_desc.layout) * get_element_size(dst_desc.element_type);

    return desc;
}

size_t CoopVec::convert_matrix_host(const void* src, CoopVecMatrixDesc src_desc, void* dst, CoopVecMatrixDesc dst_desc)
{
    rhi::DeviceOrHostAddressConst rhi_src;
    rhi_src.hostAddress = reinterpret_cast<const uint8_t*>(src) + src_desc.offset;
    rhi::DeviceOrHostAddress rhi_dst;
    rhi_dst.hostAddress = reinterpret_cast<uint8_t*>(dst) + dst_desc.offset;

    size_t actual_size;
    rhi::ConvertCooperativeVectorMatrixDesc desc
        = build_rhi_matrix_desc(rhi_src, src_desc, rhi_dst, dst_desc, &actual_size);

    SLANG_CALL(m_device->rhi_device()->convertCooperativeVectorMatrix(&desc, 1));
    SGL_CHECK(actual_size > 0, "Expected matrix size to be larger than zero.");

    return actual_size;
}

void CoopVec::convert_matrix_device(
    const Buffer* src,
    CoopVecMatrixDesc src_desc,
    const Buffer* dst,
    CoopVecMatrixDesc dst_desc,
    CommandEncoder* encoder
)
{
    convert_matrix_device(src, &src_desc, dst, &dst_desc, 1, encoder);
}

void CoopVec::convert_matrix_device(
    const Buffer* src,
    const std::vector<CoopVecMatrixDesc>& src_desc,
    const Buffer* dst,
    const std::vector<CoopVecMatrixDesc>& dst_desc,
    CommandEncoder* encoder
)
{
    SGL_CHECK(
        src_desc.size() == dst_desc.size(),
        "Number of source and destination matrices must match (%d != %d)",
        src_desc.size(),
        dst_desc.size()
    );

    convert_matrix_device(src, &src_desc[0], dst, &dst_desc[0], uint32_t(src_desc.size()), encoder);
}

void CoopVec::convert_matrix_device(
    const Buffer* src,
    const CoopVecMatrixDesc* src_desc,
    const Buffer* dst,
    const CoopVecMatrixDesc* dst_desc,
    uint32_t matrix_count,
    CommandEncoder* encoder
)
{
    SGL_CHECK(matrix_count > 0, "Matrix count must be 1 or more.");

    size_t actual_size;
    std::vector<rhi::ConvertCooperativeVectorMatrixDesc> descs(matrix_count);
    for (size_t i = 0; i < matrix_count; i++) {
        SGL_CHECK(
            dst->size() >= dst_desc[i].offset + dst_desc[i].size,
            "Destination buffer is too small (offset %d + matrix size %d > buffer size %d)",
            dst_desc[i].offset,
            dst_desc[i].size,
            dst->size()
        );
        SGL_CHECK(
            src->size() >= src_desc[i].offset + src_desc[i].size,
            "Matrix size exceeds size of source buffer (offset %d + matrix size %d > buffer size %d)",
            src_desc[i].offset,
            src_desc[i].size,
            src->size()
        );

        rhi::DeviceOrHostAddressConst rhi_src;
        rhi_src.deviceAddress = src->device_address() + src_desc[i].offset;
        rhi::DeviceOrHostAddress rhi_dst;
        rhi_dst.deviceAddress = dst->device_address() + dst_desc[i].offset;

        descs[i] = build_rhi_matrix_desc(rhi_src, src_desc[i], rhi_dst, dst_desc[i], &actual_size);
    }

    ref<CommandEncoder> temp_encoder;
    if (encoder == nullptr) {
        temp_encoder = m_device->create_command_encoder();
        encoder = temp_encoder.get();
    }

    // TODO: The API defines a new pipeline stage bit VK_PIPELINE_STAGE_2_CONVERT_COOPERATIVE_VECTOR_MATRIX_BIT_NV
    // that can be used for synchronization with VK_ACCESS_2_TRANSFER_READ_BIT / VK_ACCESS_2_TRANSFER_WRITE_BIT.
    // Slang GFX doesn't expose this yet, so we use regular ShaderResource / UnorderedAccess states for now.

    // Insert barriers to transition source to ShaderResource, and explicit UAV barrier for destination.
    encoder->rhi_command_encoder()->setBufferState(src->rhi_buffer(), rhi::ResourceState::ShaderResource);
    encoder->rhi_command_encoder()->setBufferState(dst->rhi_buffer(), rhi::ResourceState::UnorderedAccess);

    encoder->rhi_command_encoder()->convertCooperativeVectorMatrix(descs.data(), matrix_count);

    // Insert barriers to transition destination to ShaderResource.
    // After this point it should be safe to access.
    encoder->rhi_command_encoder()->setBufferState(dst->rhi_buffer(), rhi::ResourceState::ShaderResource);

    if (temp_encoder)
        m_device->submit_command_buffer(temp_encoder->finish());
}

} // namespace sgl
