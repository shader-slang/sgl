// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/resource.h"
#include "sgl/device/device.h"
#include "sgl/device/formats.h"
#include "sgl/device/command.h"

#include "sgl/core/bitmap.h"

namespace sgl {

SGL_DICT_TO_DESC_BEGIN(BufferDesc)
SGL_DICT_TO_DESC_FIELD(size, size_t)
SGL_DICT_TO_DESC_FIELD(struct_size, size_t)
SGL_DICT_TO_DESC_FIELD(format, Format)
SGL_DICT_TO_DESC_FIELD(memory_type, MemoryType)
SGL_DICT_TO_DESC_FIELD(usage, BufferUsage)
SGL_DICT_TO_DESC_FIELD(default_state, ResourceState)
SGL_DICT_TO_DESC_FIELD(label, std::string)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(BufferOffsetPair)
SGL_DICT_TO_DESC_FIELD(buffer, Buffer*)
SGL_DICT_TO_DESC_FIELD(offset, DeviceOffset)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(TextureDesc)
SGL_DICT_TO_DESC_FIELD(type, TextureType)
SGL_DICT_TO_DESC_FIELD(format, Format)
SGL_DICT_TO_DESC_FIELD(width, uint32_t)
SGL_DICT_TO_DESC_FIELD(height, uint32_t)
SGL_DICT_TO_DESC_FIELD(depth, uint32_t)
SGL_DICT_TO_DESC_FIELD(array_length, uint32_t)
SGL_DICT_TO_DESC_FIELD(mip_count, uint32_t)
SGL_DICT_TO_DESC_FIELD(sample_count, uint32_t)
SGL_DICT_TO_DESC_FIELD(sample_quality, uint32_t)
SGL_DICT_TO_DESC_FIELD(memory_type, MemoryType)
SGL_DICT_TO_DESC_FIELD(usage, TextureUsage)
SGL_DICT_TO_DESC_FIELD(default_state, ResourceState)
SGL_DICT_TO_DESC_FIELD(label, std::string)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(SubresourceRange)
SGL_DICT_TO_DESC_FIELD(mip_level, uint32_t)
SGL_DICT_TO_DESC_FIELD(mip_count, uint32_t)
SGL_DICT_TO_DESC_FIELD(base_array_layer, uint32_t)
SGL_DICT_TO_DESC_FIELD(layer_count, uint32_t)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(TextureViewDesc)
SGL_DICT_TO_DESC_FIELD(format, Format)
SGL_DICT_TO_DESC_FIELD(aspect, TextureAspect)
SGL_DICT_TO_DESC_FIELD(subresource_range, SubresourceRange)
SGL_DICT_TO_DESC_FIELD(label, std::string)
SGL_DICT_TO_DESC_END()

inline std::optional<nb::dlpack::dtype> resource_format_to_dtype(Format format)
{
    const auto& info = get_format_info(format);

    // Undefined and compressed formats are not supported.
    if (format == Format::undefined || info.is_compressed)
        return {};

    // Formats with different bits per channel are not supported.
    if (!info.has_equal_channel_bits())
        return {};

    // Only formats with 8, 16, 32, or 64 bits per channel are supported.
    uint32_t channel_bit_count = info.channel_bit_count[0];
    if (channel_bit_count != 8 && channel_bit_count != 16 && channel_bit_count != 32 && channel_bit_count != 64)
        return {};

    switch (info.type) {
    case FormatType::float_:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Float, (uint8_t)channel_bit_count, 1};
    case FormatType::uint:
    case FormatType::unorm:
    case FormatType::unorm_srgb:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::UInt, (uint8_t)channel_bit_count, 1};
    case FormatType::sint:
    case FormatType::snorm:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Int, (uint8_t)channel_bit_count, 1};
    default:
        return {};
    }
}

static const char* __doc_sgl_buffer_to_numpy = R"doc()doc";

nb::ndarray<nb::numpy> buffer_to_numpy(Buffer* self)
{
    size_t data_size = self->size();
    void* data = new uint8_t[data_size];

    self->get_data(data, data_size);

    nb::capsule owner(data, [](void* p) noexcept { delete[] reinterpret_cast<uint8_t*>(p); });

    if (auto dtype = resource_format_to_dtype(self->format())) {
        const FormatInfo& format_info = get_format_info(self->format());
        size_t channel_count = format_info.channel_count;
        size_t element_count = self->size() / format_info.bytes_per_block;
        if (channel_count == 1) {
            size_t shape[1] = {element_count};
            return nb::ndarray<nb::numpy>(data, 1, shape, owner, nullptr, *dtype, nb::device::cpu::value);
        } else {
            size_t shape[2] = {element_count, channel_count};
            return nb::ndarray<nb::numpy>(data, 2, shape, owner, nullptr, *dtype, nb::device::cpu::value);
        }
    } else {
        size_t shape[1] = {data_size};
        return nb::ndarray<nb::numpy>(data, 1, shape, owner, nullptr, nb::dtype<uint8_t>(), nb::device::cpu::value);
    }
}

