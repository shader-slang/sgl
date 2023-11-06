#include "nanobind.h"

#include "kali/device/device.h"
#include "kali/device/resource.h"
#include "kali/device/formats.h"
#include "kali/device/command.h"

namespace kali {

inline std::optional<nb::dlpack::dtype> resource_format_to_dtype(Format format)
{
    const auto& info = get_format_info(format);

    // Unknown and compressed formats are not supported.
    if (format == Format::unknown || info.is_compressed)
        return {};

    // Formats with different bits per channel are not supported.
    uint32_t channel_bit_count = info.channel_bit_count[0];
    for (uint32_t i = 1; i < info.channel_count; ++i)
        if (channel_bit_count == info.channel_bit_count[i])
            return {};

    // Only formats with 8, 16, 32, or 64 bits per channel are supported.
    if (channel_bit_count != 8 && channel_bit_count != 16 && channel_bit_count != 32 && channel_bit_count != 64)
        return {};

    switch (info.type)
    {
    case FormatType::float_:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Float, (uint8_t)channel_bit_count, 1};
    case FormatType::uint:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::UInt, (uint8_t)channel_bit_count, 1};
    case FormatType::sint:
        return nb::dlpack::dtype{(uint8_t)nb::dlpack::dtype_code::Int, (uint8_t)channel_bit_count, 1};
    }

    return {};
}

inline nb::ndarray<nb::numpy> buffer_to_numpy(Buffer* self)
{
    size_t buffer_size = self->size();
    void* cpu_data = new uint8_t[buffer_size];

    // TODO should we do this on the command stream instead?
    self->device()->read_buffer(self, 0, buffer_size, cpu_data);

    nb::capsule owner(cpu_data, [](void* p) noexcept { delete[] reinterpret_cast<uint8_t*>(p); });

    if (auto dtype = resource_format_to_dtype(self->format()))
    {
        uint32_t channel_count = get_format_info(self->format()).channel_count;
        if (channel_count == 1)
        {
            size_t shape[1] = {self->element_count()};
            return nb::ndarray<nb::numpy>(cpu_data, 1, shape, owner, nullptr, *dtype, nb::device::cpu::value);
        }
        else
        {
            size_t shape[2] = {self->element_count(), channel_count};
            return nb::ndarray<nb::numpy>(cpu_data, 2, shape, owner, nullptr, *dtype, nb::device::cpu::value);
        }
    }
    else
    {
        size_t shape[1] = {buffer_size};
        return nb::ndarray<nb::numpy>(
            cpu_data, 1, shape, owner, nullptr, nb::dtype<uint8_t>(), nb::device::cpu::value
        );
    }
}

inline void buffer_from_numpy(Buffer* self, nb::ndarray<nb::numpy> data)
{
    KALI_CHECK(is_ndarray_contiguous(data), "numpy array is not contiguous");

    size_t buffer_size = self->size();
    size_t data_size = data.nbytes();
    KALI_CHECK(data_size <= buffer_size, "numpy array is larger than the buffer ({} > {})", data_size, buffer_size);

    self->device()->command_stream()->upload_buffer_data(self, 0, data_size, data.data());
}

} // namespace kali

KALI_PY_EXPORT(device_resource)
{
    using namespace kali;

    nb::kali_enum<ResourceType>(m, "ResourceType");
    nb::kali_enum<ResourceState>(m, "ResourceState");

    nb::kali_enum<ResourceUsage>(m, "ResourceUsage")
        .def("__and__", [](ResourceUsage value1, ResourceUsage value2) { return value1 & value2; })
        .def("__or__", [](ResourceUsage value1, ResourceUsage value2) { return value1 | value2; });

    nb::kali_enum<MemoryType>(m, "MemoryType");

    nb::class_<Resource, Object>(m, "Resource").def("device_address", &Resource::get_device_address);

    nb::class_<BufferDesc>(m, "BufferDesc")
        .def_rw("size", &BufferDesc::size)
        .def_rw("struct_size", &BufferDesc::struct_size)
        .def_rw("format", &BufferDesc::format)
        .def_rw("initial_state", &BufferDesc::initial_state)
        .def_rw("usage", &BufferDesc::usage)
        .def_rw("memory_type", &BufferDesc::memory_type)
        .def_rw("debug_name", &BufferDesc::debug_name);

    nb::class_<Buffer, Resource>(m, "Buffer")
        .def(
            "__init__",
            [](Buffer* self,
               ref<Device> device,
               size_t size,
               size_t struct_size,
               Format format,
               ResourceState initial_state,
               ResourceUsage usage,
               MemoryType memory_type,
               std::string debug_name)
            {
                new (self) Buffer(
                    std::move(device),
                    BufferDesc{
                        .size = size,
                        .struct_size = struct_size,
                        .format = format,
                        .initial_state = initial_state,
                        .usage = usage,
                        .memory_type = memory_type,
                        .debug_name = std::move(debug_name),
                    },
                    nullptr
                );
            },
            "device"_a,
            "size"_a = 0,
            "struct_size"_a = 0,
            "format"_a = Format::unknown,
            "initial_state"_a = ResourceState::undefined,
            "usage"_a = ResourceUsage::none,
            "memory_type"_a = MemoryType::device_local,
            "debug_name"_a = ""
        )
        .def_prop_ro("desc", &Buffer::desc)
        .def_prop_ro("size", &Buffer::size)
        .def_prop_ro("struct_size", &Buffer::struct_size)
        .def_prop_ro("format", &Buffer::format)
        .def("to_numpy", &buffer_to_numpy)
        .def("from_numpy", &buffer_from_numpy);

    nb::kali_enum<TextureType>(m, "TextureType");

    nb::class_<TextureDesc>(m, "TextureDesc")
        .def_rw("type", &TextureDesc::type)
        .def_rw("width", &TextureDesc::width)
        .def_rw("height", &TextureDesc::height)
        .def_rw("depth", &TextureDesc::depth)
        .def_rw("array_size", &TextureDesc::array_size)
        .def_rw("mip_count", &TextureDesc::mip_count)
        .def_rw("format", &TextureDesc::format)
        .def_rw("initial_state", &TextureDesc::initial_state)
        .def_rw("usage", &TextureDesc::usage)
        .def_rw("memory_type", &TextureDesc::memory_type)
        .def_rw("debug_name", &TextureDesc::debug_name);

    nb::class_<Texture, Resource>(m, "Texture");
}
