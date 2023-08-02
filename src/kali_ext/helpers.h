#pragma once

#include "kali/core/enum.h"

#include <nanobind/nanobind.h>

namespace nanobind {

template<typename T>
class kali_enum : public enum_<T> {
public:
    static_assert(::kali::has_enum_info_v<T>, "nanobind::kali_enum<> requires an enumeration type with infos!");

    using Base = enum_<T>;

    template<typename... Extra>
    NB_INLINE kali_enum(handle scope, const char* name, const Extra&... extra)
        : Base(scope, name, extra...)
    {
        for (const auto& item : ::kali::EnumInfo<T>::items())
            Base::value(item.second.c_str(), item.first);
    }
};

} // namespace nanobind
