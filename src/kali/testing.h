#pragma once

#include <doctest.h>


namespace kali {

class Device;
struct GpuTestContext {
    Device* device;
};


void run_gpu_test(void (*func)(GpuTestContext&));

} // namespace kali

#define DOCTEST_TEST_CASE_GPU(f, name)                                                                                 \
    static void f(GpuTestContext& ctx);                                                                                \
    TEST_CASE(name)                                                                                                    \
    {                                                                                                                  \
        run_gpu_test(f);                                                                                               \
    }                                                                                                                  \
    static void f(GpuTestContext& ctx)


#define TEST_CASE_GPU(name) DOCTEST_TEST_CASE_GPU(DOCTEST_ANONYMOUS(gpu_test), name)
