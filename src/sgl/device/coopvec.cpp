#include "coopvec.h"

#include "sgl/device/device.h"
#include "sgl/device/native_handle_traits.h"

#if SGL_WINDOWS
#include <windows.h>
#elif SGL_LINUX
#include <dlfcn.h>
#endif

namespace sgl {


CoopVec::CoopVec(ref<Device> device)
    : m_device(device.get())
{
    if (device->type() != DeviceType::vulkan)
        return;

        //        || !pDevice->isFeatureSupported(Device::SupportedFeatures::CoopVector))
        //        FALCOR_THROW("Requires a Vulkan device with coop_vector support.");

#if SGL_WINDOWS
    const char* dll_name = "vulkan-1.dll";
    m_vk_module = (void*)::LoadLibraryA(dll_name);
    SGL_CHECK(m_vk_module != nullptr, "Failed to load Vulkan module '{}'.", dll_name);
#elif SGL_LINUX
    const char* dll_name = "libvulkan.so.1";
    m_vk_module = dlopen(dll_name, RTLD_NOW);
#else
    return;
#endif

    auto vk_instance = device->get_native_handle(0).as<VkInstance>();
    SGL_CHECK(vk_instance != VK_NULL_HANDLE, "Failed to get Vulkan instance handle from GFX.");

    m_vk_device = device->get_native_handle(2).as<VkDevice>();
    SGL_CHECK(m_vk_device != VK_NULL_HANDLE, "Failed to get Vulkan device handle from GFX.");

    auto vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)get_function("vkGetInstanceProcAddr");
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
#if SGL_WINDOWS
        ::FreeLibrary((HMODULE)m_vk_module);
#elif SGL_LINUX
        dlclose(m_vk_module);
#endif
        m_vk_module = nullptr;
    }
}

PFN_vkVoidFunction CoopVec::get_function(const char* name) const
{
    if (!m_vk_module)
        return nullptr;
#if SGL_WINDOWS
    return (PFN_vkVoidFunction)::GetProcAddress((HMODULE)m_vk_module, name);
#elif SGL_LINUX
    return (PFN_vkVoidFunction)dlsym(mVkModule, name);
#else
    return nullptr;
#endif
}

size_t CoopVec::calc_stride(uint32_t rows, uint32_t cols, CoopVecMatrixLayout layout)
{
    if (layout == CoopVecMatrixLayout::row_major)
        return cols * sizeof(float16_t);
    else if (layout == CoopVecMatrixLayout::column_major)
        return rows * sizeof(float16_t);
    return 0ull;
};

VkCooperativeVectorMatrixLayoutNV CoopVec::get_vk_layout(CoopVecMatrixLayout layout)
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

size_t CoopVec::query_matrix_size(uint32_t rows, uint32_t cols, const CoopVecMatrixLayout layout)
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
    info.srcComponentType = VK_COMPONENT_TYPE_FLOAT16_NV;
    info.srcLayout = VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV;
    info.srcStride = 16 * sizeof(float16_t); // Bytes between a consecutive row or column (if row/column-major layout).
    info.srcSize = 0;
    info.srcData.hostAddress = nullptr;
    info.dstComponentType = VK_COMPONENT_TYPE_FLOAT16_NV;
    info.dstLayout = get_vk_layout(layout);
    info.dstStride = 0; // Bytes between a consecutive row or column (if row/column-major layout).
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

} // namespace sgl
