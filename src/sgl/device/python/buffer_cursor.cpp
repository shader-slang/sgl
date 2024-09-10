// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/buffer_cursor.h"

#include "sgl/device/python/cursor_shared.h"

SGL_PY_EXPORT(device_buffer_cursor)
{
    using namespace sgl;

    nb::class_<BufferElementCursor> buffer_element_cursor(m, "BufferElementCursor", D_NA(BufferElementCursor));

    buffer_element_cursor //
        .def_prop_ro("offset", &BufferElementCursor::offset, D_NA(BufferElementCursor, offset))
        .def(
            "set_data",
            [](BufferElementCursor& self, nb::ndarray<nb::device::cpu> data)
            {
                SGL_CHECK(is_ndarray_contiguous(data), "data is not contiguous");
                self.set_data(data.data(), data.nbytes());
            },
            "data"_a,
            D_NA(BufferElementCursor, set_data)
        );

    bind_traversable_cursor(buffer_element_cursor);
    bind_writable_cursor_basic_types(buffer_element_cursor);

    nb::class_<BufferCursor>(m, "BufferCursor", D_NA(BufferCursor)) //
        .def(nb::init<ref<TypeLayoutReflection>, size_t>(), "layout"_a, "size"_a, D_NA(BufferCursor, BufferCursor))
        .def_prop_ro("type_layout", &BufferCursor::type_layout, D_NA(BufferCursor, type_layout))
        .def_prop_ro("type", &BufferCursor::type, D_NA(BufferCursor, type))
        .def("find_element", &BufferCursor::find_element, "index"_a, D_NA(BufferCursor, find_element))
        .def_prop_ro("element_count", &BufferCursor::element_count, D_NA(BufferCursor, element_count))
        .def_prop_ro("element_size", &BufferCursor::element_size, D_NA(BufferCursor, element_size))
        .def_prop_ro("size", &BufferCursor::size, D_NA(BufferCursor, size))
        .def("__getitem__", [](BufferCursor& self, int index) { return self[index]; })
        .def("__len__", [](BufferCursor& self) { return self.element_count(); });
}
