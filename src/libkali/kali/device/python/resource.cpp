#include "nanobind.h"

#include "kali/device/resource.h"
#include "kali/device/device.h"
#include "kali/device/formats.h"
#include "kali/device/command.h"

#include "kali/core/bitmap.h"

namespace kali {

KALI_DICT_TO_DESC_BEGIN(BufferDesc)
KALI_DICT_TO_DESC_FIELD(size, size_t)
KALI_DICT_TO_DESC_FIELD(struct_size, size_t)
KALI_DICT_TO_DESC_FIELD(format, Format)
KALI_DICT_TO_DESC_FIELD(initial_state, ResourceState)
KALI_DICT_TO_DESC_FIELD(usage, ResourceUsage)
KALI_DICT_TO_DESC_FIELD(memory_type, MemoryType)
KALI_DICT_TO_DESC_FIELD(debug_name, std::string)
KALI_DICT_TO_DESC_END()

KALI_DICT_TO_DESC_BEGIN(TextureDesc)
KALI_DICT_TO_DESC_FIELD(type, ResourceType)
KALI_DICT_TO_DESC_FIELD(format, Format)
KALI_DICT_TO_DESC_FIELD(width, uint32_t)
KALI_DICT_TO_DESC_FIELD(height, uint32_t)
KALI_DICT_TO_DESC_FIELD(depth, uint32_t)
KALI_DICT_TO_DESC_FIELD(array_size, uint32_t)
KALI_DICT_TO_DESC_FIELD(mip_count, uint32_t)
KALI_DICT_TO_DESC_FIELD(sample_count, uint32_t)
KALI_DICT_TO_DESC_FIELD(quality, uint32_t)
KALI_DICT_TO_DESC_FIELD(initial_state, ResourceState)
KALI_DICT_TO_DESC_FIELD(usage, ResourceUsage)
KALI_DICT_TO_DESC_FIELD(memory_type, MemoryType)
KALI_DICT_TO_DESC_FIELD(debug_name, std::string)
KALI_DICT_TO_DESC_END()


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
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::UInt, (uint8_t)channel_bit_count, 1};
    case FormatType::sint:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Int, (uint8_t)channel_bit_count, 1};
    default:
        return {};
    }
}

static const char* __doc_kali_buffer_to_numpy = R"doc()doc";

inline nb::ndarray<nb::numpy> buffer_to_numpy(Buffer* self)
{
    size_t buffer_size = self->size();
    void* cpu_data = new uint8_t[buffer_size];

    // TODO should we do this on the command stream instead?
    self->device()->read_buffer(self, 0, buffer_size, cpu_data);

    nb::capsule owner(cpu_data, [](void* p) noexcept { delete[] reinterpret_cast<uint8_t*>(p); });

    if (auto dtype = resource_format_to_dtype(self->format())) {
        uint32_t channel_count = get_format_info(self->format()).channel_count;
        if (channel_count == 1) {
            size_t shape[1] = {self->element_count()};
            return nb::ndarray<nb::numpy>(cpu_data, 1, shape, owner, nullptr, *dtype, nb::device::cpu::value);
        } else {
            size_t shape[2] = {self->element_count(), channel_count};
            return nb::ndarray<nb::numpy>(cpu_data, 2, shape, owner, nullptr, *dtype, nb::device::cpu::value);
        }
    } else {
        size_t shape[1] = {buffer_size};
        return nb::ndarray<nb::numpy>(cpu_data, 1, shape, owner, nullptr, nb::dtype<uint8_t>(), nb::device::cpu::value);
    }
}

static const char* __doc_kali_buffer_from_numpy = R"doc()doc";

inline void buffer_from_numpy(Buffer* self, nb::ndarray<nb::numpy> data)
{
    KALI_CHECK(is_ndarray_contiguous(data), "numpy array is not contiguous");

    size_t buffer_size = self->size();
    size_t data_size = data.nbytes();
    KALI_CHECK(data_size <= buffer_size, "numpy array is larger than the buffer ({} > {})", data_size, buffer_size);

    // TODO use convenience function on buffer when available
    ref<CommandBuffer> command_buffer = self->device()->create_command_buffer();
    command_buffer->upload_buffer_data(self, 0, data_size, data.data());
    command_buffer->submit();
}

