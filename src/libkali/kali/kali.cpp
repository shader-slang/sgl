#include "kali.h"

#include "kali/core/logger.h"
#include "kali/core/platform.h"
#include "kali/core/bitmap.h"

namespace kali {

void static_init()
{
    Logger::static_init();
    platform_static_init();
    Bitmap::static_init();
}

void static_shutdown()
{
    Bitmap::static_shutdown();
    platform_static_shutdown();
    Logger::static_shutdown();
}

} // namespace kali
