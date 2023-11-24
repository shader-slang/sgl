#include "nanobind.h"
#include "kali/core/platform.h"
#include "kali/core/thread.h"

#include "kali/kali.h"
#include "kali/device/device.h"

#include <iostream>

KALI_PY_DECLARE(core_bitmap);
KALI_PY_DECLARE(core_input);
KALI_PY_DECLARE(core_logger);
KALI_PY_DECLARE(core_object);
KALI_PY_DECLARE(core_platform);
KALI_PY_DECLARE(core_struct);
KALI_PY_DECLARE(core_thread);
KALI_PY_DECLARE(core_timer);
KALI_PY_DECLARE(core_window);

KALI_PY_DECLARE(device_command);
KALI_PY_DECLARE(device_device_resource);
KALI_PY_DECLARE(device_device);
KALI_PY_DECLARE(device_fence);
KALI_PY_DECLARE(device_formats);
KALI_PY_DECLARE(device_kernel);
KALI_PY_DECLARE(device_query);
KALI_PY_DECLARE(device_reflection);
KALI_PY_DECLARE(device_resource);
KALI_PY_DECLARE(device_sampler);
KALI_PY_DECLARE(device_shader_cursor);
KALI_PY_DECLARE(device_shader_object);
KALI_PY_DECLARE(device_shader);
KALI_PY_DECLARE(device_swapchain);
KALI_PY_DECLARE(device_types);

KALI_PY_DECLARE(math_scalar);
KALI_PY_DECLARE(math_vector);
KALI_PY_DECLARE(math_matrix);
KALI_PY_DECLARE(math_quaternion);

KALI_PY_DECLARE(utils_tev);


NB_MODULE(kali_ext, m)
{
    kali::static_init();
    kali::platform::set_python_active(true);

    kali::Device::enable_agility_sdk();

    m.attr("KALI_VERSION_MAJOR") = KALI_VERSION_MAJOR;
    m.attr("KALI_VERSION_MINOR") = KALI_VERSION_MINOR;
    m.attr("KALI_VERSION_PATCH") = KALI_VERSION_PATCH;
    m.attr("KALI_VERSION") = KALI_VERSION;
    m.attr("git_version") = kali::git_version();

    KALI_PY_IMPORT(core_object);
    m.def_submodule("platform", "Platform module");
    KALI_PY_IMPORT(core_platform);
    m.def_submodule("thread", "Threading module");
    KALI_PY_IMPORT(core_thread);
    KALI_PY_IMPORT(core_input);
    KALI_PY_IMPORT(core_logger);
    KALI_PY_IMPORT(core_timer);
    KALI_PY_IMPORT(core_window);
    KALI_PY_IMPORT(core_struct);
    KALI_PY_IMPORT(core_bitmap);

    m.def_submodule("math", "Math module");
    KALI_PY_IMPORT(math_scalar);
    KALI_PY_IMPORT(math_vector);
    KALI_PY_IMPORT(math_matrix);
    KALI_PY_IMPORT(math_quaternion);

    KALI_PY_IMPORT(device_types);
    KALI_PY_IMPORT(device_formats);
    KALI_PY_IMPORT(device_device_resource);
    KALI_PY_IMPORT(device_resource);
    KALI_PY_IMPORT(device_sampler);
    KALI_PY_IMPORT(device_fence);
    KALI_PY_IMPORT(device_query);
    KALI_PY_IMPORT(device_reflection);
    KALI_PY_IMPORT(device_shader);
    KALI_PY_IMPORT(device_shader_object);
    KALI_PY_IMPORT(device_shader_cursor);
    KALI_PY_IMPORT(device_swapchain);
    KALI_PY_IMPORT(device_command);
    KALI_PY_IMPORT(device_kernel);
    KALI_PY_IMPORT(device_device);

    m.def_submodule("utils", "Utility module");
    KALI_PY_IMPORT(utils_tev);

    // Register a cleanup callback function.
    auto atexit = nb::module_::import_("atexit");
    atexit.attr("register")(nb::cpp_function(
        [&]()
        {
            // The GIL is hold when calling this callback.
            // We need to release it here in order to not deadlock
            // when increasing/decreasing the reference count of objects.
            // This mostly happens because we are waiting for tasks to finish in static_shutdown().
            nb::gil_scoped_release guard;
            kali::static_shutdown();
        }
    ));
}
