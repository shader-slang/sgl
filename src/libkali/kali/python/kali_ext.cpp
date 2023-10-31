#include "nanobind.h"

#include "kali/kali.h"
#include "kali/device/device.h"

KALI_PY_DECLARE(core_input);
KALI_PY_DECLARE(core_logger);
KALI_PY_DECLARE(core_object);
KALI_PY_DECLARE(core_timer);
KALI_PY_DECLARE(core_version);
KALI_PY_DECLARE(core_window);

KALI_PY_DECLARE(device_device);
KALI_PY_DECLARE(device_formats);
KALI_PY_DECLARE(device_reflection);
KALI_PY_DECLARE(device_resource);
KALI_PY_DECLARE(device_sampler);
KALI_PY_DECLARE(device_shader);
KALI_PY_DECLARE(device_swapchain);
KALI_PY_DECLARE(device_types);

KALI_PY_DECLARE(math_scalar);
KALI_PY_DECLARE(math_vector);
KALI_PY_DECLARE(math_matrix);
KALI_PY_DECLARE(math_quaternion);


NB_MODULE(kali_ext, m)
{
    kali::static_init();

    kali::Device::enable_agility_sdk();

    KALI_PY_IMPORT(core_object);
    KALI_PY_IMPORT(core_input);
    KALI_PY_IMPORT(core_logger);
    KALI_PY_IMPORT(core_timer);
    KALI_PY_IMPORT(core_version);
    KALI_PY_IMPORT(core_window);

    m.def_submodule("math", "Math module");
    KALI_PY_IMPORT(math_scalar);
    KALI_PY_IMPORT(math_vector);
    KALI_PY_IMPORT(math_matrix);
    KALI_PY_IMPORT(math_quaternion);

    KALI_PY_IMPORT(device_types);
    KALI_PY_IMPORT(device_formats);
    KALI_PY_IMPORT(device_resource);
    KALI_PY_IMPORT(device_sampler);
    KALI_PY_IMPORT(device_reflection);
    KALI_PY_IMPORT(device_shader);
    KALI_PY_IMPORT(device_swapchain);
    KALI_PY_IMPORT(device_device);

    // Register a cleanup callback function.
    auto atexit = nb::module_::import_("atexit");
    atexit.attr("register")(nb::cpp_function([]() { kali::static_shutdown(); }));
}
