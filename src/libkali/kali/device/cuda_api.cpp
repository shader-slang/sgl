// SPDX-License-Identifier: Apache-2.0

#define KALI_CUDA_SYM(x) x = nullptr;

#include "cuda_api.h"

#if KALI_USE_DYNAMIC_CUDA

#include "kali/core/macros.h"
#include "kali/core/platform.h"
#include "kali/core/logger.h"

#include <string>
#include <vector>

static kali::SharedLibraryHandle kali_cuda_api_handle;

bool kali_cuda_api_init()
{
    if (kali_cuda_api_handle)
        return true;

#if KALI_WINDOWS
    std::vector<std::string> cuda_paths{"nvcuda.dll"};
#elif KALI_LINUX
    std::vector<std::string> cuda_paths{
        "/usr/lib/{x86_64-linux-gnu/libcuda.so",
        "/usr/lib/aarch64-linux-gnu/libcuda.so",
    };
#else
    std::vector<std::string> cuda_paths;
    return false;
#endif

    for (const auto& path : cuda_paths) {
        kali_cuda_api_handle = kali::platform::load_shared_library(path);
        if (kali_cuda_api_handle)
            break;
    }
    if (!kali_cuda_api_handle)
        return false;

    const char* symbol = nullptr;

#define LOAD(name, ...)                                                                                                \
    symbol = strlen(__VA_ARGS__ "") > 0 ? (#name "_" __VA_ARGS__) : #name;                                             \
    name = decltype(name)(kali::platform::get_proc_address(kali_cuda_api_handle, symbol));                             \
    if (!name)                                                                                                         \
        break;                                                                                                         \
    symbol = nullptr

    do {
        LOAD(cuGetErrorString);
        LOAD(cuGetErrorName);
        LOAD(cuInit);
        LOAD(cuDriverGetVersion);
        LOAD(cuDeviceGet);
        LOAD(cuDeviceGetCount);
        LOAD(cuDeviceGetName);
        LOAD(cuDeviceGetUuid, "v2");
        LOAD(cuDeviceGetLuid);
        LOAD(cuDeviceTotalMem, "v2");
        LOAD(cuDeviceGetAttribute);
        LOAD(cuDevicePrimaryCtxRetain);
        LOAD(cuDevicePrimaryCtxRelease, "v2");
        LOAD(cuDevicePrimaryCtxReset);
        LOAD(cuCtxPushCurrent, "v2");
        LOAD(cuCtxPopCurrent, "v2");
        LOAD(cuCtxSetCurrent);
        LOAD(cuCtxGetCurrent);
        LOAD(cuCtxGetDevice);
        LOAD(cuCtxSynchronize);
        LOAD(cuMemGetInfo);
        LOAD(cuMemAlloc, "v2");
        LOAD(cuMemFree, "v2");
        LOAD(cuMemAllocHost, "v2");
        LOAD(cuMemFreeHost);
        LOAD(cuMemcpy);
        LOAD(cuMemcpyHtoD);
        LOAD(cuMemcpyDtoH);
        LOAD(cuMemcpyDtoD);
        LOAD(cuMemcpy2D);
        LOAD(cuMemcpy2DUnaligned);
        LOAD(cuMemcpy3D);
        LOAD(cuMemcpyAsync);
        LOAD(cuMemcpyHtoDAsync);
        LOAD(cuMemcpyDtoHAsync);
        LOAD(cuMemcpyDtoDAsync);
        LOAD(cuMemcpy2DAsync, "v2");
        LOAD(cuMemcpy3DAsync, "v2");
        LOAD(cuMemsetD8);
        LOAD(cuMemsetD16);
        LOAD(cuMemsetD32);
        LOAD(cuMemsetD2D8);
        LOAD(cuMemsetD2D16);
        LOAD(cuMemsetD2D32);
        LOAD(cuMemsetD8Async);
        LOAD(cuMemsetD16Async);
        LOAD(cuMemsetD32Async);
        LOAD(cuMemsetD2D8Async);
        LOAD(cuMemsetD2D16Async);
        LOAD(cuMemsetD2D32Async);
        LOAD(cuMemAdvise, "v2");
        LOAD(cuStreamCreate);
        LOAD(cuStreamCreateWithPriority);
        LOAD(cuStreamWaitEvent);
        LOAD(cuStreamSynchronize);
        LOAD(cuStreamDestroy, "v2");
        LOAD(cuEventCreate);
        LOAD(cuEventRecord);
        LOAD(cuEventQuery);
        LOAD(cuEventSynchronize);
        LOAD(cuEventDestroy, "v2");
        LOAD(cuEventElapsedTime);
        LOAD(cuImportExternalMemory);
        LOAD(cuExternalMemoryGetMappedBuffer);
        LOAD(cuExternalMemoryGetMappedMipmappedArray);
        LOAD(cuDestroyExternalMemory);
        LOAD(cuImportExternalSemaphore);
        LOAD(cuSignalExternalSemaphoresAsync);
        LOAD(cuWaitExternalSemaphoresAsync);
        LOAD(cuDestroyExternalSemaphore);
    } while (false);
#undef LOAD

    if (symbol) {
        kali_cuda_api_shutdown();
        kali::log_warn("kali_cuda_api_init(): could not find symbol \"{}\"", symbol);
        return false;
    }

    return true;
}

void kali_cuda_api_shutdown()
{
    if (!kali_cuda_api_handle)
        return;

#define UNLOAD(name, ...) name = nullptr
    UNLOAD(cuGetErrorString);
    UNLOAD(cuGetErrorName);
    UNLOAD(cuInit);
    UNLOAD(cuDriverGetVersion);
    UNLOAD(cuDeviceGet);
    UNLOAD(cuDeviceGetCount);
    UNLOAD(cuDeviceGetName);
    UNLOAD(cuDeviceGetUuid);
    UNLOAD(cuDeviceGetLuid);
    UNLOAD(cuDeviceTotalMem);
    UNLOAD(cuDeviceGetAttribute);
    UNLOAD(cuDevicePrimaryCtxRetain);
    UNLOAD(cuDevicePrimaryCtxRelease);
    UNLOAD(cuDevicePrimaryCtxReset);
    UNLOAD(cuCtxPushCurrent);
    UNLOAD(cuCtxPopCurrent);
    UNLOAD(cuCtxSetCurrent);
    UNLOAD(cuCtxGetCurrent);
    UNLOAD(cuCtxGetDevice);
    UNLOAD(cuCtxSynchronize);
    UNLOAD(cuMemGetInfo);
    UNLOAD(cuMemAlloc);
    UNLOAD(cuMemFree);
    UNLOAD(cuMemAllocHost);
    UNLOAD(cuMemFreeHost);
    UNLOAD(cuMemcpy);
    UNLOAD(cuMemcpyHtoD);
    UNLOAD(cuMemcpyDtoH);
    UNLOAD(cuMemcpyDtoD);
    UNLOAD(cuMemcpy2D);
    UNLOAD(cuMemcpy2DUnaligned);
    UNLOAD(cuMemcpy3D);
    UNLOAD(cuMemcpyAsync);
    UNLOAD(cuMemcpyHtoDAsync);
    UNLOAD(cuMemcpyDtoHAsync);
    UNLOAD(cuMemcpyDtoDAsync);
    UNLOAD(cuMemcpy2DAsync);
    UNLOAD(cuMemcpy3DAsync);
    UNLOAD(cuMemsetD8);
    UNLOAD(cuMemsetD16);
    UNLOAD(cuMemsetD32);
    UNLOAD(cuMemsetD2D8);
    UNLOAD(cuMemsetD2D16);
    UNLOAD(cuMemsetD2D32);
    UNLOAD(cuMemsetD8Async);
    UNLOAD(cuMemsetD16Async);
    UNLOAD(cuMemsetD32Async);
    UNLOAD(cuMemsetD2D8Async);
    UNLOAD(cuMemsetD2D16Async);
    UNLOAD(cuMemsetD2D32Async);
    UNLOAD(cuMemAdvise);
    UNLOAD(cuStreamCreate);
    UNLOAD(cuStreamCreateWithPriority);
    UNLOAD(cuStreamWaitEvent);
    UNLOAD(cuStreamSynchronize);
    UNLOAD(cuStreamDestroy);
    UNLOAD(cuEventCreate);
    UNLOAD(cuEventRecord);
    UNLOAD(cuEventQuery);
    UNLOAD(cuEventSynchronize);
    UNLOAD(cuEventDestroy);
    UNLOAD(cuEventElapsedTime);
    UNLOAD(cuImportExternalMemory);
    UNLOAD(cuExternalMemoryGetMappedBuffer);
    UNLOAD(cuExternalMemoryGetMappedMipmappedArray);
    UNLOAD(cuDestroyExternalMemory);
    UNLOAD(cuImportExternalSemaphore);
    UNLOAD(cuSignalExternalSemaphoresAsync);
    UNLOAD(cuWaitExternalSemaphoresAsync);
    UNLOAD(cuDestroyExternalSemaphore);
#undef UNLOAD

    kali::platform::release_shared_library(kali_cuda_api_handle);
    kali_cuda_api_handle = nullptr;
}

#else // KALI_USE_DYNAMIC_CUDA

bool kali_cuda_api_init()
{
    return true;
}

void kali_cuda_api_shutdown() { }

#endif // KALI_USE_DYNAMIC_CUDA
