#include "kali.h"

#include "kali/core/logger.h"

namespace kali {

void static_init()
{
    Logger::static_init();
}

void static_shutdown()
{
    Logger::static_shutdown();
}

} // namespace kali