static const char* __doc_sgl_buffer_from_numpy = R"doc()doc";

void buffer_copy_from_numpy(Buffer* self, nb::ndarray<nb::numpy> data)
{
    SGL_CHECK(is_ndarray_contiguous(data), "numpy array is not contiguous");

    size_t buffer_size = self->size();
    size_t data_size = data.nbytes();
    SGL_CHECK(data_size <= buffer_size, "numpy array is larger than the buffer ({} > {})", data_size, buffer_size);

    self->set_data(data.data(), data_size);
}

static const char* __doc_sgl_buffer_to_torch = R"doc()doc";

nb::ndarray<nb::pytorch, nb::device::cuda>
buffer_to_torch(Buffer* self, DataType type, std::vector<size_t> shape, std::vector<int64_t> strides, size_t offset)
{
    SGL_CHECK(offset < self->size(), "offset is out of bounds");
    void* data = reinterpret_cast<uint8_t*>(self->cuda_memory()) + offset;
    size_t data_size = self->size() - offset;

    nb::dlpack::dtype dtype = data_type_to_dtype(type);
    size_t element_size = dtype.bits / 8;

    if (shape.empty())
        shape.push_back(data_size / element_size);
    int64_t* strides_ptr = nullptr;
    if (!strides.empty()) {
        SGL_CHECK(strides.size() == shape.size(), "strides must have the same length as the shape");
        strides_ptr = strides.data();
    }

    size_t required_size = 1;
    for (size_t dim : shape)
        required_size *= dim;
    SGL_CHECK(data_size >= required_size * element_size, "requested shape exceeds the buffer size");

    return nb::ndarray<nb::pytorch, nb::device::cuda>(
        data,
        shape.size(),
        shape.data(),
        nb::find(self),
        strides.empty() ? nullptr : strides.data(),
        dtype
    );
}

static const char* __doc_sgl_texture_to_numpy = R"doc()doc";

/**
 * Python binding wrapper for returning the content of a texture as a numpy array.
 */
nb::ndarray<nb::numpy> texture_to_numpy(Texture* self, uint32_t layer, uint32_t mip_level)
{
    SGL_CHECK_LT(layer, self->layer_count());
    SGL_CHECK_LT(mip_level, self->mip_count());

    uint3 mip_size = self->get_mip_size(mip_level);
    OwnedSubresourceData subresource_data = self->get_subresource_data(layer, mip_level);

    size_t size = subresource_data.size;
    void* data = subresource_data.owned_data.release();

    nb::capsule owner(data, [](void* p) noexcept { delete[] reinterpret_cast<uint8_t*>(p); });

    if (auto dtype = resource_format_to_dtype(self->format())) {
        uint32_t channel_count = get_format_info(self->format()).channel_count;
        int64_t row_stride = subresource_data.row_pitch / get_format_info(self->format()).bytes_per_block;
        std::vector<size_t> shape;
        std::vector<int64_t> strides;
        if (mip_size.z > 1) {
            shape.push_back(mip_size.z);
            strides.push_back(mip_size.y * row_stride * channel_count);
        }
        if (mip_size.y > 1) {
            shape.push_back(mip_size.y);
            strides.push_back(row_stride * channel_count);
        }
        shape.push_back(mip_size.x);
        strides.push_back(channel_count);
        if (channel_count > 1) {
            shape.push_back(channel_count);
            strides.push_back(1);
        }
        return nb::ndarray<nb::numpy>(data, shape.size(), shape.data(), owner, strides.data(), *dtype, nb::device::cpu::value);
    } else {
        size_t shape[1] = {size};
        return nb::ndarray<nb::numpy>(data, 1, shape, owner, nullptr, nb::dtype<uint8_t>(), nb::device::cpu::value);
    }
}

static const char* __doc_sgl_texture_from_numpy = R"doc()doc";

