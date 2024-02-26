// SPDX-License-Identifier: Apache-2.0

#include "thread.h"

#include "kali/core/error.h"

namespace kali::thread {

static std::unique_ptr<BS::thread_pool> s_global_thread_pool;

void static_init()
{
    s_global_thread_pool = std::make_unique<BS::thread_pool>();
}

void static_shutdown()
{
    s_global_thread_pool->wait_for_tasks();
    s_global_thread_pool.reset();
}

void wait_for_tasks()
{
    global_thread_pool().wait_for_tasks();
}

BS::thread_pool& global_thread_pool()
{
    KALI_CHECK(s_global_thread_pool, "Global thread pool not initialized!");
    return *s_global_thread_pool;
}

} // namespace kali::thread
