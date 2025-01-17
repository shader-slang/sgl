/***************************************************************************
 # Copyright (c) 2015-24, NVIDIA CORPORATION. All rights reserved.
 #
 # NVIDIA CORPORATION and its licensors retain all intellectual property
 # and proprietary rights in and to this software, related documentation
 # and any modifications thereto.  Any use, reproduction, disclosure or
 # distribution of this software and related documentation without an express
 # license agreement from NVIDIA CORPORATION is strictly prohibited.
 **************************************************************************/

// Header from Jeff Bolz, up-to-date as of 2024-02-21.
#pragma once
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VK_NV_cooperative_vector 1
#define VK_NV_COOPERATIVE_VECTOR_SPEC_VERSION 2
#define VK_NV_COOPERATIVE_VECTOR_EXTENSION_NAME "VK_NV_cooperative_vector"
#define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_FEATURES_NV ((VkStructureType)1000491000)
#define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_PROPERTIES_NV ((VkStructureType)1000491001)
#define VK_STRUCTURE_TYPE_COOPERATIVE_VECTOR_PROPERTIES_NV ((VkStructureType)1000491002)
#define VK_STRUCTURE_TYPE_CONVERT_COOPERATIVE_VECTOR_MATRIX_LAYOUT_INFO_NV ((VkStructureType)1000491003)
#define VK_STRUCTURE_TYPE_CONVERT_COOPERATIVE_VECTOR_MATRIX_INFO_NV ((VkStructureType)1000491004)
#define VK_COMPONENT_TYPE_SINT8_PACKED_NV ((VkComponentTypeKHR)1000491000)
#define VK_COMPONENT_TYPE_UINT8_PACKED_NV ((VkComponentTypeKHR)1000491001)
#define VK_COMPONENT_TYPE_FLOAT_E4M3_NV ((VkComponentTypeKHR)1000491002)
#define VK_COMPONENT_TYPE_FLOAT_E5M2_NV ((VkComponentTypeKHR)1000491003)
#define VK_PIPELINE_STAGE_2_CONVERT_COOPERATIVE_VECTOR_MATRIX_LAYOUT_BIT_NV                                            \
    ((VkPipelineStageFlagBits2)0x0000100000000000ULL)

// TODO [fpclarbe]: Added this
typedef VkComponentTypeNV VkComponentTypeKHR;

typedef struct VkPhysicalDeviceCooperativeVectorPropertiesNV {
    VkStructureType sType;
    void* pNext;
    VkShaderStageFlags cooperativeVectorSupportedStages;
    VkBool32 cooperativeVectorTraining;
} VkPhysicalDeviceCooperativeVectorPropertiesNV;

typedef struct VkPhysicalDeviceCooperativeVectorFeaturesNV {
    VkStructureType sType;
    void* pNext;
    VkBool32 cooperativeVector;
    VkBool32 cooperativeVectorRobustBufferAccess;
} VkPhysicalDeviceCooperativeVectorFeaturesNV;

typedef struct VkCooperativeVectorPropertiesNV {
    VkStructureType sType;
    void* pNext;
    VkComponentTypeKHR inputType;
    VkComponentTypeKHR inputInterpretation;
    VkComponentTypeKHR matrixInterpretation;
    VkComponentTypeKHR biasInterpretation;
    VkComponentTypeKHR resultType;
    VkBool32 transpose;
} VkCooperativeVectorPropertiesNV;

typedef enum VkCooperativeVectorMatrixLayoutNV {
    VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV = 0,
    VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_COLUMN_MAJOR_NV = 1,
    VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_INFERENCING_OPTIMAL_NV = 2,
    VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_TRAINING_OPTIMAL_NV = 3,
    VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_MAX_ENUM_NV = 0x7FFFFFFF
} VkCooperativeVectorMatrixLayoutNV;

typedef struct VkConvertCooperativeVectorMatrixLayoutInfoNV {
    VkStructureType sType;
    void const* pNext;
    VkComponentTypeKHR srcComponentType;
    VkComponentTypeKHR dstComponentType;
    uint32_t numRows;
    uint32_t numColumns;
    VkCooperativeVectorMatrixLayoutNV srcLayout;
    size_t srcStride;
    VkCooperativeVectorMatrixLayoutNV dstLayout;
    size_t dstStride;
} VkConvertCooperativeVectorMatrixLayoutInfoNV;

typedef struct VkConvertCooperativeVectorMatrixInfoNV {
    VkStructureType sType;
    void const* pNext;
    size_t srcSize;
    VkDeviceOrHostAddressConstKHR srcData;
    size_t* pDstSize;
    VkDeviceOrHostAddressKHR dstData;
    VkComponentTypeKHR srcComponentType;
    VkComponentTypeKHR dstComponentType;
    uint32_t numRows;
    uint32_t numColumns;
    VkCooperativeVectorMatrixLayoutNV srcLayout;
    size_t srcStride;
    VkCooperativeVectorMatrixLayoutNV dstLayout;
    size_t dstStride;
} VkConvertCooperativeVectorMatrixInfoNV;

typedef VkResult(VKAPI_PTR* PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV)(
    VkPhysicalDevice physicalDevice,
    uint32_t* pPropertyCount,
    VkCooperativeVectorPropertiesNV* pProperties
);
typedef VkResult(VKAPI_PTR* PFN_vkConvertCooperativeVectorMatrixLayoutNV)(
    VkDevice device,
    const VkConvertCooperativeVectorMatrixLayoutInfoNV* pInfo,
    size_t srcSize,
    const void* pSrcData,
    size_t* pDstSize,
    void* pDstData
);
typedef VkResult(VKAPI_PTR* PFN_vkConvertCooperativeVectorMatrixNV)(
    VkDevice device,
    const VkConvertCooperativeVectorMatrixInfoNV* pInfo
);
typedef void(VKAPI_PTR* PFN_vkCmdConvertCooperativeVectorMatrixNV)(
    VkCommandBuffer commandBuffer,
    uint32_t infoCount,
    const VkConvertCooperativeVectorMatrixInfoNV* pInfos
);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceCooperativeVectorPropertiesNV(
    VkPhysicalDevice physicalDevice,
    uint32_t* pPropertyCount,
    VkCooperativeVectorPropertiesNV* pProperties
);

VKAPI_ATTR VkResult VKAPI_CALL vkConvertCooperativeVectorMatrixLayoutNV(
    VkDevice device,
    VkConvertCooperativeVectorMatrixLayoutInfoNV const* pInfo,
    size_t srcSize,
    void const* pSrcData,
    size_t* pDstSize,
    void* pDstData
);

VKAPI_ATTR VkResult VKAPI_CALL
vkConvertCooperativeVectorMatrixNV(VkDevice device, VkConvertCooperativeVectorMatrixInfoNV const* pInfo);

VKAPI_ATTR void VKAPI_CALL vkCmdConvertCooperativeVectorMatrixNV(
    VkCommandBuffer commandBuffer,
    uint32_t infoCount,
    VkConvertCooperativeVectorMatrixInfoNV const* pInfos
);
#endif
