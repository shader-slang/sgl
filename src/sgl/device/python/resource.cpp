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
SGL_DICT_TO_DESC_FIELD(initial_state, ResourceState)
SGL_DICT_TO_DESC_FIELD(usage, ResourceUsage)
SGL_DICT_TO_DESC_FIELD(memory_type, MemoryType)
SGL_DICT_TO_DESC_FIELD(debug_name, std::string)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(TextureDesc)
SGL_DICT_TO_DESC_FIELD(type, ResourceType)
SGL_DICT_TO_DESC_FIELD(format, Format)
SGL_DICT_TO_DESC_FIELD(width, uint32_t)
SGL_DICT_TO_DESC_FIELD(height, uint32_t)
SGL_DICT_TO_DESC_FIELD(depth, uint32_t)
SGL_DICT_TO_DESC_FIELD(array_size, uint32_t)
SGL_DICT_TO_DESC_FIELD(mip_count, uint32_t)
SGL_DICT_TO_DESC_FIELD(sample_count, uint32_t)
SGL_DICT_TO_DESC_FIELD(quality, uint32_t)
SGL_DICT_TO_DESC_FIELD(initial_state, ResourceState)
SGL_DICT_TO_DESC_FIELD(usage, ResourceUsage)
SGL_DICT_TO_DESC_FIELD(memory_type, MemoryType)
SGL_DICT_TO_DESC_FIELD(debug_name, std::string)
SGL_DICT_TO_DESC_END()