inline void texture_from_numpy(Texture* self, nb::ndarray<nb::numpy> data, uint32_t layer, uint32_t mip_level)
{
    SGL_CHECK(is_ndarray_contiguous(data), "numpy array is not contiguous");
    SGL_CHECK_LT(layer, self->layer_count());
    SGL_CHECK_LT(mip_level, self->mip_count());

    uint3 mip_size = self->get_mip_size(mip_level);
    SubresourceLayout subresource_layout = self->get_subresource_layout(mip_level, 1);
    SubresourceData subresource_data{
        .data = data.data(),
        .size = data.nbytes(),
        .row_pitch = subresource_layout.row_pitch,
        .slice_pitch = subresource_layout.slice_pitch,
    };

    if (auto dtype = resource_format_to_dtype(self->format())) {
        std::vector<size_t> expected_shape;
        switch (self->type()) {
        case TextureType::texture_1d:
        case TextureType::texture_1d_array:
            expected_shape = {size_t(mip_size.x)};
            break;
        case TextureType::texture_2d:
        case TextureType::texture_2d_array:
        case TextureType::texture_2d_ms:
        case TextureType::texture_2d_ms_array:
        case TextureType::texture_cube:
        case TextureType::texture_cube_array:
            expected_shape = {size_t(mip_size.y), size_t(mip_size.x)};
            break;
        case TextureType::texture_3d:
            expected_shape = {size_t(mip_size.z), size_t(mip_size.y), size_t(mip_size.x)};
            break;
        }
        uint32_t channel_count = get_format_info(self->format()).channel_count;
        if (channel_count > 1)
            expected_shape.push_back(channel_count);

        SGL_CHECK(
            data.ndim() == expected_shape.size(),
            "numpy array has wrong number of dimensions (expected {})",
            expected_shape.size()
        );
        for (size_t i = 0; i < expected_shape.size(); ++i)
            SGL_CHECK(data.shape(i) == expected_shape[i], "numpy array has wrong shape (expected {})", expected_shape);
    }

    SGL_CHECK(data.nbytes() == subresource_layout.size_in_bytes, "numpy array doesn't match the subresource size");
    self->set_subresource_data(layer, mip_level, subresource_data);
}

} // namespace sgl

