// SPDX-License-Identifier: Apache-2.0

#include "sgl.h"

#include "sgl/core/logger.h"
#include "sgl/core/platform.h"
#include "sgl/core/bitmap.h"
#include "sgl/core/format.h"
#include "sgl/core/thread.h"

#include "git_version.h"

static inline const char* git_version()
{
    static std::string str{
        fmt::format("commit: {} / branch: {}", GIT_VERSION_COMMIT, GIT_VERSION_BRANCH)
        + (GIT_VERSION_DIRTY ? " (local changes)" : "")};
    return str.c_str();
}

const char* SGL_GIT_VERSION = git_version();

namespace sgl {

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

} // namespace sgl
