// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/buffer_cursor.h"

#include "sgl/device/python/cursor_shared.h"

#include "sgl/core/string.h"

namespace sgl {
namespace detail {

    template<typename ValType>
    inline nb::object read_scalar(const BufferElementCursor& self)
    {
        ValType res;
        self.get(res);
        return nb::cast(res);
    }

    template<typename ValType>
    inline nb::object read_vector(const BufferElementCursor& self)
    {
        ValType res;
        self.get(res);
        return nb::cast(res);
    }

    template<typename ValType>
    inline nb::object read_matrix(const BufferElementCursor& self)
    {
        ValType res;
        self.get(res);
        return nb::cast(res);
    }

#define scalar_case(c_type, scalar_type)                                                                               \
    scalar[(int)TypeReflection::ScalarType::scalar_type]                                                               \
        = [](const BufferElementCursor& self) { return read_scalar<c_type>(self); };

#define vector_case(c_type, scalar_type)                                                                               \
    vector[(int)TypeReflection::ScalarType::scalar_type][c_type::dimension]                                            \
        = [](const BufferElementCursor& self) { return read_vector<c_type>(self); };

#define matrix_case(c_type, scalar_type)                                                                               \
    matrix[(int)TypeReflection::ScalarType::scalar_type][c_type::rows][c_type::cols]                                   \
        = [](const BufferElementCursor& self) { return read_matrix<c_type>(self); };

    struct Converters {
        std::function<nb::object(const BufferElementCursor&)> scalar[(int)TypeReflection::ScalarType::COUNT];
        std::function<nb::object(const BufferElementCursor&)> vector[(int)TypeReflection::ScalarType::COUNT][5];
        std::function<nb::object(const BufferElementCursor&)> matrix[(int)TypeReflection::ScalarType::COUNT][5][5];

        Converters()
        {
            auto err_func = [](const BufferElementCursor&)
            {
                if (true) { // avoid 'unreachable code' warnings
                    SGL_THROW("Unsupported element type");
                }
                return nb::none();
            };

            for (int i = 0; i < (int)TypeReflection::ScalarType::COUNT; i++) {
                scalar[i] = err_func;
                for (int j = 0; j < 5; ++j) {
                    vector[i][j] = err_func;
                    for (int k = 0; k < 5; ++k)
                        matrix[i][j][k] = err_func;
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
            return detail::_conv.scalar[(int)type->scalar_type()](self);
        }
        case TypeReflection::Kind::vector: {
            return detail::_conv.vector[(int)type->scalar_type()][type->col_count()](self);
        }
        case TypeReflection::Kind::matrix: {
            return detail::_conv.matrix[(int)type->scalar_type()][type->row_count()][type->col_count()](self);
        }
        case TypeReflection::Kind::struct_: {
            nb::dict res;
            for (uint32_t i = 0; i < type->field_count(); i++) {
                auto field = type->get_field_by_index(i);
                res[field->name()] = read(self[field->name()]);
            }
            return res;
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
    bind_writable_cursor_basic_types(buffer_element_cursor);

    buffer_element_cursor //
        .def(
            "read",
            [](BufferElementCursor& self) { return detail::read(self); },
            D_NA(BufferElementCursor, val)
        );

    nb::class_<BufferCursor>(m, "BufferCursor", D_NA(BufferCursor)) //
        .def(nb::init<ref<TypeLayoutReflection>, size_t>(), "layout"_a, "size"_a, D_NA(BufferCursor, BufferCursor))
        .def_prop_ro("type_layout", &BufferCursor::type_layout, D_NA(BufferCursor, type_layout))
        .def_prop_ro("type", &BufferCursor::type, D_NA(BufferCursor, type))
        .def("find_element", &BufferCursor::find_element, "index"_a, D_NA(BufferCursor, find_element))
        .def_prop_ro("element_count", &BufferCursor::element_count, D_NA(BufferCursor, element_count))
        .def_prop_ro("element_size", &BufferCursor::element_size, D_NA(BufferCursor, element_size))
        .def_prop_ro("size", &BufferCursor::size, D_NA(BufferCursor, size))
        .def("__getitem__", [](BufferCursor& self, int index) { return self[index]; })
        .def("__len__", [](BufferCursor& self) { return self.element_count(); })
        .def(
            "to_numpy",
            [](BufferCursor& self)
            {
                size_t data_size = self.size();
                void* data = new uint8_t[data_size];
                memcpy(data, &self.data()[0], data_size);
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

                memcpy(&self.data()[0], data.data(), data_size);
            },
            "data"_a
        )
        .def(
            "__dir__",
            [](BufferCursor& self)
            {
                size_t first = 0;
                std::vector<std::string> attributes;
                attributes.push_back("type_layout");
                attributes.push_back("type");
                attributes.push_back("size");
                attributes.push_back("element_count");
                attributes.push_back("element_size");
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
