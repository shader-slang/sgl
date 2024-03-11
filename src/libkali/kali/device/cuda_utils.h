// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "kali/core/config.h"

#if KALI_HAS_CUDA

#include "kali/core/macros.h"

#include "kali/device/fwd.h"
#include "kali/device/fence.h"
#include "kali/device/cuda_api.h"

#define KALI_CU_CHECK(call)                                                                                            \
    do {                                                                                                               \
        CUresult result = call;                                                                                        \
        if (result != CUDA_SUCCESS) {                                                                                  \
            const char* errorName;                                                                                     \
            cuGetErrorName(result, &errorName);                                                                        \
            const char* errorString;                                                                                   \
            cuGetErrorString(result, &errorString);                                                                    \
            KALI_THROW("CUDA call {} failed with error {} ({}).", #call, errorName, errorString);                      \
        }                                                                                                              \
    } while (0)

namespace kali::cuda {

KALI_API void* malloc_device(size_t size);
KALI_API void free_device(void* ptr);

KALI_API void memcpy_device_to_device(void* dst, const void* src, size_t count);
KALI_API void memcpy_host_to_device(void* dst, const void* src, size_t count);
KALI_API void memcpy_device_to_host(void* dst, const void* src, size_t count);

KALI_API void memset_device(void* dst, uint8_t value, size_t count);

KALI_API CUexternalMemory import_external_memory(const Buffer* buffer);
KALI_API void destroy_external_memory(CUexternalMemory ext_mem);
KALI_API void* external_memory_get_mapped_buffer(CUexternalMemory ext_mem, size_t offset, size_t size);

KALI_API CUexternalSemaphore import_external_semaphore(const Fence* fence);
KALI_API void destroy_external_semaphore(CUexternalSemaphore ext_sem);
KALI_API void signal_external_semaphore(CUexternalSemaphore ext_sem, uint64_t value, CUstream stream = 0);
KALI_API void wait_external_semaphore(CUexternalSemaphore ext_sem, uint64_t value, CUstream stream = 0);

/// Wraps a CUDA device, context and stream.
class KALI_API Device : public Object {
    KALI_OBJECT(cuda::Device)
public:
    /// Constructor.
    /// Creates a CUDA device on the same adapter as the kali device.
    explicit Device(const kali::Device* device);
    ~Device();

    CUdevice device() const { return m_device; }
    CUcontext context() const { return m_context; }
    CUstream stream() const { return m_stream; }

private:
    CUdevice m_device;
    CUcontext m_context;
    CUstream m_stream;
};

/// Wraps an external memory resource.
class KALI_API ExternalMemory : public Object {
    KALI_OBJECT(cuda::ExternalMemory)
public:
    explicit ExternalMemory(const Buffer* buffer);
    ~ExternalMemory();

    size_t size() const { return m_size; }

    void* mapped_data() const;

private:
    /// Non-owning pointer to the resource.
    const Resource* m_resource;
    CUexternalMemory m_external_memory;
    size_t m_size;
    mutable void* m_mapped_data{nullptr};
};

/// Wraps an external semaphore.
class KALI_API ExternalSemaphore : public Object {
    KALI_OBJECT(cuda::ExternalSemaphore)
public:
    explicit ExternalSemaphore(Fence* fence);
    ~ExternalSemaphore();

    void signal(uint64_t value, CUstream stream = 0);
    void wait(uint64_t value, CUstream stream = 0);

    void wait_for_cuda(CommandQueue* command_queue, CUstream cuda_stream = 0, uint64_t value = Fence::AUTO);
    void wait_for_device(CommandQueue* command_queue, CUstream cuda_stream = 0, uint64_t value = Fence::AUTO);

private:
    /// Non-owning pointer to the fence.
    Fence* m_fence;
    CUexternalSemaphore m_external_semaphore;
};

} // namespace kali::cuda

#endif // KALI_HAS_CUDA
