#pragma once

#include "kali/core/error.h"

#include <utility>

namespace kali {

template<typename R, typename T>
R narrow_cast(T value)
{
    if (!std::in_range<R>(value))
        KALI_THROW("narrow_cast failed");
    return static_cast<R>(value);
}

} // namespace kali
