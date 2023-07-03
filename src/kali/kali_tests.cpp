#include "testing.h"
#include "core/object.h"
#include "core/logger.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

int main(int argc, char** argv)
{
    KALI_UNUSED(argc);
    KALI_UNUSED(argv);

    kali::Logger::global().remove_all_outputs();
    kali::Logger::global().add_debug_console_output();
    kali::Logger::global().add_file_output("kali_tests.log");

    doctest::Context context;

    // context.setOption("success", true);
    // context.setOption("-ts", "formats");

    int result = context.run();

#if KALI_ENABLE_OBJECT_TRACKING
    kali::Logger::global().add_console_output();
    kali::Object::report_alive_objects();
#endif

    return result;
}
