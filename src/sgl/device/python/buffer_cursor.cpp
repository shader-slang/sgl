// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/resource.h"

#include "sgl/device/buffer_cursor.h"

#include "sgl/device/python/cursor_shared.h"

#include "sgl/core/string.h"

namespace sgl {
namespace detail {

    template<typename T>
    concept IsSpecializationOfVector = requires {
        T::dimension;
        typename T::value_type;
    };
    template<typename T>
    concept IsSpecializationOfMatrix = requires {
        T::rows;
        T::cols;
        typename T::value_type;
    };

    template<typename ValType>
    inline nb::object _read_scalar(const BufferElementCursor& self)
    {
        ValType res;
        self.get(res);
        return nb::cast(res);
    }

    template<typename ValType>
    inline void _write_scalar(BufferElementCursor& self, nb::object nbval)
    {
        auto val = nb::cast<ValType>(nbval);
        self.set(val);
    }

    template<typename ValType>
        requires IsSpecializationOfVector<ValType>
    inline nb::object _read_vector(const BufferElementCursor& self)
    {
        ValType res;
        self.get(res);
        return nb::cast(res);
    }

    template<typename ValType>
        requires IsSpecializationOfVector<ValType>
    inline void _write_vector(BufferElementCursor& self, nb::object nbval)
    {
        if (nb::isinstance<ValType>(nbval)) {
            auto val = nb::cast<ValType>(nbval);
            self.set(val);
        } else if (nb::isinstance<nb::ndarray<nb::numpy>>(nbval)) {
            auto nbarray = nb::cast<nb::ndarray<nb::numpy>>(nbval);
            SGL_CHECK(nbarray.ndim() == 1 || nbarray.ndim() == 2, "numpy array must have 1 or 2 dimensions.");
            size_t dimension = 1;
            for (size_t i = 0; i < nbarray.ndim(); ++i)
                dimension *= nbarray.shape(i);
            SGL_CHECK(dimension == ValType::dimension, "numpy array has wrong dimension.");
            SGL_CHECK(nbarray.nbytes() == sizeof(ValType), "numpy array has wrong size.");
            auto val = *reinterpret_cast<const ValType*>(nbarray.data());
            self.set(val);
        } else if (nb::isinstance<nb::sequence>(nbval)) {
            auto seq = nb::cast<nb::sequence>(nbval);
            SGL_CHECK(nb::len(seq) == ValType::dimension, "sequence has wrong dimension.");
            ValType val;
            for (int i = 0; i < ValType::dimension; i++) {
                val[i] = nb::cast<typename ValType::value_type>(seq[i]);
            }
            self.set(val);
        } else {
            SGL_THROW("Expected numpy array or vector");
        }
    }

    template<typename ValType>
        requires IsSpecializationOfMatrix<ValType>
    inline nb::object _read_matrix(const BufferElementCursor& self)
    {
        ValType res;
        self.get(res);
        return nb::cast(res);
    }

    template<typename ValType>
        requires IsSpecializationOfMatrix<ValType>
    inline void _write_matrix(BufferElementCursor& self, nb::object nbval)
    {
        if (nb::isinstance<ValType>(nbval)) {
            auto val = nb::cast<ValType>(nbval);
            self.set(val);
        } else if (nb::isinstance<nb::ndarray<nb::numpy>>(nbval)) {
            auto nbarray = nb::cast<nb::ndarray<nb::numpy>>(nbval);
            SGL_CHECK(nbarray.ndim() == 2, "numpy array must have 2 dimensions.");
            SGL_CHECK(narrow_cast<int>(nbarray.shape(0)) == ValType::rows, "numpy array has wrong number of rows.");
            SGL_CHECK(narrow_cast<int>(nbarray.shape(1)) == ValType::cols, "numpy array has wrong number of cols.");
            SGL_CHECK(nbarray.nbytes() == sizeof(ValType), "numpy array has wrong size.");
            auto val = *reinterpret_cast<const ValType*>(nbarray.data());
            self.set(val);
        } else {
            SGL_THROW("Expected numpy array or matrix");
        }
    }

#define scalar_case(c_type, scalar_type)                                                                               \
    read_scalar[(int)TypeReflection::ScalarType::scalar_type]                                                          \
        = [](const BufferElementCursor& self) { return _read_scalar<c_type>(self); };                                  \
    write_scalar[(int)TypeReflection::ScalarType::scalar_type]                                                         \
        = [](BufferElementCursor& self, nb::object nbval) { _write_scalar<c_type>(self, nbval); };

#define vector_case(c_type, scalar_type)                                                                               \
    read_vector[(int)TypeReflection::ScalarType::scalar_type][c_type::dimension]                                       \
        = [](const BufferElementCursor& self) { return _read_vector<c_type>(self); };                                  \
    write_vector[(int)TypeReflection::ScalarType::scalar_type][c_type::dimension]                                      \
        = [](BufferElementCursor& self, nb::object nbval) { _write_vector<c_type>(self, nbval); };

#define matrix_case(c_type, scalar_type)                                                                               \
    read_matrix[(int)TypeReflection::ScalarType::scalar_type][c_type::rows][c_type::cols]                              \
        = [](const BufferElementCursor& self) { return _read_matrix<c_type>(self); };                                  \
    write_matrix[(int)TypeReflection::ScalarType::scalar_type][c_type::rows][c_type::cols]                             \
        = [](BufferElementCursor& self, nb::object nbval) { _write_matrix<c_type>(self, nbval); };

