// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <doctest/doctest.h>

namespace sgl {

class Device;

} // namespace sgl

namespace sgl::testing {

void static_init();
void static_shutdown();

struct GpuTestContext {
    Device* device;
};

void run_gpu_test(void (*func)(GpuTestContext&));

void release_cached_devices();

} // namespace sgl::testing


#define DOCTEST_TEST_CASE_GPU(f, name)                                                                                 \
    static void f(::sgl::testing::GpuTestContext& ctx);                                                               \
    TEST_CASE(name)                                                                                                    \
    {                                                                                                                  \
        ::sgl::testing::run_gpu_test(f);                                                                              \
    }                                                                                                                  \
    static void f(::sgl::testing::GpuTestContext& ctx)


#define TEST_CASE_GPU(name) DOCTEST_TEST_CASE_GPU(DOCTEST_ANONYMOUS(gpu_test), name)
