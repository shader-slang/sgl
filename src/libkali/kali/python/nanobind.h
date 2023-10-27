#pragma once

#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/operators.h>
#include <nanobind/stl/array.h>
#include <nanobind/stl/bind_map.h>
#include <nanobind/stl/filesystem.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/map.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "kali/core/object.h"
#include "kali/core/enum.h"

namespace nb = nanobind;
using namespace nb::literals;

NAMESPACE_BEGIN(NB_NAMESPACE)
NAMESPACE_BEGIN(detail)

template<typename T>
struct type_caster<kali::ref<T>> {
    using Caster = make_caster<T>;
    static constexpr bool IsClass = true;
    NB_TYPE_CASTER(kali::ref<T>, Caster::Name);

    bool from_python(handle src, uint8_t flags, cleanup_list* cleanup) noexcept
    {
        Caster caster;
        if (!caster.from_python(src, flags, cleanup))
            return false;

        value = Value(caster.operator T*());
        return true;
    }

    static handle from_cpp(const kali::ref<T>& value, rv_policy policy, cleanup_list* cleanup) noexcept
    {
        return Caster::from_cpp(value.get(), policy, cleanup);
    }
};

NAMESPACE_END(detail)

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

NAMESPACE_END(NB_NAMESPACE)

#define KALI_PY_DECLARE(name) extern void kali_python_export_##name(nb::module_& m)
#define KALI_PY_EXPORT(name) void kali_python_export_##name([[maybe_unused]] ::nb::module_& m)
#define KALI_PY_IMPORT(name) kali_python_export_##name(m)
