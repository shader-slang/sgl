#pragma once

#include "native_handle.h"

#include "kali/core/config.h"

#if KALI_HAS_D3D12
#include <d3d12.h>
#endif

#if KALI_HAS_VULKAN
#include <vulkan/vulkan.h>
#endif

#include <bit>

namespace kali {

template<typename T>
struct NativeHandleTrait { };

#define KALI_NATIVE_HANDLE(T, TYPE)                                                                                    \
    template<>                                                                                                         \
    struct NativeHandleTrait<T> {                                                                                      \
        static const NativeHandleType type = TYPE;                                                                     \
        static uint64_t pack(T native)                                                                                 \
        {                                                                                                              \
            return std::bit_cast<uint64_t>(native);                                                                    \
        }                                                                                                              \
        static T unpack(uint64_t value)                                                                                \
        {                                                                                                              \
            return std::bit_cast<T>(value);                                                                            \
        }                                                                                                              \
    };

#if KALI_HAS_D3D12
KALI_NATIVE_HANDLE(ID3D12Device*, NativeHandleType::ID3D12Device);
KALI_NATIVE_HANDLE(ID3D12Resource*, NativeHandleType::ID3D12Resource);
KALI_NATIVE_HANDLE(ID3D12PipelineState*, NativeHandleType::ID3D12PipelineState);
KALI_NATIVE_HANDLE(ID3D12Fence*, NativeHandleType::ID3D12Fence);
KALI_NATIVE_HANDLE(ID3D12CommandQueue*, NativeHandleType::ID3D12CommandQueue);
KALI_NATIVE_HANDLE(ID3D12GraphicsCommandList*, NativeHandleType::ID3D12GraphicsCommandList);
KALI_NATIVE_HANDLE(D3D12_CPU_DESCRIPTOR_HANDLE, NativeHandleType::D3D12_CPU_DESCRIPTOR_HANDLE);
#endif // KALI_HAS_D3D12

#if KALI_HAS_VULKAN
KALI_NATIVE_HANDLE(VkInstance, NativeHandleType::VkInstance);
KALI_NATIVE_HANDLE(VkPhysicalDevice, NativeHandleType::VkPhysicalDevice);
KALI_NATIVE_HANDLE(VkDevice, NativeHandleType::VkDevice);
KALI_NATIVE_HANDLE(VkImage, NativeHandleType::VkImage);
KALI_NATIVE_HANDLE(VkImageView, NativeHandleType::VkImageView);
KALI_NATIVE_HANDLE(VkBuffer, NativeHandleType::VkBuffer);
KALI_NATIVE_HANDLE(VkBufferView, NativeHandleType::VkBufferView);
KALI_NATIVE_HANDLE(VkPipeline, NativeHandleType::VkPipeline);
KALI_NATIVE_HANDLE(VkFence, NativeHandleType::VkFence);
KALI_NATIVE_HANDLE(VkQueue, NativeHandleType::VkQueue);
KALI_NATIVE_HANDLE(VkCommandBuffer, NativeHandleType::VkCommandBuffer);
KALI_NATIVE_HANDLE(VkSampler, NativeHandleType::VkSampler);
#endif // KALI_HAS_VULKAN

#undef KALI_NATIVE_HANDLE

} // namespace kali
