#include "coopvec.h"

#include "sgl/device/device.h"
#include "sgl/device/native_handle_traits.h"
#include "sgl/device/command.h"
#include "sgl/device/helpers.h"

#include <vector>

namespace sgl {

CoopVec::CoopVec(ref<Device> device)
    : m_device(device.get())
{
    SGL_CHECK(device->type() == DeviceType::vulkan, "Requires a Vulkan device");
    bool have_coop_vec = false;
    for (const auto& f : device->features())
        have_coop_vec = have_coop_vec || f == "cooperative-vector";
    SGL_CHECK(have_coop_vec, "Requires a Vulkan device with coop_vector support.");

#if SGL_WINDOWS
    const char* dll_name = "vulkan-1.dll";
#elif SGL_LINUX
    const char* dll_name = "libvulkan.so.1";
#else
    SGL_THROW("Platform does not support CoopVec");
    return;
#endif

    m_vk_module = platform::load_shared_library(dll_name);
    SGL_CHECK(m_vk_module != nullptr, "Failed to load Vulkan module '{}'.", dll_name);

    auto vk_instance = device->get_native_handle(0).as<VkInstance>();
    SGL_CHECK(vk_instance != VK_NULL_HANDLE, "Failed to get Vulkan instance handle from GFX.");

    m_vk_device = device->get_native_handle(2).as<VkDevice>();
    SGL_CHECK(m_vk_device != VK_NULL_HANDLE, "Failed to get Vulkan device handle from GFX.");

    auto vkGetInstanceProcAddr
        = (PFN_vkGetInstanceProcAddr)platform::get_proc_address(m_vk_module, "vkGetInstanceProcAddr");
    SGL_CHECK(vkGetInstanceProcAddr != nullptr, "Failed to get Vulkan function 'vkGetInstanceProcAddr'.");

    auto vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(vk_instance, "vkGetDeviceProcAddr");
    SGL_CHECK(vkGetDeviceProcAddr != nullptr, "Failed to get Vulkan function 'vkGetDeviceProcAddr'.");

    m_VkConvertCooperativeVectorMatrixNV = (PFN_vkConvertCooperativeVectorMatrixNV
    )vkGetDeviceProcAddr(m_vk_device, "vkConvertCooperativeVectorMatrixNV");
    SGL_CHECK(
        m_VkConvertCooperativeVectorMatrixNV != nullptr,
        "Failed to get Vulkan function 'vkConvertCooperativeVectorMatrixNV'."
    );

    m_VkCmdConvertCooperativeVectorMatrixNV = (PFN_vkCmdConvertCooperativeVectorMatrixNV
    )vkGetDeviceProcAddr(m_vk_device, "vkCmdConvertCooperativeVectorMatrixNV");
    SGL_CHECK(
        m_VkCmdConvertCooperativeVectorMatrixNV != nullptr,
        "Failed to get Vulkan function 'vkCmdConvertCooperativeVectorMatrixNV'."
    );
}

CoopVec::~CoopVec()
{
    if (m_vk_module != nullptr) {
        platform::release_shared_library(m_vk_module);
        m_vk_module = nullptr;
    }
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
        SGL_THROW("\"%d\" is not a valid component type of a coop_vector matrix", dtype);
    }
}

static VkCooperativeVectorMatrixLayoutNV get_vk_layout(CoopVecMatrixLayout layout)
{
    switch (layout) {
    case CoopVecMatrixLayout::row_major:
        return VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV;
    case CoopVecMatrixLayout::column_major:
        return VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_COLUMN_MAJOR_NV;
    case CoopVecMatrixLayout::inferencing_optimal:
        return VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_INFERENCING_OPTIMAL_NV;
    case CoopVecMatrixLayout::training_optimal:
        return VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_TRAINING_OPTIMAL_NV;
    default:
        SGL_UNREACHABLE();
    }
}

