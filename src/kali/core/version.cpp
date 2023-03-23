#include "version.h"
#include "format.h"

#include "git_version.h"

namespace kali {

#define KALI_VERSION_MAJOR 0
#define KALI_VERSION_MINOR 1

Version& get_version()
{
    static std::string short_tag{fmt::format("kali {}.{}", KALI_VERSION_MAJOR, KALI_VERSION_MINOR)};
    static std::string long_tag{fmt::format(
        "kali {}.{} (commit: {}, branch: {}, dirty: {})",
        KALI_VERSION_MAJOR,
        KALI_VERSION_MINOR,
        GIT_VERSION_COMMIT,
        GIT_VERSION_BRANCH,
        GIT_VERSION_DIRTY ? "yes" : "no"
    )};

    static Version version{
        .major = KALI_VERSION_MAJOR,
        .minor = KALI_VERSION_MINOR,
        .git_commit = GIT_VERSION_COMMIT,
        .git_branch = GIT_VERSION_BRANCH,
        .git_dirty = GIT_VERSION_DIRTY,
        .short_tag = short_tag.c_str(),
        .long_tag = long_tag.c_str(),
    };

    return version;
}

} // namespace kali