static const char* __doc_kali_texture_to_numpy = R"doc()doc";

/**
 * Python binding wrapper for returning the content of a texture as a numpy array.
 */
inline nb::ndarray<nb::numpy> texture_to_numpy(Texture* self, uint32_t mip_level, uint32_t array_slice)
{
    KALI_CHECK_LT(mip_level, self->mip_count());
    KALI_CHECK_LT(array_slice, self->array_size());

    // Get image dimensions.
    uint3 dimensions = self->get_mip_dimensions(mip_level);

    uint32_t subresource = self->get_subresource_index(array_slice, mip_level);
    SubresourceLayout layout = self->get_subresource_layout(subresource);

    size_t size = layout.total_size_aligned();
    void* data = new uint8_t[size];
    size_t row_pitch, pixel_size;
    self->device()->read_texture(self, size, data, &row_pitch, &pixel_size);

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

static const char* __doc_kali_texture_from_numpy = R"doc()doc";

inline void texture_from_numpy(Texture* self, nb::ndarray<nb::numpy> data, uint32_t mip_level, uint32_t array_slice)
{
    KALI_CHECK(is_ndarray_contiguous(data), "numpy array is not contiguous");
    KALI_CHECK_LT(mip_level, self->mip_count());
    KALI_CHECK_LT(array_slice, self->array_size());

    uint32_t subresource = self->get_subresource_index(array_slice, mip_level);
    SubresourceLayout layout = self->get_subresource_layout(subresource);

    size_t subresource_size = layout.total_size_aligned();
    size_t data_size = data.nbytes();
    KALI_CHECK(
        data_size == subresource_size,
        "numpy array is doesn't match the subresource size ({} != {})",
        data_size,
        subresource_size
    );

    // TODO use convenience function on texture when available
    ref<CommandBuffer> command_buffer = self->device()->create_command_buffer();
    command_buffer->upload_texture_data(self, subresource, data.data());
    command_buffer->submit();
}

} // namespace kali

