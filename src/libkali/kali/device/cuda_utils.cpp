// SPDX-License-Identifier: Apache-2.0

#include "cuda_utils.h"

#if KALI_HAS_CUDA

#include "kali/core/error.h"

#include "kali/device/device.h"
#include "kali/device/resource.h"
#include "kali/device/fence.h"
#include "kali/device/command.h"

#include <span>

namespace kali::cuda {

void device_synchronize()
{
    cudaDeviceSynchronize();
    cudaError_t error = cudaGetLastError();
    KALI_CHECK(error == cudaSuccess, "Failed to sync CUDA device: {}.", cudaGetErrorString(error));
}

void* malloc_device(size_t size)
{
    void* dev_ptr;
    KALI_CUDA_CHECK(cudaMalloc(&dev_ptr, size));
    return dev_ptr;
}

void free_device(void* dev_ptr)
{
    if (!dev_ptr)
        return;
    KALI_CUDA_CHECK(cudaFree(dev_ptr));
}

void memcpy_device_to_device(void* dst, const void* src, size_t count)
{
    KALI_CUDA_CHECK(cudaMemcpy(dst, src, count, cudaMemcpyDeviceToDevice));
}

void memcpy_host_to_device(void* dst, const void* src, size_t count)
{
    KALI_CUDA_CHECK(cudaMemcpy(dst, src, count, cudaMemcpyHostToDevice));
}

void memcpy_device_to_host(void* dst, const void* src, size_t count)
{
    KALI_CUDA_CHECK(cudaMemcpy(dst, src, count, cudaMemcpyDeviceToHost));
}

void memset_device(void* dev_ptr, int value, size_t count)
{
    KALI_CUDA_CHECK(cudaMemset(dev_ptr, value, count));
}

cudaExternalMemory_t import_external_memory(const Buffer* buffer)
{
    KALI_CHECK_NOT_NULL(buffer);
    KALI_CHECK(
        is_set(buffer->desc().usage, ResourceUsage::shared),
        "Buffer was not created with ResourceUsage::shared."
    );
    SharedResourceHandle shared_handle = buffer->get_shared_handle();
    KALI_CHECK(shared_handle, "Buffer shared handle creation failed.");

    cudaExternalMemoryHandleDesc desc = {};
    switch (buffer->device()->type()) {
#if KALI_WINDOWS
    case DeviceType::d3d12:
        desc.type = cudaExternalMemoryHandleTypeD3D12Resource;
        desc.handle.win32.handle = shared_handle;
        break;
    case DeviceType::vulkan:
        desc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
        desc.handle.win32.handle = shared_handle;
        break;
#elif KALI_LINUX
    case DeviceType::vulkan:
        desc.type = cudaExternalMemoryHandleTypeOpaqueFd;
        desc.handle.fd = shared_handle;
        break;
#endif
    default:
        KALI_THROW("Unsupported device type {}.", buffer->device()->type());
    }
    desc.size = buffer->size();
    desc.flags = cudaExternalMemoryDedicated;

    cudaExternalMemory_t ext_mem;
    KALI_CUDA_CHECK(cudaImportExternalMemory(&ext_mem, &desc));
    return ext_mem;
}

void destroy_external_memory(cudaExternalMemory_t ext_mem)
{
    KALI_CUDA_CHECK(cudaDestroyExternalMemory(ext_mem));
}

void* external_memory_get_mapped_buffer(cudaExternalMemory_t ext_mem, size_t offset, size_t size)
{
    cudaExternalMemoryBufferDesc desc = {};
    desc.offset = offset;
    desc.size = size;

    void* dev_ptr = nullptr;
    KALI_CUDA_CHECK(cudaExternalMemoryGetMappedBuffer(&dev_ptr, ext_mem, &desc));
    return dev_ptr;
}

cudaExternalSemaphore_t import_external_semaphore(const Fence* fence)
{
    KALI_CHECK_NOT_NULL(fence);
    KALI_CHECK(fence->desc().shared, "Fence was not created with shared flag.");
    SharedFenceHandle shared_handle = fence->get_shared_handle();
    KALI_CHECK(shared_handle, "Fence shared handle creation failed.");

    cudaExternalSemaphoreHandleDesc desc = {};
    switch (fence->device()->type()) {
#if KALI_WINDOWS
    case DeviceType::d3d12:
        desc.type = cudaExternalSemaphoreHandleTypeD3D12Fence;
        desc.handle.win32.handle = (void*)shared_handle;
        break;
    case DeviceType::vulkan:
        desc.type = cudaExternalSemaphoreHandleTypeTimelineSemaphoreWin32;
        desc.handle.win32.handle = (void*)shared_handle;
        break;
#elif KALI_LINUX
    case DeviceType::vulkan:
        desc.type = cudaExternalSemaphoreHandleTypeTimelineSemaphoreFd;
        desc.handle.fd = shared_handle;
        break;
#endif
    default:
        KALI_THROW("Unsupported device type {}.", fence->device()->type());
    }

    cudaExternalSemaphore_t ext_sem;
    KALI_CUDA_CHECK(cudaImportExternalSemaphore(&ext_sem, &desc));
    return ext_sem;
}

void destroy_external_semaphore(cudaExternalSemaphore_t ext_sem)
{
    KALI_CUDA_CHECK(cudaDestroyExternalSemaphore(ext_sem));
}

void signal_external_semaphore(cudaExternalSemaphore_t ext_sem, uint64_t value, cudaStream_t stream)
{
    cudaExternalSemaphoreSignalParams params = {};
    params.params.fence.value = value;
    KALI_CUDA_CHECK(cudaSignalExternalSemaphoresAsync(&ext_sem, &params, 1, stream));
}

void wait_external_semaphore(cudaExternalSemaphore_t ext_sem, uint64_t value, cudaStream_t stream)
{
    cudaExternalSemaphoreWaitParams params = {};
    params.params.fence.value = value;
    KALI_CUDA_CHECK(cudaWaitExternalSemaphoresAsync(&ext_sem, &params, 1, stream));
}

inline int32_t find_device_by_luid(std::span<cudaDeviceProp> devices, const AdapterLUID& luid)
{
    for (int32_t i = 0; i < devices.size(); ++i) {
#if KALI_WINDOWS
        // On Windows, we compare the 8-byte LUID. The LUID is the same for
        // D3D12, Vulkan and CUDA.
        static_assert(sizeof(cudaDeviceProp::luid) <= sizeof(AdapterLUID));
        if (std::memcmp(devices[i].luid, luid.data(), sizeof(cudaDeviceProp::luid)) == 0)
            return i;
#elif KALI_LINUX
        // On Linux, the LUID is not supported. Instead we compare the 16-byte
        // UUID which GFX conveniently returns in-place of the LUID.
        static_assert(sizeof(cudaDeviceProp::uuid) <= sizeof(AdapterLUID));
        if (std::memcmp(&devices[i].uuid, luid.data(), sizeof(cudaDeviceProp::uuid)) == 0)
            return i;
#endif
    }
    return -1;
}

inline int32_t find_device_by_name(std::span<cudaDeviceProp> devices, std::string_view name)
{
    for (int32_t i = 0; i < devices.size(); ++i)
        if (devices[i].name == name)
            return i;
    return -1;
}

Device::Device(const kali::Device* device)
{
    KALI_CHECK_NOT_NULL(device);
    KALI_CU_CHECK(cuInit(0));

    // Get a list of all available CUDA devices.
    int32_t cuda_device_count;
    KALI_CUDA_CHECK(cudaGetDeviceCount(&cuda_device_count));
    std::vector<cudaDeviceProp> cuda_devices(cuda_device_count);
    for (int32_t i = 0; i < cuda_device_count; ++i)
        KALI_CUDA_CHECK(cudaGetDeviceProperties(&cuda_devices[i], i));

    // First we try to find the matching CUDA device by LUID.
    int32_t selected_device = find_device_by_luid(cuda_devices, device->info().adapter_luid);
    if (selected_device < 0) {
        log_warn("Failed to find CUDA device by LUID. Falling back to device name.");
        // Next we try to find the matching CUDA device by name.
        selected_device = find_device_by_name(cuda_devices, device->info().adapter_name);
        if (selected_device < 0) {
            log_warn("Failed to find CUDA device by name. Falling back to first compatible device.");
            // Finally we try to find the first compatible CUDA device.
            for (int32_t i = 0; i < cuda_devices.size(); ++i) {
                if (cuda_devices[i].major >= 7) {
                    selected_device = i;
                    break;
                }
            }
        }
    }

    if (selected_device < 0)
        KALI_THROW("No compatible CUDA device found.");

    KALI_CUDA_CHECK(cudaSetDevice(selected_device));
    KALI_CU_CHECK(cuDeviceGet(&m_device, selected_device));
    KALI_CU_CHECK(cuDevicePrimaryCtxRetain(&m_context, m_device));
    KALI_CU_CHECK(cuStreamCreate(&m_stream, CU_STREAM_DEFAULT));

    const auto& props = cuda_devices[selected_device];
    log_info("Created CUDA device '{}' (architecture {}.{}).", props.name, props.major, props.minor);
}

Device::~Device()
{
    KALI_CU_CHECK(cuStreamDestroy(m_stream));
    KALI_CU_CHECK(cuDevicePrimaryCtxRelease(m_device));
}

ExternalMemory::ExternalMemory(const Buffer* buffer)
    : m_resource(buffer)
    , m_external_memory(import_external_memory(buffer))
    , m_size(buffer->size())
{
}

ExternalMemory::~ExternalMemory()
{
    destroy_external_memory(m_external_memory);
}

void* ExternalMemory::mapped_data() const
{
    if (!m_mapped_data)
        m_mapped_data = external_memory_get_mapped_buffer(m_external_memory, 0, m_size);
    return m_mapped_data;
}

ExternalSemaphore::ExternalSemaphore(Fence* fence)
    : m_fence(fence)
    , m_external_semaphore(import_external_semaphore(fence))
{
}

ExternalSemaphore::~ExternalSemaphore()
{
    destroy_external_semaphore(m_external_semaphore);
}

void ExternalSemaphore::signal(uint64_t value, cudaStream_t stream)
{
    signal_external_semaphore(m_external_semaphore, value, stream);
}

void ExternalSemaphore::wait(uint64_t value, cudaStream_t stream)
{
    wait_external_semaphore(m_external_semaphore, value, stream);
}

void ExternalSemaphore::wait_for_cuda(CommandQueue* command_queue, cudaStream_t cuda_stream, uint64_t value)
{
    uint64_t signal_value = m_fence->update_signaled_value(value);
    signal(signal_value, cuda_stream);
    command_queue->wait(m_fence, signal_value);
}

void ExternalSemaphore::wait_for_device(CommandQueue* command_queue, cudaStream_t cuda_stream, uint64_t value)
{
    uint64_t signal_value = command_queue->signal(m_fence, value);
    wait(signal_value, cuda_stream);
}

} // namespace kali::cuda

#endif // KALI_HAS_CUDA
