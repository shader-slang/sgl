#pragma once

#include "kali/error.h"

#include <utility>

namespace kali {

template<typename R, typename T>
R narrow_cast(T value)
{
    // TODO(@skallweit): add better error reporting with source_lcation?
    if (!std::in_range<R>(value))
        KALI_THROW(RuntimeError("narrow_cast failed"));
    return static_cast<R>(value);
}

} // namespace kali