static VkComponentTypeKHR get_vk_component_type(DataType dtype)
{
    switch (dtype) {
    case DataType::float16:
        return VK_COMPONENT_TYPE_FLOAT16_NV;
    case DataType::float32:
        return VK_COMPONENT_TYPE_FLOAT32_NV;
    case DataType::float64:
        return VK_COMPONENT_TYPE_FLOAT64_NV;
    case DataType::int8:
        return VK_COMPONENT_TYPE_SINT8_NV;
    case DataType::int16:
        return VK_COMPONENT_TYPE_SINT16_NV;
    case DataType::int32:
        return VK_COMPONENT_TYPE_SINT32_NV;
    case DataType::uint8:
        return VK_COMPONENT_TYPE_UINT8_NV;
    case DataType::uint16:
        return VK_COMPONENT_TYPE_UINT16_NV;
    case DataType::uint32:
        return VK_COMPONENT_TYPE_UINT32_NV;
    case DataType::uint64:
        return VK_COMPONENT_TYPE_UINT64_NV;
    default:
        SGL_THROW("\"%d\" is not a valid component of a coopvector matrix", dtype);
    }
}

size_t CoopVec::query_matrix_size(uint32_t rows, uint32_t cols, CoopVecMatrixLayout layout, DataType element_type)
{
    SGL_ASSERT(m_vk_device);
    SGL_ASSERT(m_VkConvertCooperativeVectorMatrixNV);
    SGL_CHECK(rows > 0 && rows <= 128, "Number of rows must be 1..128.");
    SGL_CHECK(cols > 0 && cols <= 128, "Number of columns must be 1..128.");

    size_t required_size = 0;

    VkConvertCooperativeVectorMatrixInfoNV info = {};
    info.sType = VK_STRUCTURE_TYPE_CONVERT_COOPERATIVE_VECTOR_MATRIX_INFO_NV;
    info.numRows = rows;
    info.numColumns = cols;
    info.srcComponentType = get_vk_component_type(element_type);
    info.srcLayout = get_vk_layout(layout);
    info.srcStride = calc_element_stride(rows, cols, layout) * get_element_size(element_type);
    info.srcSize = 0;
    info.srcData.hostAddress = nullptr;
    info.dstComponentType = get_vk_component_type(element_type);
    info.dstLayout = get_vk_layout(layout);
    info.dstStride = calc_element_stride(rows, cols, layout) * get_element_size(element_type);
    info.pDstSize = &required_size;
    info.dstData.hostAddress = nullptr;

    VkResult res = m_VkConvertCooperativeVectorMatrixNV(m_vk_device, &info);
    SGL_CHECK(
        res == VK_SUCCESS,
        "Call to vkConvertCooperativeVectorMatrixNV failed with return code ({}).",
        (uint32_t)res
    );
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

static VkConvertCooperativeVectorMatrixInfoNV build_vk_matrix_info(
    VkDeviceOrHostAddressConstKHR src,
    CoopVecMatrixDesc src_desc,
    VkDeviceOrHostAddressKHR dst,
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

    VkConvertCooperativeVectorMatrixInfoNV info = {};
    info.sType = VK_STRUCTURE_TYPE_CONVERT_COOPERATIVE_VECTOR_MATRIX_INFO_NV;
    info.numRows = src_desc.rows;
    info.numColumns = src_desc.cols;
    info.srcComponentType = get_vk_component_type(src_desc.element_type);
    info.srcLayout = get_vk_layout(src_desc.layout);
    // Bytes between a consecutive row or column (if row/column-major layout).
    info.srcStride
        = calc_element_stride(src_desc.rows, src_desc.cols, src_desc.layout) * get_element_size(src_desc.element_type);
    info.srcSize = src_desc.size;
    info.srcData = src;

    info.dstComponentType = get_vk_component_type(dst_desc.element_type);
    info.dstLayout = get_vk_layout(dst_desc.layout);
    // Bytes between a consecutive row or column (if row/column-major layout).
    info.dstStride
        = calc_element_stride(dst_desc.rows, dst_desc.cols, dst_desc.layout) * get_element_size(dst_desc.element_type);
    *dst_size = dst_desc.size;
    info.pDstSize = dst_size;
    info.dstData = dst;

    return info;
}

size_t CoopVec::convert_matrix_host(const void* src, CoopVecMatrixDesc src_desc, void* dst, CoopVecMatrixDesc dst_desc)
{
    SGL_ASSERT(m_vk_device);
    SGL_ASSERT(m_VkConvertCooperativeVectorMatrixNV);

    VkDeviceOrHostAddressConstKHR vk_src;
    vk_src.hostAddress = reinterpret_cast<const uint8_t*>(src) + src_desc.offset;
    VkDeviceOrHostAddressKHR vk_dst;
    vk_dst.hostAddress = reinterpret_cast<uint8_t*>(dst) + dst_desc.offset;

    size_t actual_size;
    VkConvertCooperativeVectorMatrixInfoNV info
        = build_vk_matrix_info(vk_src, src_desc, vk_dst, dst_desc, &actual_size);

    VkResult res = m_VkConvertCooperativeVectorMatrixNV(m_vk_device, &info);

    SGL_CHECK(
        res == VK_SUCCESS,
        "Call to vkConvertCooperativeVectorMatrixNV failed with return code ({}).",
        (uint32_t)res
    );
    SGL_CHECK(actual_size > 0, "Expected matrix size to be larger than zero.");

    return actual_size;
}

void CoopVec::convert_matrix_device(
    const ref<Buffer>& src,
    CoopVecMatrixDesc src_desc,
    const ref<Buffer>& dst,
    CoopVecMatrixDesc dst_desc,
    CommandBuffer* cmd
)
{
    convert_matrix_device(src, &src_desc, dst, &dst_desc, 1, cmd);
}

void CoopVec::convert_matrix_device(
    const ref<Buffer>& src,
    const std::vector<CoopVecMatrixDesc>& src_desc,
    const ref<Buffer>& dst,
    const std::vector<CoopVecMatrixDesc>& dst_desc,
    CommandBuffer* cmd
)
{
    SGL_CHECK(
        src_desc.size() == dst_desc.size(),
        "Number of source and destination matrices must match (%d != %d)",
        src_desc.size(),
        dst_desc.size()
    );

    convert_matrix_device(src, &src_desc[0], dst, &dst_desc[0], uint32_t(src_desc.size()), cmd);
}

void CoopVec::convert_matrix_device(
    const ref<Buffer>& src,
    const CoopVecMatrixDesc* src_desc,
    const ref<Buffer>& dst,
    const CoopVecMatrixDesc* dst_desc,
    uint32_t matrix_count,
    CommandBuffer* cmd
)
{
    SGL_ASSERT(m_vk_device);
    SGL_ASSERT(m_VkCmdConvertCooperativeVectorMatrixNV);
    SGL_CHECK(matrix_count > 0, "Matrix count must be 1 or more.");

    size_t actual_size;
    std::vector<VkConvertCooperativeVectorMatrixInfoNV> infos(matrix_count);
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

        VkDeviceOrHostAddressConstKHR vk_src;
        vk_src.deviceAddress = src->device_address() + src_desc[i].offset;
        VkDeviceOrHostAddressKHR vk_dst;
        vk_dst.deviceAddress = dst->device_address() + dst_desc[i].offset;

        infos[i] = build_vk_matrix_info(vk_src, src_desc[i], vk_dst, dst_desc[i], &actual_size);
    }

    CommandBuffer* actual_cmd = cmd;
    if (cmd == nullptr)
        actual_cmd = m_device->_begin_shared_command_buffer();

    // TODO: The API defines a new pipeline stage bit VK_PIPELINE_STAGE_2_CONVERT_COOPERATIVE_VECTOR_MATRIX_BIT_NV
    // that can be used for synchronization with VK_ACCESS_2_TRANSFER_READ_BIT / VK_ACCESS_2_TRANSFER_WRITE_BIT.
    // Slang GFX doesn't expose this yet, so we use regular ShaderResource / UnorderedAccess states for now.

    // Insert barriers to transition source to ShaderResource, and explicit UAV barrier for destination.
    actual_cmd->set_resource_state(src.get(), ResourceState::shader_resource);
    actual_cmd->uav_barrier(dst.get());

    gfx::InteropHandle handle = {};
    SLANG_CALL(actual_cmd->gfx_command_buffer()->getNativeHandle(&handle));

    VkCommandBuffer vkCommandBuffer = reinterpret_cast<VkCommandBuffer>(handle.handleValue);

    m_VkCmdConvertCooperativeVectorMatrixNV(vkCommandBuffer, matrix_count, infos.data());

    // Insert barriers to transition destination to ShaderResource.
    // After this point it should be safe to access.
    actual_cmd->set_resource_state(dst.get(), ResourceState::shader_resource);

    if (cmd == nullptr)
        m_device->_end_shared_command_buffer(false);
}

} // namespace sgl
