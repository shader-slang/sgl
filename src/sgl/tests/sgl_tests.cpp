// SPDX-License-Identifier: Apache-2.0

#include "testing.h"
#include "sgl/sgl.h"
#include "sgl/core/object.h"
#include "sgl/core/logger.h"
#include "sgl/core/error.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

int main(int argc, char** argv)
{
    sgl::static_init();

    sgl::Logger::get().remove_all_outputs();
    sgl::Logger::get().add_debug_console_output();
    sgl::Logger::get().add_file_output("sgl_tests.log");

    /// Do not break debugger on exceptions when running tests.
    sgl::set_exception_diagnostics(sgl::ExceptionDiagnosticFlags::none);

    int result = 1;
    {
        sgl::testing::static_init();

        doctest::Context context(argc, argv);

        // Select specific test suite to run
        // context.setOption("-ts", "formats");
        // Report successful tests
        // context.setOption("success", true);

        result = context.run();

        sgl::testing::static_shutdown();
    }

#if SGL_ENABLE_OBJECT_TRACKING
    sgl::Logger::get().add_console_output();
    sgl::Object::report_alive_objects();
#endif

    sgl::static_shutdown();

    return result;
}