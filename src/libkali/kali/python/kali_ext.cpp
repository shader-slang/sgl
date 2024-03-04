// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"
#include "kali/core/platform.h"
#include "kali/core/thread.h"

#include "kali/kali.h"
#include "kali/device/device.h"

#include <iostream>

KALI_PY_DECLARE(core_bitmap);
KALI_PY_DECLARE(core_crypto);
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
KALI_PY_DECLARE(device_framebuffer);
KALI_PY_DECLARE(device_input_layout);
KALI_PY_DECLARE(device_kernel);
KALI_PY_DECLARE(device_memory_heap);
KALI_PY_DECLARE(device_pipeline);
KALI_PY_DECLARE(device_query);
KALI_PY_DECLARE(device_raytracing);
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

KALI_PY_DECLARE(ui);
KALI_PY_DECLARE(ui_widgets);

KALI_PY_DECLARE(utils_tev);


NB_MODULE(kali_ext, m_)
{
    KALI_UNUSED(m_);

#if !KALI_DEBUG
    nb::set_leak_warnings(false);
#endif

    nb::module_ m = nb::module_::import_("kali");
    m.attr("__doc__") = "kali";

    kali::static_init();
    kali::platform::set_python_active(true);

    kali::Device::enable_agility_sdk();

    m.attr("KALI_VERSION_MAJOR") = KALI_VERSION_MAJOR;
    m.attr("KALI_VERSION_MINOR") = KALI_VERSION_MINOR;
    m.attr("KALI_VERSION_PATCH") = KALI_VERSION_PATCH;
    m.attr("KALI_VERSION") = KALI_VERSION;
    m.attr("KALI_GIT_VERSION") = KALI_GIT_VERSION;

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
    KALI_PY_IMPORT(core_crypto);

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
    KALI_PY_IMPORT(device_input_layout);
    KALI_PY_IMPORT(device_pipeline);
    KALI_PY_IMPORT(device_raytracing);
    KALI_PY_IMPORT(device_reflection);
    KALI_PY_IMPORT(device_shader);
    KALI_PY_IMPORT(device_shader_object);
    KALI_PY_IMPORT(device_shader_cursor);
    KALI_PY_IMPORT(device_framebuffer);
    KALI_PY_IMPORT(device_swapchain);
    KALI_PY_IMPORT(device_command);
    KALI_PY_IMPORT(device_kernel);
    KALI_PY_IMPORT(device_memory_heap);
    KALI_PY_IMPORT(device_device);

    m.def_submodule("ui", "UI module");
    KALI_PY_IMPORT(ui);
    KALI_PY_IMPORT(ui_widgets);

    m.def_submodule("utils", "Utility module");
    KALI_PY_IMPORT(utils_tev);

    // Wait for all tasks to finish before shutting down.
    auto atexit = nb::module_::import_("atexit");
    atexit.attr("register")(nb::cpp_function(
        []()
        {
            {
                // While waiting for tasks to finish, we block the main thread
                // while holding the GIL. This makes it impossible for other
                // threads to get hold of the GIL to acquire/release reference
                // counted objects.
                nb::gil_scoped_release guard;
                kali::thread::wait_for_tasks();
            }
        }
    ));

    // Cleanup when the last kali::Object is garbage collected.
    nb::weakref(
        m.attr("Object"),
        nb::cpp_function(
            [](nb::handle weakref)
            {
                kali::static_shutdown();
                weakref.dec_ref();
            }
        )
    ).release();
}