inline std::optional<nb::dlpack::dtype> resource_format_to_dtype(Format format)
{
    const auto& info = get_format_info(format);

    // Unknown and compressed formats are not supported.
    if (format == Format::unknown || info.is_compressed)
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

inline nb::ndarray<nb::numpy> buffer_to_numpy(Buffer* self)
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

inline void buffer_from_numpy(Buffer* self, nb::ndarray<nb::numpy> data)
{
    SGL_CHECK(is_ndarray_contiguous(data), "numpy array is not contiguous");

    size_t buffer_size = self->size();
    size_t data_size = data.nbytes();
    SGL_CHECK(data_size <= buffer_size, "numpy array is larger than the buffer ({} > {})", data_size, buffer_size);

    self->set_data(data.data(), data_size);
}

static const char* __doc_sgl_texture_to_numpy = R"doc()doc";

/**
 * Python binding wrapper for returning the content of a texture as a numpy array.
 */
inline nb::ndarray<nb::numpy> texture_to_numpy(Texture* self, uint32_t mip_level, uint32_t array_slice)
{
    SGL_CHECK_LT(mip_level, self->mip_count());
    SGL_CHECK_LT(array_slice, self->layer_count());

    uint3 dimensions = self->get_mip_dimensions(mip_level);
    uint32_t subresource = self->get_subresource_index(mip_level, array_slice);
    OwnedSubresourceData subresource_data = self->get_subresource_data(subresource);

    size_t size = subresource_data.size;
    void* data = subresource_data.owned_data.release();

    nb::capsule owner(data, [](void* p) noexcept { delete[] reinterpret_cast<uint8_t*>(p); });

    if (auto dtype = resource_format_to_dtype(self->format())) {
        uint32_t channel_count = get_format_info(self->format()).channel_count;
        std::vector<size_t> shape;
        if (dimensions.z > 1)
            shape.push_back(dimensions.z);
        if (dimensions.y > 1)
            shape.push_back(dimensions.y);
        shape.push_back(dimensions.x);
        if (channel_count > 1)
            shape.push_back(channel_count);
        return nb::ndarray<nb::numpy>(data, shape.size(), shape.data(), owner, nullptr, *dtype, nb::device::cpu::value);
    } else {
        size_t shape[1] = {size};
        return nb::ndarray<nb::numpy>(data, 1, shape, owner, nullptr, nb::dtype<uint8_t>(), nb::device::cpu::value);
    }
}

static const char* __doc_sgl_texture_from_numpy = R"doc()doc";

inline void texture_from_numpy(Texture* self, nb::ndarray<nb::numpy> data, uint32_t mip_level, uint32_t array_slice)
{
    SGL_CHECK(is_ndarray_contiguous(data), "numpy array is not contiguous");
    SGL_CHECK_LT(mip_level, self->mip_count());
    SGL_CHECK_LT(array_slice, self->layer_count());

    uint3 dimensions = self->get_mip_dimensions(mip_level);
    uint32_t subresource = self->get_subresource_index(mip_level, array_slice);
    SubresourceLayout subresource_layout = self->get_subresource_layout(subresource);
    SubresourceData subresource_data{
        .data = data.data(),
        .size = data.nbytes(),
        .row_pitch = subresource_layout.row_pitch,
        .slice_pitch = subresource_layout.row_count * subresource_layout.row_pitch,
    };

    if (auto dtype = resource_format_to_dtype(self->format())) {
        std::vector<size_t> expected_shape;
        switch (self->type()) {
        case ResourceType::texture_1d:
            expected_shape = {size_t(dimensions.x)};
            break;
        case ResourceType::texture_2d:
            expected_shape = {size_t(dimensions.y), size_t(dimensions.x)};
            break;
        case ResourceType::texture_3d:
            expected_shape = {size_t(dimensions.z), size_t(dimensions.y), size_t(dimensions.x)};
            break;
        case ResourceType::texture_cube:
            expected_shape = {size_t(dimensions.y), size_t(dimensions.x)};
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

    SGL_CHECK(data.nbytes() == subresource_layout.total_size(), "numpy array doesn't match the subresource size");
    self->set_subresource_data(subresource, subresource_data);
}

} // namespace sgl

SGL_PY_EXPORT(device_resource)
{
    using namespace sgl;

    nb::sgl_enum<ResourceType>(m, "ResourceType");
    nb::sgl_enum<ResourceState>(m, "ResourceState");

    nb::sgl_enum_flags<ResourceUsage>(m, "ResourceUsage");

    nb::sgl_enum<MemoryType>(m, "MemoryType");

    nb::class_<Resource, DeviceResource>(m, "Resource", D(Resource))
        .def_prop_ro("type", &Resource::type, D(Resource, type))
        .def_prop_ro("format", &Resource::format, D(Resource, format));

    nb::sgl_enum<ResourceViewType>(m, "ResourceViewType");

    nb::sgl_enum<TextureAspect>(m, "TextureAspect");

    nb::class_<BufferRange>(m, "BufferRange", D(SubresourceRange))
        .def(nb::init<>())
        .def_ro("offset", &BufferRange::offset, D_NA(BufferRange, offset))
        .def_ro("size", &BufferRange::size, D_NA(BufferRange, size))
        .def("__repr__", &BufferRange::to_string, D_NA(BufferRange, to_string));

    nb::class_<SubresourceRange>(m, "SubresourceRange", D(SubresourceRange))
        .def(nb::init<>())
        .def_ro("texture_aspect", &SubresourceRange::texture_aspect, D_NA(SubresourceRange, texture_aspect))
        .def_ro("mip_level", &SubresourceRange::mip_level, D_NA(SubresourceRange, mip_level))
        .def_ro("mip_count", &SubresourceRange::mip_count, D_NA(SubresourceRange, mip_count))
        .def_ro("base_array_layer", &SubresourceRange::base_array_layer, D_NA(SubresourceRange, base_array_layer))
        .def_ro("layer_count", &SubresourceRange::layer_count, D_NA(SubresourceRange, layer_count))
        .def("__repr__", &SubresourceRange::to_string, D_NA(SubresourceRange, to_string));

    nb::class_<ResourceView, Object>(m, "ResourceView", D(ResourceView))
        .def_prop_ro("type", &ResourceView::type, D(ResourceView, type))
        .def_prop_ro("resource", &ResourceView::resource, D(ResourceView, resource))
        .def_prop_ro("buffer_range", &ResourceView::buffer_range, D_NA(ResourceView, buffer_range))
        .def_prop_ro("subresource_range", &ResourceView::subresource_range, D_NA(ResourceView, subresource_range));

    nb::class_<BufferDesc>(m, "BufferDesc", D(BufferDesc))
        .def(nb::init<>())
        .def("__init__", [](BufferDesc* self, nb::dict dict) { new (self) BufferDesc(dict_to_BufferDesc(dict)); })
        .def_rw("size", &BufferDesc::size, D(BufferDesc, size))
        .def_rw("struct_size", &BufferDesc::struct_size, D(BufferDesc, struct_size))
        .def_rw("format", &BufferDesc::format, D(BufferDesc, format))
        .def_rw("initial_state", &BufferDesc::initial_state, D(BufferDesc, initial_state))
        .def_rw("usage", &BufferDesc::usage, D(BufferDesc, usage))
        .def_rw("memory_type", &BufferDesc::memory_type, D(BufferDesc, memory_type))
        .def_rw("debug_name", &BufferDesc::debug_name, D(BufferDesc, debug_name));
    nb::implicitly_convertible<nb::dict, BufferDesc>();

    nb::class_<Buffer, Resource>(m, "Buffer", D(Buffer))
        .def_prop_ro("desc", &Buffer::desc, D(Buffer, desc))
        .def_prop_ro("size", &Buffer::size, D(Buffer, size))
        .def_prop_ro("struct_size", &Buffer::struct_size, D(Buffer, struct_size))
        .def_prop_ro("device_address", &Buffer::device_address, D(Buffer, device_address))
        .def(
            "get_srv",
            [](Buffer* self, uint64_t offset, uint64_t size)
            {
                return self->get_srv({
                    .offset = offset,
                    .size = size,
                });
            },
            "offset"_a = 0,
            "size"_a = BufferRange::ALL,
            D(Buffer, get_srv)
        )
        .def(
            "get_uav",
            [](Buffer* self, uint64_t offset, uint64_t size)
            {
                return self->get_uav({
                    .offset = offset,
                    .size = size,
                });
            },
            "offset"_a = 0,
            "size"_a = BufferRange::ALL,
            D(Buffer, get_uav)
        )
        .def("to_numpy", &buffer_to_numpy, D(buffer_to_numpy))
        .def("from_numpy", &buffer_from_numpy, "data"_a, D(buffer_from_numpy));

    nb::class_<TextureDesc>(m, "TextureDesc", D(TextureDesc))
        .def(nb::init<>())
        .def("__init__", [](TextureDesc* self, nb::dict dict) { new (self) TextureDesc(dict_to_TextureDesc(dict)); })
        .def_rw("type", &TextureDesc::type, D(TextureDesc, type))
        .def_rw("format", &TextureDesc::format, D(TextureDesc, format))
        .def_rw("width", &TextureDesc::width, D(TextureDesc, width))
        .def_rw("height", &TextureDesc::height, D(TextureDesc, height))
        .def_rw("depth", &TextureDesc::depth, D(TextureDesc, depth))
        .def_rw("array_size", &TextureDesc::array_size, D(TextureDesc, array_size))
        .def_rw("mip_count", &TextureDesc::mip_count, D(TextureDesc, mip_count))
        .def_rw("sample_count", &TextureDesc::sample_count, D(TextureDesc, sample_count))
        .def_rw("quality", &TextureDesc::quality, D(TextureDesc, quality))
        .def_rw("initial_state", &TextureDesc::initial_state, D(TextureDesc, initial_state))
        .def_rw("usage", &TextureDesc::usage, D(TextureDesc, usage))
        .def_rw("memory_type", &TextureDesc::memory_type, D(TextureDesc, memory_type))
        .def_rw("debug_name", &TextureDesc::debug_name, D(TextureDesc, debug_name));
    nb::implicitly_convertible<nb::dict, TextureDesc>();

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

    nb::class_<Texture, Resource>(m, "Texture", D(Texture))
        .def_prop_ro("desc", &Texture::desc, D(Texture, desc))
        .def_prop_ro("width", &Texture::width, D(Texture, width))
        .def_prop_ro("height", &Texture::height, D(Texture, height))
        .def_prop_ro("depth", &Texture::depth, D(Texture, depth))
        .def_prop_ro("array_size", &Texture::array_size, D(Texture, array_size))
        .def_prop_ro("mip_count", &Texture::mip_count, D(Texture, mip_count))
        .def_prop_ro("subresource_count", &Texture::subresource_count, D(Texture, subresource_count))
        .def(
            "get_subresource_index",
            &Texture::get_subresource_index,
            "mip_level"_a,
            "array_slice"_a = 0,
            D(Texture, get_subresource_index)
        )
        .def(
            "get_subresource_array_slice",
            &Texture::get_subresource_array_slice,
            "subresource"_a,
            D(Texture, get_subresource_array_slice)
        )
        .def(
            "get_subresource_mip_level",
            &Texture::get_subresource_mip_level,
            "subresource"_a,
            D(Texture, get_subresource_mip_level)
        )
        .def("get_mip_width", &Texture::get_mip_width, "mip_level"_a = 0, D(Texture, get_mip_width))
        .def("get_mip_height", &Texture::get_mip_height, "mip_level"_a = 0, D(Texture, get_mip_height))
        .def("get_mip_depth", &Texture::get_mip_depth, "mip_level"_a = 0, D(Texture, get_mip_depth))
        .def("get_mip_dimensions", &Texture::get_mip_dimensions, "mip_level"_a = 0, D(Texture, get_mip_dimensions))
        .def(
            "get_subresource_layout",
            &Texture::get_subresource_layout,
            "subresource"_a,
            D(Texture, get_subresource_layout)
        )
        .def(
            "get_srv",
            [](Texture* self, uint32_t mip_level, uint32_t mip_count, uint32_t base_array_layer, uint32_t layer_count)
            {
                return self->get_srv({
                    .mip_level = mip_level,
                    .mip_count = mip_count,
                    .base_array_layer = base_array_layer,
                    .layer_count = layer_count,
                });
            },
            "mip_level"_a = 0,
            "mip_count"_a = SubresourceRange::ALL,
            "base_array_layer"_a = 0,
            "layer_count"_a = SubresourceRange::ALL,
            D(Texture, get_srv)
        )
        .def(
            "get_uav",
            [](Texture* self, uint32_t mip_level, uint32_t base_array_layer, uint32_t layer_count)
            {
                return self->get_uav({
                    .mip_level = mip_level,
                    .base_array_layer = base_array_layer,
                    .layer_count = layer_count,
                });
            },
            "mip_level"_a = 0,
            "base_array_layer"_a = 0,
            "layer_count"_a = SubresourceRange::ALL,
            D(Texture, get_uav)
        )
        .def(
            "get_dsv",
            [](Texture* self, uint32_t mip_level, uint32_t base_array_layer, uint32_t layer_count)
            {
                return self->get_dsv({
                    .mip_level = mip_level,
                    .base_array_layer = base_array_layer,
                    .layer_count = layer_count,
                });
            },
            "mip_level"_a = 0,
            "base_array_layer"_a = 0,
            "layer_count"_a = SubresourceRange::ALL,
            D(Texture, get_dsv)
        )
        .def(
            "get_rtv",
            [](Texture* self, uint32_t mip_level, uint32_t base_array_layer, uint32_t layer_count)
            {
                return self->get_rtv({
                    .mip_level = mip_level,
                    .base_array_layer = base_array_layer,
                    .layer_count = layer_count,
                });
            },
            "mip_level"_a = 0,
            "base_array_layer"_a = 0,
            "layer_count"_a = SubresourceRange::ALL,
            D(Texture, get_rtv)
        )
        .def("to_bitmap", &Texture::to_bitmap, "mip_level"_a = 0, "array_slice"_a = 0, D(Texture, to_bitmap))
        .def("to_numpy", &texture_to_numpy, "mip_level"_a = 0, "array_slice"_a = 0, D(texture_to_numpy))
        .def(
            "from_numpy",
            &texture_from_numpy,
            "data"_a,
            "mip_level"_a = 0,
            "array_slice"_a = 0,
            D(texture_from_numpy)
        );
}
