#include "nanobind.h"

#include "kali/core/struct.h"

KALI_PY_EXPORT(core_struct)
{
    using namespace kali;

    nb::class_<Struct, Object> struct_(m, "Struct");

    nb::kali_enum<Struct::Type>(struct_, "Type");
    nb::kali_enum<Struct::Flags>(struct_, "Flags").def_enum_operators();
    nb::kali_enum<Struct::ByteOrder>(struct_, "ByteOrder");

    nb::class_<Struct::Field>(struct_, "Field")
        .def_rw("name", &Struct::Field::name)
        .def_rw("type", &Struct::Field::type)
        .def_rw("flags", &Struct::Field::flags)
        .def_rw("size", &Struct::Field::size)
        .def_rw("offset", &Struct::Field::offset)
        .def_rw("default_value", &Struct::Field::default_value)
        .def("is_integer", &Struct::Field::is_integer)
        .def("is_unsigned", &Struct::Field::is_unsigned)
        .def("is_signed", &Struct::Field::is_signed)
        .def("is_float", &Struct::Field::is_float)
        .def(nb::self == nb::self)
        .def(nb::self != nb::self)
        .def("__repr__", &Struct::Field::to_string);

    struct_ //
        .def(nb::init<bool, Struct::ByteOrder>(), "pack"_a = false, "byte_order"_a = Struct::ByteOrder::host)
        .def("append", nb::overload_cast<Struct::Field>(&Struct::append), "field"_a, nb::rv_policy::reference)
        .def(
            "append",
            nb::overload_cast<std::string_view, Struct::Type, Struct::Flags, double, const Struct::Field::BlendList&>(
                &Struct::append
            ),
            "name"_a,
            "type"_a,
            "flags"_a = Struct::Flags::none,
            "default_value"_a = 0.0,
            "blend"_a = Struct::Field::BlendList(),
            nb::rv_policy::reference
        )
        .def("has_field", &Struct::has_field, "name"_a)
        .def("field", &Struct::field, "name"_a)
        .def(nb::self == nb::self)
        .def(nb::self != nb::self)
        .def_prop_ro("fields", &Struct::fields)
        .def_prop_ro("size", &Struct::size)
        .def_prop_ro("alignment", &Struct::alignment)
        .def_prop_ro("byte_order", &Struct::byte_order)
        .def_static("type_size", &Struct::type_size)
        .def_static("type_range", &Struct::type_range)
        .def_static("is_integer", &Struct::is_integer)
        .def_static("is_unsigned", &Struct::is_unsigned)
        .def_static("is_signed", &Struct::is_signed)
        .def_static("is_float", &Struct::is_float);

    nb::class_<StructConverter, Object>(m, "StructConverter")
        .def(
            "__init__",
            [](StructConverter* self, const Struct* src, const Struct* dst)
            { new (self) StructConverter(ref<const Struct>(src), ref<const Struct>(dst)); },
            "src"_a,
            "dst"_a
        )
        .def_prop_ro("src", &StructConverter::src)
        .def_prop_ro("dst", &StructConverter::dst)
        .def(
            "convert",
            [](StructConverter* self, nb::bytes input) -> nb::bytes
            {
                size_t count = input.size() / self->src()->size();
                std::string output(self->dst()->size() * count, '\0');
                self->convert(input.c_str(), output.data(), count);
                return nb::bytes(output.data(), output.size());
            },
            "input"_a
        );
}
