// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/macros.h"

#include <BS_thread_pool.hpp>

#include <type_traits>
#include <future>

namespace sgl::thread {

SGL_API void static_init();
SGL_API void static_shutdown();

/// Block until all scheduled tasks are completed.
SGL_API void wait_for_tasks();

SGL_API BS::thread_pool& global_thread_pool();

template<typename F, typename... A, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>
std::future<R> do_async(F&& task, A&&... args)
{
    return global_thread_pool().submit(std::forward<F>(task), std::forward<A>(args)...);
}

} // namespace sgl::thread
