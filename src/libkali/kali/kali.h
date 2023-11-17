#include "kali/core/macros.h"

#define KALI_VERSION_MAJOR 0
#define KALI_VERSION_MINOR 1
#define KALI_VERSION_PATCH 0

#define KALI_VERSION                                                                                                   \
    KALI_TO_STRING(KALI_VERSION_MAJOR) "." KALI_TO_STRING(KALI_VERSION_MINOR) "." KALI_TO_STRING(KALI_VERSION_PATCH)

namespace kali {

KALI_API const char* git_version();

KALI_API void static_init();
KALI_API void static_shutdown();

} // namespace kali