    struct Converters {
        std::function<nb::object(const BufferElementCursor&)> read_scalar[(int)TypeReflection::ScalarType::COUNT];
        std::function<nb::object(const BufferElementCursor&)> read_vector[(int)TypeReflection::ScalarType::COUNT][5];
        std::function<nb::object(const BufferElementCursor&)> read_matrix[(int)TypeReflection::ScalarType::COUNT][5][5];
        std::function<void(BufferElementCursor&, nb::object)> write_scalar[(int)TypeReflection::ScalarType::COUNT];
        std::function<void(BufferElementCursor&, nb::object)> write_vector[(int)TypeReflection::ScalarType::COUNT][5];
        std::function<void(BufferElementCursor&, nb::object)> write_matrix[(int)TypeReflection::ScalarType::COUNT][5]
                                                                          [5];

        Converters()
        {
            auto read_err_func = [](const BufferElementCursor&)
            {
                if (true) { // avoid 'unreachable code' warnings
                    SGL_THROW("Unsupported element type");
                }
                return nb::none();
            };
            auto write_err_func = [](const BufferElementCursor&, nb::object) { SGL_THROW("Unsupported element type"); };

            for (int i = 0; i < (int)TypeReflection::ScalarType::COUNT; i++) {
                read_scalar[i] = read_err_func;
                write_scalar[i] = write_err_func;
                for (int j = 0; j < 5; ++j) {
                    read_vector[i][j] = read_err_func;
                    write_vector[i][j] = write_err_func;
                    for (int k = 0; k < 5; ++k) {
                        read_matrix[i][j][k] = read_err_func;
                        write_matrix[i][j][k] = write_err_func;
                    }
                }
            }

            scalar_case(bool, bool_);
            scalar_case(int8_t, int8);
            scalar_case(uint8_t, uint8);
            scalar_case(int16_t, int16);
            scalar_case(uint16_t, uint16);
            scalar_case(int32_t, int32);
            scalar_case(uint32_t, uint32);
            scalar_case(int64_t, int64);
            scalar_case(uint64_t, uint64);
            scalar_case(float16_t, float16);
            scalar_case(float, float32);
            scalar_case(double, float64);
            scalar_case(intptr_t, intptr);
            scalar_case(uintptr_t, uintptr);

            vector_case(bool1, bool_);
            vector_case(float1, float32);
            vector_case(int1, int32);
            vector_case(uint1, uint32);
            vector_case(bool2, bool_);
            vector_case(float2, float32);
            vector_case(int2, int32);
            vector_case(uint2, uint32);
            vector_case(bool3, bool_);
            vector_case(float3, float32);
            vector_case(int3, int32);
            vector_case(uint3, uint32);
            vector_case(bool4, bool_);
            vector_case(float4, float32);
            vector_case(int4, int32);
            vector_case(uint4, uint32);

            matrix_case(float2x2, float32);
            matrix_case(float3x3, float32);
            matrix_case(float2x4, float32);
            matrix_case(float3x4, float32);
            matrix_case(float4x4, float32);
        }
    };
    static Converters _conv;

    nb::object read(const BufferElementCursor& self)
    {
        if (!self.is_valid())
            return nb::none();
        auto type = self.type();
        switch (type->kind()) {
        case TypeReflection::Kind::scalar: {
            return detail::_conv.read_scalar[(int)type->scalar_type()](self);
        }
        case TypeReflection::Kind::vector: {
            return detail::_conv.read_vector[(int)type->scalar_type()][type->col_count()](self);
        }
        case TypeReflection::Kind::matrix: {
            return detail::_conv.read_matrix[(int)type->scalar_type()][type->row_count()][type->col_count()](self);
        }
        case TypeReflection::Kind::struct_: {
            nb::dict res;
            for (uint32_t i = 0; i < type->field_count(); i++) {
                auto field = type->get_field_by_index(i);
                res[field->name()] = read(self[field->name()]);
            }
            return res;
        }
        case TypeReflection::Kind::array: {
            nb::list res;
            for (uint32_t i = 0; i < type->element_count(); i++) {
                res.append(read(self[i]));
            }
            return res;
        }
        default:
            break;
        }
        SGL_THROW("Unsupported element type");
    }

