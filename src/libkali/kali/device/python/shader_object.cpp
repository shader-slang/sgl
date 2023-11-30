#include "nanobind.h"

#include "kali/device/shader_object.h"

KALI_PY_EXPORT(device_shader_object)
{
    using namespace kali;

    nb::class_<ShaderObject, Object>(m, "ShaderObject");
    nb::class_<TransientShaderObject, ShaderObject>(m, "TransientShaderObject");
    nb::class_<MutableShaderObject, ShaderObject>(m, "MutableShaderObject");
}
