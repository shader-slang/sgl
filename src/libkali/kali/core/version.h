#pragma once

#include "kali/core/macros.h"

#include <cstdint>

namespace kali {

struct Version {
    uint32_t major;
    uint32_t minor;

    const char* git_commit;
    const char* git_branch;
    bool git_dirty;

    const char* short_tag;
    const char* long_tag;
};

KALI_API Version& get_version();

} // namespace kali