    void write(BufferElementCursor& self, nb::object nbval)
    {
        if (!self.is_valid())
            return;

        auto type = self.type();
        switch (type->kind()) {
        case TypeReflection::Kind::scalar: {
            return detail::_conv.write_scalar[(int)type->scalar_type()](self, nbval);
        }
        case TypeReflection::Kind::vector: {
            return detail::_conv.write_vector[(int)type->scalar_type()][type->col_count()](self, nbval);
        }
        case TypeReflection::Kind::matrix: {
            return detail::_conv.write_matrix[(int)type->scalar_type()][type->row_count()][type->col_count()](
                self,
                nbval
            );
        }
        case TypeReflection::Kind::struct_: {
            if (nb::isinstance<nb::dict>(nbval)) {
                auto dict = nb::cast<nb::dict>(nbval);
                for (uint32_t i = 0; i < type->field_count(); i++) {
                    auto field = type->get_field_by_index(i);
                    auto child = self[field->name()];
                    write(child, dict[field->name()]);
                }
                return;
            } else {
                SGL_THROW("Expected dict");
            }
        }
        case TypeReflection::Kind::array: {
            if (nb::isinstance<nb::ndarray<nb::numpy>>(nbval)) {
                // TODO: Should be able to do better job of interpreting nb array values by reading
                // data type and extracting individual elements.
                auto nbarray = nb::cast<nb::ndarray<nb::numpy>>(nbval);
                SGL_CHECK(nbarray.ndim() == 1, "numpy array must have 1 dimension.");
                SGL_CHECK(nbarray.shape(0) == type->element_count(), "numpy array is the wrong length.");
                self._set_array(
                    nbarray.data(),
                    nbarray.nbytes(),
                    type->element_type()->scalar_type(),
                    narrow_cast<int>(nbarray.shape(0))
                );
                return;
            } else if (nb::isinstance<nb::list>(nbval)) {
                auto list = nb::cast<nb::list>(nbval);
                for (uint32_t i = 0; i < type->element_count(); i++) {
                    auto child = self[i];
                    write(child, list[i]);
                }
                return;
            } else {
                SGL_THROW("Expected list");
            }
        }
        default:
            break;
        }
        SGL_THROW("Unsupported element type");
    }

} // namespace detail
} // namespace sgl

