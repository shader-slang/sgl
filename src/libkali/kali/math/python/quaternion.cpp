#include "nanobind.h"

#include "kali/math/quaternion.h"
#include "kali/core/traits.h"

#include <array>
#include <type_traits>

namespace kali::math {

template<typename T>
void bind_quaternion_type(nb::module_& m, const char* name)
{
    nb::class_<T> quat(m, name);
}

inline void bind_quaternion(nb::module_& m)
{
    bind_quaternion_type<quatf>(m, "quatf");

    // TODO add bindings
}

} // namespace kali::math

KALI_PY_EXPORT(math_quaternion)
{
    nb::module_ math = m.attr("math");

    kali::math::bind_quaternion(math);

    m.attr("quatf") = math.attr("quatf");
}
