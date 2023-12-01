#pragma once

#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/operators.h>
#include <nanobind/trampoline.h>
#include <nanobind/stl/array.h>
#include <nanobind/stl/bind_map.h>
#include <nanobind/stl/filesystem.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/map.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/unique_ptr.h>

#include "kali/core/object.h"
#include "kali/core/enum.h"
#include "kali/core/type_utils.h"
#include "kali/core/struct.h"

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

template<typename T>
class kali_enum_flags : public kali_enum<T> {
public:
    static_assert(
        std::is_same_v<T, decltype(std::declval<T>() & std::declval<T>())>,
        "nanobind::kali_enum_flags<> requires an enumeration type with bitwise operators!"
    );

    using Base = kali_enum<T>;

    template<typename... Extra>
    NB_INLINE kali_enum_flags(handle scope, const char* name, const Extra&... extra)
        : Base(scope, name, extra...)
    {
        for (const auto& item : ::kali::EnumInfo<T>::items())
            Base::value(item.second.c_str(), item.first);

        Base::def("__and__", [](T value1, T value2) { return value1 & value2; });
        Base::def("__or__", [](T value1, T value2) { return value1 | value2; });
    }
};

NAMESPACE_END(NB_NAMESPACE)

namespace kali {

template<typename... Args>
size_t is_ndarray_contiguous(const nb::ndarray<Args...>& array)
{
    if (array.ndim() == 0)
        return false;
    size_t prod = 1;
    for (size_t i = array.ndim() - 1;;) {
        if (array.stride(i) != narrow_cast<int64_t>(prod))
            return false;
        prod *= array.shape(i);
        if (i == 0)
            break;
        --i;
    }
    return true;
}

inline std::optional<Struct::Type> dtype_to_struct_type(nb::dlpack::dtype dtype)
{
    switch (dtype.code) {
    case uint8_t(nb::dlpack::dtype_code::Int):
        switch (dtype.bits) {
        case 8:
            return Struct::Type::int8;
        case 16:
            return Struct::Type::int16;
        case 32:
            return Struct::Type::int32;
        case 64:
            return Struct::Type::int64;
        }
        break;
    case uint8_t(nb::dlpack::dtype_code::UInt):
        switch (dtype.bits) {
        case 8:
            return Struct::Type::uint8;
        case 16:
            return Struct::Type::uint16;
        case 32:
            return Struct::Type::uint32;
        case 64:
            return Struct::Type::uint64;
        }
    case uint8_t(nb::dlpack::dtype_code::Float):
        switch (dtype.bits) {
        case 16:
            return Struct::Type::float16;
        case 32:
            return Struct::Type::float32;
        case 64:
            return Struct::Type::float64;
        }
    }
    return {};
}

} // namespace kali

#define KALI_PY_DECLARE(name) extern void kali_python_export_##name(nb::module_& m)
#define KALI_PY_EXPORT(name) void kali_python_export_##name([[maybe_unused]] ::nb::module_& m)
#define KALI_PY_IMPORT(name) kali_python_export_##name(m)


#define def_enum_operators()                                                                                           \
    def(nb::self == nb::self)                                                                                          \
        .def(nb::self != nb::self)                                                                                     \
        .def(nb::self | nb::self)                                                                                      \
        .def(nb::self |= nb::self)                                                                                     \
        .def(nb::self& nb::self)                                                                                       \
        .def(nb::self &= nb::self)                                                                                     \
        .def(~nb::self)
