#pragma once

#include "kali/core/macros.h"

#include <BS_thread_pool.hpp>

#include <type_traits>
#include <future>

namespace kali::thread {

KALI_API void static_init();
KALI_API void static_shutdown();

KALI_API void wait_for_tasks();

KALI_API BS::thread_pool& global_thread_pool();

template<typename F, typename... A, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>
std::future<R> do_async(F&& task, A&&... args)
{
    return global_thread_pool().submit(std::forward<F>(task), std::forward<A>(args)...);
}

} // namespace kali::thread
