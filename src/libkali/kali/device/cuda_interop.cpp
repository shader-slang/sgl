// SPDX-License-Identifier: Apache-2.0

#include "cuda_interop.h"

#include "kali/device/device.h"
#include "kali/device/command.h"
#include "kali/device/cuda_utils.h"

namespace kali::cuda {

InteropBuffer::InteropBuffer(kali::Device* device, const TensorView tensor_view, bool is_uav)
    : m_device(device)
    , m_tensor_view(tensor_view)
    , m_is_uav(is_uav)
{
    m_buffer = device->create_buffer({
        .size = m_tensor_view.size,
        .struct_size = 4,
        .initial_state = is_uav ? ResourceState::unordered_access : ResourceState::shader_resource,
        .usage = ResourceUsage::shared | (is_uav ? ResourceUsage::unordered_access : ResourceUsage::shader_resource),
    });
    m_external_memory = make_ref<cuda::ExternalMemory>(m_buffer);
}

InteropBuffer::~InteropBuffer() { }

ref<ResourceView> InteropBuffer::get_resource_view() const
{
    return m_is_uav ? m_buffer->get_uav() : m_buffer->get_srv();
}

void InteropBuffer::copy_from_cuda(void* cuda_stream)
{
    KALI_CU_CHECK(cuMemcpyAsync(
        reinterpret_cast<CUdeviceptr>(m_external_memory->mapped_data()),
        reinterpret_cast<CUdeviceptr>(m_tensor_view.data),
        m_tensor_view.size,
        static_cast<CUstream>(cuda_stream)
    ));
}

void InteropBuffer::copy_to_cuda(void* cuda_stream)
{
    KALI_CU_CHECK(cuMemcpyAsync(
        reinterpret_cast<CUdeviceptr>(m_tensor_view.data),
        reinterpret_cast<CUdeviceptr>(m_external_memory->mapped_data()),
        m_tensor_view.size,
        static_cast<CUstream>(cuda_stream)
    ));
}

} // namespace kali::cuda
