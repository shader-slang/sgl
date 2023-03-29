#pragma once

namespace kali {

template<typename To, typename From>
To narrow_cast(From from)
{
    // TODO(@skallweit): use std::narrow_cast when it's available or std::in_range
    return static_cast<To>(from);
}

} // namespace kali
