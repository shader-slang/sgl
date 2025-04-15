// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"
#include "sgl/core/platform.h"
#include "sgl/core/thread.h"

#include "sgl/sgl.h"
#include "sgl/device/device.h"

#include <iostream>

SGL_PY_DECLARE(app_app);

SGL_PY_DECLARE(core_bitmap);
SGL_PY_DECLARE(core_crypto);
SGL_PY_DECLARE(core_data_type);
SGL_PY_DECLARE(core_input);
SGL_PY_DECLARE(core_logger);
SGL_PY_DECLARE(core_object);
SGL_PY_DECLARE(core_platform);
SGL_PY_DECLARE(core_struct);
SGL_PY_DECLARE(core_thread);
SGL_PY_DECLARE(core_timer);
SGL_PY_DECLARE(core_window);

SGL_PY_DECLARE(device_buffer_cursor);
SGL_PY_DECLARE(device_command);
SGL_PY_DECLARE(device_coopvec);
SGL_PY_DECLARE(device_device_resource);
SGL_PY_DECLARE(device_device);
SGL_PY_DECLARE(device_fence);
SGL_PY_DECLARE(device_formats);
SGL_PY_DECLARE(device_framebuffer);
SGL_PY_DECLARE(device_input_layout);
SGL_PY_DECLARE(device_kernel);
SGL_PY_DECLARE(device_native_handle);
SGL_PY_DECLARE(device_pipeline);
SGL_PY_DECLARE(device_query);
SGL_PY_DECLARE(device_raytracing);
SGL_PY_DECLARE(device_reflection);
SGL_PY_DECLARE(device_resource);
SGL_PY_DECLARE(device_sampler);
SGL_PY_DECLARE(device_shader_cursor);
SGL_PY_DECLARE(device_shader_object);
SGL_PY_DECLARE(device_shader);
SGL_PY_DECLARE(device_surface);
SGL_PY_DECLARE(device_types);

SGL_PY_DECLARE(math_scalar);
SGL_PY_DECLARE(math_vector);
SGL_PY_DECLARE(math_matrix);
SGL_PY_DECLARE(math_quaternion);

SGL_PY_DECLARE(ui);
SGL_PY_DECLARE(ui_widgets);

SGL_PY_DECLARE(utils_renderdoc);
SGL_PY_DECLARE(utils_slangpy);
SGL_PY_DECLARE(utils_slangpy_buffer);
SGL_PY_DECLARE(utils_slangpy_function);
SGL_PY_DECLARE(utils_slangpy_resources);
SGL_PY_DECLARE(utils_slangpy_tensor);
SGL_PY_DECLARE(utils_slangpy_value);
SGL_PY_DECLARE(utils_tev);
SGL_PY_DECLARE(utils_texture_loader);


NB_MODULE(sgl_ext, m_)
{
    SGL_UNUSED(m_);

#if !SGL_DEBUG
    nb::set_leak_warnings(false);
#endif

    nb::module_ m = nb::module_::import_("sgl");
    m.attr("__doc__") = "sgl";

    sgl::static_init();
    sgl::platform::set_python_active(true);

    sgl::Device::enable_agility_sdk();

    m.attr("SGL_VERSION_MAJOR") = SGL_VERSION_MAJOR;
    m.attr("SGL_VERSION_MINOR") = SGL_VERSION_MINOR;
    m.attr("SGL_VERSION_PATCH") = SGL_VERSION_PATCH;
    m.attr("SGL_VERSION") = SGL_VERSION;
    m.attr("SGL_GIT_VERSION") = SGL_GIT_VERSION;

    SGL_PY_IMPORT(core_object);
    m.def_submodule("platform", "Platform module");
    SGL_PY_IMPORT(core_platform);
    m.def_submodule("thread", "Threading module");
    SGL_PY_IMPORT(core_thread);
    SGL_PY_IMPORT(core_input);
    SGL_PY_IMPORT(core_logger);
    SGL_PY_IMPORT(core_timer);
    SGL_PY_IMPORT(core_window);
    SGL_PY_IMPORT(core_struct);
    SGL_PY_IMPORT(core_bitmap);
    SGL_PY_IMPORT(core_crypto);
    SGL_PY_IMPORT(core_data_type);

    m.def_submodule("math", "Math module");
    SGL_PY_IMPORT(math_scalar);
    SGL_PY_IMPORT(math_vector);
    SGL_PY_IMPORT(math_matrix);
    SGL_PY_IMPORT(math_quaternion);

    SGL_PY_IMPORT(device_native_handle);
    SGL_PY_IMPORT(device_types);
    SGL_PY_IMPORT(device_formats);
    SGL_PY_IMPORT(device_device_resource);
    SGL_PY_IMPORT(device_resource);
    SGL_PY_IMPORT(device_sampler);
    SGL_PY_IMPORT(device_fence);
    SGL_PY_IMPORT(device_query);
    SGL_PY_IMPORT(device_input_layout);
    SGL_PY_IMPORT(device_pipeline);
    SGL_PY_IMPORT(device_raytracing);
    SGL_PY_IMPORT(device_reflection);
    SGL_PY_IMPORT(device_shader);
    SGL_PY_IMPORT(device_buffer_cursor);
    SGL_PY_IMPORT(device_shader_object);
    SGL_PY_IMPORT(device_shader_cursor);
    SGL_PY_IMPORT(device_surface);
    SGL_PY_IMPORT(device_command);
    SGL_PY_IMPORT(device_coopvec);
    SGL_PY_IMPORT(device_kernel);
    SGL_PY_IMPORT(device_device);

    m.def_submodule("ui", "UI module");
    SGL_PY_IMPORT(ui);
    SGL_PY_IMPORT(ui_widgets);

    m.def_submodule("renderdoc", "RenderDoc module");
    SGL_PY_IMPORT(utils_renderdoc);

    m.def_submodule("slangpy", "SlangPy module");
    SGL_PY_IMPORT(utils_slangpy);
    SGL_PY_IMPORT(utils_slangpy_buffer);
    SGL_PY_IMPORT(utils_slangpy_function);
    SGL_PY_IMPORT(utils_slangpy_resources);
    SGL_PY_IMPORT(utils_slangpy_tensor);
    SGL_PY_IMPORT(utils_slangpy_value);

    m.def_submodule("tev", "tev image viewer module");
    SGL_PY_IMPORT(utils_tev);
    SGL_PY_IMPORT(utils_texture_loader);

    SGL_PY_IMPORT(app_app);

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
                sgl::thread::wait_for_tasks();

                // Close all devices before shutting down.
                // This is mostly convenience for the user, as the devices will
                // be closed automatically when the program exits.
                sgl::Device::close_all_devices();
            }
        }
    ));

    // Cleanup when the last sgl::Object is garbage collected.
    nb::weakref(
        m.attr("Object"),
        nb::cpp_function(
            [](nb::handle weakref)
            {
                sgl::static_shutdown();
                weakref.dec_ref();
            }
        )
    ).release();
}