KALI_PY_EXPORT(device_resource)
{
    using namespace kali;

    nb::kali_enum<ResourceType>(m, "ResourceType");
    nb::kali_enum<ResourceState>(m, "ResourceState");

    nb::kali_enum_flags<ResourceUsage>(m, "ResourceUsage");

    nb::kali_enum<MemoryType>(m, "MemoryType");

    nb::class_<Resource, DeviceResource>(m, "Resource")
        .def("get_srv", &Resource::get_srv)
        .def("get_uav", &Resource::get_uav);

    nb::kali_enum<ResourceViewType>(m, "ResourceViewType");

    nb::class_<ResourceView, Object>(m, "ResourceView", D(ResourceView))
        .def_prop_ro("type", &ResourceView::type, D(ResourceView, type))
        .def_prop_ro("resource", &ResourceView::resource, D(ResourceView, resource));

    nb::class_<BufferDesc>(m, "BufferDesc")
        .def(nb::init<>())
        .def("__init__", [](BufferDesc* self, nb::dict dict) { new (self) BufferDesc(dict_to_BufferDesc(dict)); })
        .def_rw("size", &BufferDesc::size)
        .def_rw("struct_size", &BufferDesc::struct_size)
        .def_rw("format", &BufferDesc::format)
        .def_rw("initial_state", &BufferDesc::initial_state)
        .def_rw("usage", &BufferDesc::usage)
        .def_rw("memory_type", &BufferDesc::memory_type)
        .def_rw("debug_name", &BufferDesc::debug_name);
    nb::implicitly_convertible<nb::dict, BufferDesc>();

    nb::class_<Buffer, Resource>(m, "Buffer", D(Buffer))
        .def_prop_ro("desc", &Buffer::desc, D(Buffer, desc))
        .def_prop_ro("size", &Buffer::size, D(Buffer, size))
        .def_prop_ro("struct_size", &Buffer::struct_size, D(Buffer, struct_size))
        .def_prop_ro("format", &Buffer::format, D(Buffer, format))
        .def_prop_ro("element_size", &Buffer::element_size, D(Buffer, element_size))
        .def_prop_ro("element_count", &Buffer::element_count, D(Buffer, element_count))
        .def_prop_ro("device_address", &Buffer::device_address, D(Buffer, device_address))
        .def(
            "get_srv",
            nb::overload_cast<uint64_t, uint64_t>(&Buffer::get_srv, nb::const_),
            "first_element"_a = 0,
            "element_count"_a = BufferRange::ALL,
            D(Buffer, get_srv)
        )
        .def(
            "get_uav",
            nb::overload_cast<uint64_t, uint64_t>(&Buffer::get_uav, nb::const_),
            "first_element"_a = 0,
            "element_count"_a = BufferRange::ALL,
            D(Buffer, get_uav)
        )
        .def("to_numpy", &buffer_to_numpy, D(buffer_to_numpy))
        .def("from_numpy", &buffer_from_numpy, "data"_a, D(buffer_from_numpy));

    nb::class_<TextureDesc>(m, "TextureDesc")
        .def(nb::init<>())
        .def("__init__", [](TextureDesc* self, nb::dict dict) { new (self) TextureDesc(dict_to_TextureDesc(dict)); })
        .def_rw("type", &TextureDesc::type)
        .def_rw("format", &TextureDesc::format)
        .def_rw("width", &TextureDesc::width)
        .def_rw("height", &TextureDesc::height)
        .def_rw("depth", &TextureDesc::depth)
        .def_rw("array_size", &TextureDesc::array_size)
        .def_rw("mip_count", &TextureDesc::mip_count)
        .def_rw("sample_count", &TextureDesc::sample_count)
        .def_rw("quality", &TextureDesc::quality)
        .def_rw("initial_state", &TextureDesc::initial_state)
        .def_rw("usage", &TextureDesc::usage)
        .def_rw("memory_type", &TextureDesc::memory_type)
        .def_rw("debug_name", &TextureDesc::debug_name);
    nb::implicitly_convertible<nb::dict, TextureDesc>();

    nb::class_<SubresourceLayout>(m, "SubresourceLayout")
        .def_ro("row_size", &SubresourceLayout::row_size)
        .def_ro("row_size_aligned", &SubresourceLayout::row_size_aligned)
        .def_ro("row_count", &SubresourceLayout::row_count)
        .def_ro("depth", &SubresourceLayout::depth)
        .def_prop_ro("total_size", &SubresourceLayout::total_size)
        .def_prop_ro("total_size_aligned", &SubresourceLayout::total_size_aligned);

    nb::class_<Texture, Resource>(m, "Texture", D(Texture))
        .def_prop_ro("desc", &Texture::desc, D(Texture, desc))
        .def_prop_ro("format", &Texture::format, D(Texture, format))
        .def_prop_ro("width", &Texture::width, D(Texture, width))
        .def_prop_ro("height", &Texture::height, D(Texture, height))
        .def_prop_ro("depth", &Texture::depth, D(Texture, depth))
        .def_prop_ro("array_size", &Texture::array_size, D(Texture, array_size))
        .def_prop_ro("mip_count", &Texture::mip_count, D(Texture, mip_count))
        .def_prop_ro("subresource_count", &Texture::subresource_count, D(Texture, subresource_count))
        .def(
            "get_subresource_index",
            &Texture::get_subresource_index,
            "array_slice"_a,
            "mip_level"_a = 0,
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
            nb::overload_cast<uint32_t, uint32_t, uint32_t, uint32_t>(&Texture::get_srv, nb::const_),
            "mip_level"_a = 0,
            "mip_count"_a = SubresourceRange::ALL,
            "base_array_layer"_a = 0,
            "layer_count"_a = SubresourceRange::ALL,
            D(Texture, get_srv)
        )
        .def(
            "get_uav",
            nb::overload_cast<uint32_t, uint32_t, uint32_t>(&Texture::get_uav, nb::const_),
            "mip_level"_a = 0,
            "base_array_layer"_a = 0,
            "layer_count"_a = SubresourceRange::ALL,
            D(Texture, get_uav)
        )
        .def(
            "get_dsv",
            &Texture::get_dsv,
            "mip_level"_a = 0,
            "base_array_layer"_a = 0,
            "layer_count"_a = SubresourceRange::ALL,
            D(Texture, get_dsv)
        )
        .def(
            "get_rtv",
            &Texture::get_rtv,
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
