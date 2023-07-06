#pragma once

#include <doctest.h>


namespace kali {

class Device;

} // namespace kali

namespace kali::testing {

struct GpuTestContext {
    Device* device;
};

void run_gpu_test(void (*func)(GpuTestContext&));

void release_cached_devices();

} // namespace kali::testing


#define DOCTEST_TEST_CASE_GPU(f, name)                                                                                 \
    static void f(::kali::testing::GpuTestContext& ctx);                                                               \
    TEST_CASE(name)                                                                                                    \
    {                                                                                                                  \
        ::kali::testing::run_gpu_test(f);                                                                              \
    }                                                                                                                  \
    static void f(::kali::testing::GpuTestContext& ctx)


#define TEST_CASE_GPU(name) DOCTEST_TEST_CASE_GPU(DOCTEST_ANONYMOUS(gpu_test), name)
