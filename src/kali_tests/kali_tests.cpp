#include "testing.h"
#include "kali/object.h"
#include "kali/logger.h"
#include "kali/error.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

int main(int argc, char** argv)
{
    kali::Logger::global().remove_all_outputs();
    kali::Logger::global().add_debug_console_output();
    kali::Logger::global().add_file_output("kali_tests.log");

    /// Do not break debugger on exceptions when running tests.
    kali::set_exception_diagnostics(kali::ExceptionDiagnosticFlags::none);

    int result = 1;
    {
        doctest::Context context(argc, argv);

        // Select specific test suite to run
        // context.setOption("-ts", "formats");
        // Report successful tests
        // context.setOption("success", true);

        result = context.run();

        kali::testing::release_cached_devices();
    }

#if KALI_ENABLE_OBJECT_TRACKING
    kali::Logger::global().add_console_output();
    kali::Object::report_alive_objects();
#endif

    return result;
}