// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/error.h"

#include <slang-rhi.h>

#include <cstdint>

namespace sgl {

enum class NativeHandleType {
    unknown = static_cast<uint32_t>(rhi::NativeHandleType::Unknown),

    win32 = static_cast<uint32_t>(rhi::NativeHandleType::Win32),
    file_descriptor = static_cast<uint32_t>(rhi::NativeHandleType::FileDescriptor),

    D3D12Device = static_cast<uint32_t>(rhi::NativeHandleType::D3D12Device),
    D3D12CommandQueue = static_cast<uint32_t>(rhi::NativeHandleType::D3D12CommandQueue),
    D3D12GraphicsCommandList = static_cast<uint32_t>(rhi::NativeHandleType::D3D12GraphicsCommandList),
    D3D12Resource = static_cast<uint32_t>(rhi::NativeHandleType::D3D12Resource),
    D3D12PipelineState = static_cast<uint32_t>(rhi::NativeHandleType::D3D12PipelineState),
    D3D12StateObject = static_cast<uint32_t>(rhi::NativeHandleType::D3D12StateObject),
    D3D12CpuDescriptorHandle = static_cast<uint32_t>(rhi::NativeHandleType::D3D12CpuDescriptorHandle),
    D3D12Fence = static_cast<uint32_t>(rhi::NativeHandleType::D3D12Fence),
    D3D12DeviceAddress = static_cast<uint32_t>(rhi::NativeHandleType::D3D12DeviceAddress),

    VkDevice = static_cast<uint32_t>(rhi::NativeHandleType::VkDevice),
    VkPhysicalDevice = static_cast<uint32_t>(rhi::NativeHandleType::VkPhysicalDevice),
    VkInstance = static_cast<uint32_t>(rhi::NativeHandleType::VkInstance),
    VkQueue = static_cast<uint32_t>(rhi::NativeHandleType::VkQueue),
    VkCommandBuffer = static_cast<uint32_t>(rhi::NativeHandleType::VkCommandBuffer),
    VkBuffer = static_cast<uint32_t>(rhi::NativeHandleType::VkBuffer),
    VkImage = static_cast<uint32_t>(rhi::NativeHandleType::VkImage),
    VkImageView = static_cast<uint32_t>(rhi::NativeHandleType::VkImageView),
    VkAccelerationStructureKHR = static_cast<uint32_t>(rhi::NativeHandleType::VkAccelerationStructureKHR),
    VkSampler = static_cast<uint32_t>(rhi::NativeHandleType::VkSampler),
    VkPipeline = static_cast<uint32_t>(rhi::NativeHandleType::VkPipeline),
    VkSemaphore = static_cast<uint32_t>(rhi::NativeHandleType::VkSemaphore),

    MTLDevice = static_cast<uint32_t>(rhi::NativeHandleType::MTLDevice),
    MTLCommandQueue = static_cast<uint32_t>(rhi::NativeHandleType::MTLCommandQueue),
    MTLCommandBuffer = static_cast<uint32_t>(rhi::NativeHandleType::MTLCommandBuffer),
    MTLTexture = static_cast<uint32_t>(rhi::NativeHandleType::MTLTexture),
    MTLBuffer = static_cast<uint32_t>(rhi::NativeHandleType::MTLBuffer),
    MTLComputePipelineState = static_cast<uint32_t>(rhi::NativeHandleType::MTLComputePipelineState),
    MTLRenderPipelineState = static_cast<uint32_t>(rhi::NativeHandleType::MTLRenderPipelineState),
    MTLSharedEvent = static_cast<uint32_t>(rhi::NativeHandleType::MTLSharedEvent),
    MTLSamplerState = static_cast<uint32_t>(rhi::NativeHandleType::MTLSamplerState),
    MTLAccelerationStructure = static_cast<uint32_t>(rhi::NativeHandleType::MTLAccelerationStructure),

    CUdevice = static_cast<uint32_t>(rhi::NativeHandleType::CUdevice),
    CUdeviceptr = static_cast<uint32_t>(rhi::NativeHandleType::CUdeviceptr),
    CUtexObject = static_cast<uint32_t>(rhi::NativeHandleType::CUtexObject),
    CUstream = static_cast<uint32_t>(rhi::NativeHandleType::CUstream),

    OptixDeviceContext = static_cast<uint32_t>(rhi::NativeHandleType::OptixDeviceContext),
    OptixTraversableHandle = static_cast<uint32_t>(rhi::NativeHandleType::OptixTraversableHandle),

    WGPUDevice = static_cast<uint32_t>(rhi::NativeHandleType::WGPUDevice),
    WGPUBuffer = static_cast<uint32_t>(rhi::NativeHandleType::WGPUBuffer),
    WGPUTexture = static_cast<uint32_t>(rhi::NativeHandleType::WGPUTexture),
    WGPUSampler = static_cast<uint32_t>(rhi::NativeHandleType::WGPUSampler),
    WGPURenderPipeline = static_cast<uint32_t>(rhi::NativeHandleType::WGPURenderPipeline),
    WGPUComputePipeline = static_cast<uint32_t>(rhi::NativeHandleType::WGPUComputePipeline),
    WGPUQueue = static_cast<uint32_t>(rhi::NativeHandleType::WGPUQueue),
    WGPUCommandBuffer = static_cast<uint32_t>(rhi::NativeHandleType::WGPUCommandBuffer),
    WGPUTextureView = static_cast<uint32_t>(rhi::NativeHandleType::WGPUTextureView),
    WGPUCommandEncoder = static_cast<uint32_t>(rhi::NativeHandleType::WGPUCommandEncoder),
};

template<typename T>
struct NativeHandleTrait;

/// Represents a native graphics API handle (e.g. D3D12 or Vulkan).
/// Native handles are expected to fit into 64 bits.
/// Type information and conversion from/to native handles is done
/// using type traits from native_handle_traits.h which needs to be
/// included when creating and accessing NativeHandle.
/// This separation is done so we don't expose the heavy D3D12/Vulkan
/// headers everywhere.
class NativeHandle {
public:
    NativeHandle() = default;

    explicit NativeHandle(const rhi::NativeHandle& handle)
    {
        m_type = static_cast<NativeHandleType>(handle.type);
        m_value = handle.value;
    }

    template<typename T>
    explicit NativeHandle(T native)
    {
        m_type = NativeHandleTrait<T>::type;
        m_value = NativeHandleTrait<T>::pack(native);
    }

    NativeHandleType type() const { return m_type; }
    uint64_t value() const { return m_value; }

    bool is_valid() const { return m_type != NativeHandleType::unknown; }

    operator bool() const { return is_valid(); }

    template<typename T>
    T as() const
    {
        SGL_ASSERT(m_type == NativeHandleTrait<T>::type);
        return NativeHandleTrait<T>::unpack(m_value);
    }

private:
    NativeHandleType m_type{NativeHandleType::unknown};
    uint64_t m_value{0};
};

} // namespace sgl
