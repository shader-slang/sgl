#include "kali.h"

#include "kali/core/logger.h"
#include "kali/core/platform.h"
#include "kali/core/bitmap.h"
#include "kali/core/format.h"

#include "git_version.h"


namespace kali {

const char* git_version()
{
    static std::string str{
        fmt::format("commit: {} / branch: {}", GIT_VERSION_COMMIT, GIT_VERSION_BRANCH)
        + (GIT_VERSION_DIRTY ? " (local changes)" : "")};
    return str.c_str();
}

void static_init()
{
    Logger::static_init();
    platform::static_init();
    Bitmap::static_init();
}

void static_shutdown()
{
    Bitmap::static_shutdown();
    platform::static_shutdown();
    Logger::static_shutdown();
}

} // namespace kali