SGL_PY_EXPORT(device_resource)
{
    using namespace sgl;

    nb::sgl_enum<ResourceState>(m, "ResourceState");
    nb::sgl_enum_flags<BufferUsage>(m, "BufferUsage");
    nb::sgl_enum_flags<TextureUsage>(m, "TextureUsage");
    nb::sgl_enum<TextureType>(m, "TextureType");
    nb::sgl_enum<MemoryType>(m, "MemoryType");

    nb::class_<Resource, DeviceResource>(m, "Resource", D(Resource));

    nb::sgl_enum<TextureAspect>(m, "TextureAspect");

    nb::class_<BufferRange>(m, "BufferRange", D_NA(BufferRange))
        .def(nb::init<>())
        .def_ro("offset", &BufferRange::offset, D_NA(BufferRange, offset))
        .def_ro("size", &BufferRange::size, D_NA(BufferRange, size))
        .def("__repr__", &BufferRange::to_string, D_NA(BufferRange, to_string));

    nb::class_<SubresourceRange>(m, "SubresourceRange", D(SubresourceRange))
        .def(nb::init<>())
        .def(
            "__init__",
            [](SubresourceRange* self, nb::dict dict) { new (self) SubresourceRange(dict_to_SubresourceRange(dict)); }
        )
        .def_ro("mip_level", &SubresourceRange::mip_level, D_NA(SubresourceRange, mip_level))
        .def_ro("mip_count", &SubresourceRange::mip_count, D_NA(SubresourceRange, mip_count))
        .def_ro("base_array_layer", &SubresourceRange::base_array_layer, D_NA(SubresourceRange, base_array_layer))
        .def_ro("layer_count", &SubresourceRange::layer_count, D_NA(SubresourceRange, layer_count))
        .def("__repr__", &SubresourceRange::to_string, D_NA(SubresourceRange, to_string));

#if 0 // TODO(slang-rhi)
    nb::class_<ResourceView, Object>(m, "ResourceView", D(ResourceView))
        .def_prop_ro("type", &ResourceView::type, D(ResourceView, type))
        .def_prop_ro("resource", &ResourceView::resource, D(ResourceView, resource))
        .def_prop_ro("buffer_range", &ResourceView::buffer_range, D_NA(ResourceView, buffer_range))
        .def_prop_ro("subresource_range", &ResourceView::subresource_range, D_NA(ResourceView, subresource_range));
#endif

    nb::class_<BufferDesc>(m, "BufferDesc", D(BufferDesc))
        .def(nb::init<>())
        .def("__init__", [](BufferDesc* self, nb::dict dict) { new (self) BufferDesc(dict_to_BufferDesc(dict)); })
        .def_rw("size", &BufferDesc::size, D(BufferDesc, size))
        .def_rw("struct_size", &BufferDesc::struct_size, D(BufferDesc, struct_size))
        .def_rw("format", &BufferDesc::format, D(BufferDesc, format))
        .def_rw("memory_type", &BufferDesc::memory_type, D(BufferDesc, memory_type))
        .def_rw("usage", &BufferDesc::usage, D(BufferDesc, usage))
        .def_rw("default_state", &BufferDesc::default_state, D_NA(BufferDesc, default_state))
        .def_rw("label", &BufferDesc::label, D_NA(BufferDesc, label));
    nb::implicitly_convertible<nb::dict, BufferDesc>();

    nb::class_<Buffer, Resource>(m, "Buffer", D(Buffer))
        .def_prop_ro("desc", &Buffer::desc, D(Buffer, desc))
        .def_prop_ro("size", &Buffer::size, D(Buffer, size))
        .def_prop_ro("struct_size", &Buffer::struct_size, D(Buffer, struct_size))
        .def_prop_ro("device_address", &Buffer::device_address, D(Buffer, device_address))
        .def("to_numpy", &buffer_to_numpy, D(buffer_to_numpy))
        .def("copy_from_numpy", &buffer_copy_from_numpy, "data"_a, D(buffer_from_numpy))
        .def(
            "to_torch",
            &buffer_to_torch,
            "type"_a = DataType::void_,
            "shape"_a = std::vector<size_t>{},
            "strides"_a = std::vector<int64_t>{},
            "offset"_a = 0,
            D(buffer_to_torch)
        );

    nb::class_<BufferOffsetPair>(m, "BufferOffsetPair", D_NA(BufferOffsetPair))
        .def(nb::init<>())
        .def(nb::init<Buffer*>(), "buffer"_a)
        .def(nb::init<Buffer*, DeviceOffset>(), "buffer"_a, "offset"_a = 0)
        .def(
            "__init__",
            [](BufferOffsetPair* self, nb::dict dict) { new (self) BufferOffsetPair(dict_to_BufferOffsetPair(dict)); }
        )
        .def_rw("buffer", &BufferOffsetPair::buffer, D_NA(BufferOffsetPair, buffer))
        .def_rw("offset", &BufferOffsetPair::offset, D_NA(BufferOffsetPair, offset));
    nb::implicitly_convertible<nb::dict, BufferOffsetPair>();
    nb::implicitly_convertible<Buffer, BufferOffsetPair>();

    nb::class_<TextureDesc>(m, "TextureDesc", D(TextureDesc))
        .def(nb::init<>())
        .def("__init__", [](TextureDesc* self, nb::dict dict) { new (self) TextureDesc(dict_to_TextureDesc(dict)); })
        .def_rw("type", &TextureDesc::type, D(TextureDesc, type))
        .def_rw("format", &TextureDesc::format, D(TextureDesc, format))
        .def_rw("width", &TextureDesc::width, D(TextureDesc, width))
        .def_rw("height", &TextureDesc::height, D(TextureDesc, height))
        .def_rw("depth", &TextureDesc::depth, D(TextureDesc, depth))
        .def_rw("array_length", &TextureDesc::array_length, D_NA(TextureDesc, array_length))
        .def_rw("mip_count", &TextureDesc::mip_count, D(TextureDesc, mip_count))
        .def_rw("sample_count", &TextureDesc::sample_count, D(TextureDesc, sample_count))
        .def_rw("sample_quality", &TextureDesc::sample_quality, D_NA(TextureDesc, quality))
        .def_rw("memory_type", &TextureDesc::memory_type, D(TextureDesc, memory_type))
        .def_rw("usage", &TextureDesc::usage, D(TextureDesc, usage))
        .def_rw("default_state", &TextureDesc::default_state, D_NA(TextureDesc, default_state))
        .def_rw("label", &TextureDesc::label, D_NA(TextureDesc, label));
    nb::implicitly_convertible<nb::dict, TextureDesc>();

    nb::class_<TextureViewDesc>(m, "TextureViewDesc", D_NA(TextureViewDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](TextureViewDesc* self, nb::dict dict) { new (self) TextureViewDesc(dict_to_TextureViewDesc(dict)); }
        )
        .def_rw("format", &TextureViewDesc::format, D_NA(TextureViewDesc, format))
        .def_rw("aspect", &TextureViewDesc::aspect, D_NA(TextureViewDesc, aspect))
        .def_rw("subresource_range", &TextureViewDesc::subresource_range, D_NA(TextureViewDesc, subresource_range))
        .def_rw("label", &TextureViewDesc::label, D_NA(TextureViewDesc, label));
    nb::implicitly_convertible<nb::dict, TextureViewDesc>();

// TODO(slang-rhi)
#if 0
    nb::class_<SubresourceLayout>(m, "SubresourceLayout", D(SubresourceLayout))
        .def_ro("row_pitch", &SubresourceLayout::row_pitch, D(SubresourceLayout, row_pitch))
        .def_ro("row_pitch_aligned", &SubresourceLayout::row_pitch_aligned, D(SubresourceLayout, row_pitch_aligned))
        .def_ro("row_count", &SubresourceLayout::row_count, D(SubresourceLayout, row_count))
        .def_ro("depth", &SubresourceLayout::depth, D(SubresourceLayout, depth))
        .def_prop_ro("total_size", &SubresourceLayout::total_size, D(SubresourceLayout, total_size))
        .def_prop_ro(
            "total_size_aligned",
            &SubresourceLayout::total_size_aligned,
            D(SubresourceLayout, total_size_aligned)
        );
#endif

    nb::class_<Texture, Resource>(m, "Texture", D(Texture))
        .def_prop_ro("desc", &Texture::desc, D(Texture, desc))
        .def_prop_ro("type", &Texture::type, D_NA(Texture, type))
        .def_prop_ro("format", &Texture::format, D(Texture, format))
        .def_prop_ro("width", &Texture::width, D(Texture, width))
        .def_prop_ro("height", &Texture::height, D(Texture, height))
        .def_prop_ro("depth", &Texture::depth, D(Texture, depth))
        .def_prop_ro("array_length", &Texture::array_length, D_NA(Texture, array_length))
        .def_prop_ro("mip_count", &Texture::mip_count, D(Texture, mip_count))
        .def_prop_ro("layer_count", &Texture::layer_count, D_NA(Texture, layer_count))
        .def_prop_ro("subresource_count", &Texture::subresource_count, D(Texture, subresource_count))
        .def("get_mip_width", &Texture::get_mip_width, "mip_level"_a = 0, D(Texture, get_mip_width))
        .def("get_mip_height", &Texture::get_mip_height, "mip_level"_a = 0, D(Texture, get_mip_height))
        .def("get_mip_depth", &Texture::get_mip_depth, "mip_level"_a = 0, D(Texture, get_mip_depth))
        .def("get_mip_size", &Texture::get_mip_size, "mip_level"_a = 0, D_NA(Texture, get_mip_size))
        .def(
            "get_subresource_layout",
            &Texture::get_subresource_layout,
            "subresource"_a,
            "row_alignment"_a = DEFAULT_ALIGNMENT,
            D(Texture, get_subresource_layout)
        )
        .def("create_view", &Texture::create_view, "desc"_a, D_NA(Texture, create_view))
        .def(
            "create_view",
            [](Texture* self, nb::dict dict) { return self->create_view(dict_to_TextureViewDesc(dict)); },
            "dict"_a,
            D_NA(Texture, create_view)
        )
        .def("to_bitmap", &Texture::to_bitmap, "layer"_a = 0, "mip_level"_a = 0, D(Texture, to_bitmap))
        .def("to_numpy", &texture_to_numpy, "layer"_a = 0, "mip_level"_a = 0, D(texture_to_numpy))
        .def("copy_from_numpy", &texture_from_numpy, "data"_a, "layer"_a = 0, "mip_level"_a = 0, D(texture_from_numpy));

    nb::class_<TextureView, DeviceResource>(m, "TextureView", D_NA(TextureView))
        .def_prop_ro("texture", &TextureView::texture, D_NA(TextureView, texture))
        .def_prop_ro("desc", &TextureView::desc, D_NA(TextureView, desc))
        .def_prop_ro("format", &TextureView::format, D_NA(TextureView, format))
        .def_prop_ro("aspect", &TextureView::aspect, D_NA(TextureView, aspect))
        .def_prop_ro("subresource_range", &TextureView::subresource_range, D_NA(TextureView, subresource_range))
        .def_prop_ro("label", &TextureView::label, D_NA(TextureView, label))
        .def("__repr__", &TextureView::to_string, D_NA(TextureView, to_string));
}
