// SPDX-License-Identifier: Apache-2.0

#include "kali.h"

#include "kali/core/logger.h"
#include "kali/core/platform.h"
#include "kali/core/bitmap.h"
#include "kali/core/format.h"
#include "kali/core/thread.h"

#include "git_version.h"

static inline const char* git_version()
{
    static std::string str{
        fmt::format("commit: {} / branch: {}", GIT_VERSION_COMMIT, GIT_VERSION_BRANCH)
        + (GIT_VERSION_DIRTY ? " (local changes)" : "")};
    return str.c_str();
}

const char* KALI_GIT_VERSION = git_version();

namespace kali {

void static_init()
{
    thread::static_init();
    Logger::static_init();
    platform::static_init();
    Bitmap::static_init();
}

void static_shutdown()
{
    thread::wait_for_tasks();

    Bitmap::static_shutdown();
    platform::static_shutdown();
    Logger::static_shutdown();
    thread::static_shutdown();
}

} // namespace kali