SGL_PY_EXPORT(device_buffer_cursor)
{
    using namespace sgl;

    nb::class_<BufferElementCursor> buffer_element_cursor(m, "BufferElementCursor", D_NA(BufferElementCursor));

    buffer_element_cursor //
        .def_prop_ro("_offset", &BufferElementCursor::offset, D_NA(BufferElementCursor, offset))
        .def(
            "set_data",
            [](BufferElementCursor& self, nb::ndarray<nb::device::cpu> data)
            {
                SGL_CHECK(is_ndarray_contiguous(data), "data is not contiguous");
                self.set_data(data.data(), data.nbytes());
            },
            "data"_a,
            D_NA(BufferElementCursor, set_data)
        )
        .def(
            "__dir__",
            [](const BufferElementCursor& self)
            {
                SGL_UNUSED(self);
                std::vector<std::string> attributes;
                attributes.push_back("_offset");
                attributes.push_back("_type");
                attributes.push_back("_type_layout");
                if (self.type()->kind() == TypeReflection::Kind::struct_) {
                    for (uint32_t i = 0; i < self.type()->field_count(); i++) {
                        auto field = self.type()->get_field_by_index(i);
                        attributes.push_back(field->name());
                    }
                }
                return attributes;
            }
        )
        .def(
            "__repr__",
            [](const BufferElementCursor& self)
            {
                auto val = nb::cast<std::string>(detail::read(self).attr("__repr__")());
                if (self.is_valid())
                    val += fmt::format(" [{}]", self.type()->full_name());
                return val;
            }
        );

    bind_traversable_cursor(buffer_element_cursor);

    buffer_element_cursor //
        .def(
            "__setattr__",
            [](BufferElementCursor& self, std::string_view name, nb::object nbval)
            {
                auto child = self[name];
                detail::write(child, nbval);
            },
            "name"_a,
            "val"_a,
            D_NA(BufferElementCursor, write)
        );

    buffer_element_cursor //
        .def(
            "__setitem__",
            [](BufferElementCursor& self, std::string_view name, nb::object nbval)
            {
                auto child = self[name];
                detail::write(child, nbval);
            },
            "index"_a,
            "val"_a,
            D_NA(BufferElementCursor, write)
        );

    buffer_element_cursor //
        .def(
            "__setitem__",
            [](BufferElementCursor& self, int index, nb::object nbval)
            {
                auto child = self[index];
                detail::write(child, nbval);
            },
            "index"_a,
            "val"_a,
            D_NA(BufferElementCursor, write)
        );

    buffer_element_cursor //
        .def(
            "write",
            [](BufferElementCursor& self, nb::object nbval) { detail::write(self, nbval); },
            "val"_a,
            D_NA(BufferElementCursor, write)
        );

    buffer_element_cursor //
        .def(
            "read",
            [](BufferElementCursor& self) { return detail::read(self); },
            D_NA(BufferElementCursor, read)
        );

    nb::class_<BufferCursor, Object>(m, "BufferCursor", D_NA(BufferCursor)) //
        .def(
            nb::init<ref<TypeLayoutReflection>, size_t>(),
            "element_layout"_a,
            "size"_a,
            D_NA(BufferCursor, BufferCursor)
        )
        .def(
            nb::init<ref<TypeLayoutReflection>, ref<Buffer>>(),
            "element_layout"_a,
            "buffer_resource"_a,
            D_NA(BufferCursor, BufferCursor)
        )
        .def_prop_ro("element_type_layout", &BufferCursor::element_type_layout, D_NA(BufferCursor, type_layout))
        .def_prop_ro("element_type", &BufferCursor::element_type, D_NA(BufferCursor, type))
        .def("find_element", &BufferCursor::find_element, "index"_a, D_NA(BufferCursor, find_element))
        .def_prop_ro("element_count", &BufferCursor::element_count, D_NA(BufferCursor, element_count))
        .def_prop_ro("element_size", &BufferCursor::element_size, D_NA(BufferCursor, element_size))
        .def_prop_ro("size", &BufferCursor::size, D_NA(BufferCursor, size))
        .def_prop_ro("is_loaded", &BufferCursor::is_loaded, D_NA(BufferCursor, is_loaded))
        .def("load", &BufferCursor::load, D_NA(BufferCursor, load))
        .def("apply", &BufferCursor::apply, D_NA(BufferCursor, apply))
        .def_prop_ro("resource", &BufferCursor::resource, D_NA(BufferCursor, resource))
        .def("__getitem__", [](BufferCursor& self, int index) { return self[index]; })
        .def("__len__", [](BufferCursor& self) { return self.element_count(); })
        .def(
            "to_numpy",
            [](BufferCursor& self)
            {
                size_t data_size = self.size();
                void* data = new uint8_t[data_size];
                self.read_data(0, data, data_size);
                nb::capsule owner(data, [](void* p) noexcept { delete[] reinterpret_cast<uint8_t*>(p); });
                size_t shape[1] = {data_size};
                return nb::ndarray<
                    nb::numpy>(data, 1, shape, owner, nullptr, nb::dtype<uint8_t>(), nb::device::cpu::value);
            }
        )
        .def(
            "from_numpy",
            [](BufferCursor& self, nb::ndarray<nb::numpy> data)
            {
                SGL_CHECK(is_ndarray_contiguous(data), "numpy array is not contiguous");

                size_t buffer_size = self.size();
                size_t data_size = data.nbytes();
                SGL_CHECK(
                    data_size <= buffer_size,
                    "numpy array is larger than the buffer ({} > {})",
                    data_size,
                    buffer_size
                );

                self.write_data(0, data.data(), data_size);
            },
            "data"_a
        )
        .def(
            "__dir__",
            [](BufferCursor& self)
            {
                size_t first = 0;
                std::vector<std::string> attributes;
                attributes.push_back("element_type_layout");
                attributes.push_back("element_type");
                attributes.push_back("size");
                attributes.push_back("element_count");
                attributes.push_back("element_size");
                attributes.push_back("is_loaded");
                attributes.push_back("resource");
                while (first < self.element_count() && first < 1000) {
                    size_t last = std::min(first + 100, self.element_count());
                    attributes.push_back(fmt::format("[{}:{}]", first, last - 1));
                    first += 100;
                }
                return attributes;
            }
        )
        .def(
            "__getattr__",
            [](BufferCursor& self, std::string_view name)
            {
                if (name[0] == '[' && name[name.length() - 1] == ']') {
                    uint32_t first = 0;
                    uint32_t last = 0;
                    auto parts = string::split(name, ":");
                    if (parts.size() == 2) {
                        first = std::stoul(parts[0].substr(1));
                        last = std::stoul(parts[1].substr(0, parts[1].length() - 1));
                    }
                    std::vector<BufferElementCursor> res;
                    for (uint32_t i = first; i <= last; i++) {
                        res.push_back(self[i]);
                    }
                    return nb::cast(res);
                } else {
                    return nb::none();
                }
            }
        );
}
