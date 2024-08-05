// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/error.h"

#include <cstdint>

namespace sgl {

enum class NativeHandleType {
    unknown,

    ID3D12Device,
    ID3D12Resource,
    ID3D12PipelineState,
    ID3D12Fence,
    ID3D12CommandQueue,
    ID3D12GraphicsCommandList,
    D3D12_CPU_DESCRIPTOR_HANDLE,

    VkInstance,
    VkPhysicalDevice,
    VkDevice,
    VkImage,
    VkImageView,
    VkBuffer,
    VkBufferView,
    VkPipeline,
    VkFence,
    VkQueue,
    VkCommandBuffer,
    VkSampler,
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

    template<typename T>
    explicit NativeHandle(T native)
    {
        m_type = NativeHandleTrait<T>::type;
        m_value = NativeHandleTrait<T>::pack(native);
    }

    NativeHandleType type() const { return m_type; }

    bool is_valid() const { return m_type != NativeHandleType::unknown; }

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
