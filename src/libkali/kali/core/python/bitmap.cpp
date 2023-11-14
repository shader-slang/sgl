#include "nanobind.h"

#include "kali/core/bitmap.h"

KALI_PY_EXPORT(core_bitmap)
{
    using namespace kali;

    nb::class_<Bitmap, Object> bitmap(m, "Bitmap");

    nb::kali_enum<Bitmap::FileFormat>(bitmap, "FileFormat");

    bitmap //
        .def(
            "__init__",
            [](Bitmap* self,
               nb::ndarray<nb::device::cpu> data,
               std::optional<Bitmap::PixelFormat> pixel_format_,
               std::optional<std::vector<std::string>> channel_names_)
            {
                if (data.ndim() != 2 && data.ndim() != 3)
                    KALI_THROW("Expect array with dimension 2 or 3.");

                Struct::Type component_type;
                if (auto struct_type = dtype_to_struct_type(data.dtype()))
                    component_type = *struct_type;
                else
                    KALI_THROW("Unsupported data type.");

                Bitmap::PixelFormat pixel_format = Bitmap::PixelFormat::y;
                uint32_t channel_count = 1;
                if (data.ndim() == 3) {
                    channel_count = narrow_cast<uint32_t>(data.shape(2));
                    switch (channel_count) {
                    case 1:
                        pixel_format = Bitmap::PixelFormat::y;
                        break;
                    case 2:
                        pixel_format = Bitmap::PixelFormat::ya;
                        break;
                    case 3:
                        pixel_format = Bitmap::PixelFormat::rgb;
                        break;
                    case 4:
                        pixel_format = Bitmap::PixelFormat::rgba;
                        break;
                    default:
                        pixel_format = Bitmap::PixelFormat::multi_channel;
                        break;
                    }
                }

                pixel_format = pixel_format_.value_or(pixel_format);

                uint32_t width = narrow_cast<uint32_t>(data.shape(1));
                uint32_t height = narrow_cast<uint32_t>(data.shape(0));
                std::vector channel_names = channel_names_.value_or(std::vector<std::string>(channel_count, ""));

                new (self) Bitmap(pixel_format, component_type, width, height, channel_count, channel_names);

                KALI_ASSERT(self->buffer_size() == data.nbytes());

                if (is_ndarray_contiguous(data)) {
                    std::memcpy(self->data(), data.data(), self->buffer_size());
                } else {
                    KALI_THROW("data is not contiguous.");
                }
            },
            "data"_a,
            "pixel_format"_a = std::optional<Bitmap::PixelFormat>{},
            "channel_names"_a = std::optional<std::vector<std::string>>{}
        )
        .def_prop_ro("pixel_format", &Bitmap::pixel_format)
        .def_prop_ro("component_type", &Bitmap::component_type)
        .def_prop_ro("width", &Bitmap::width)
        .def_prop_ro("height", &Bitmap::height)
        .def_prop_ro("pixel_count", &Bitmap::pixel_count)
        .def_prop_ro("channel_count", &Bitmap::channel_count)
        .def("has_alpha", &Bitmap::has_alpha)
        .def_prop_ro("bytes_per_pixel", &Bitmap::bytes_per_pixel)
        .def_prop_ro("buffer_size", &Bitmap::buffer_size)
        .def("clear", &Bitmap::clear)
        .def("vflip", &Bitmap::vflip)
        .def(
            "write",
            nb::overload_cast<const std::filesystem::path&, Bitmap::FileFormat, int>(&Bitmap::write, nb::const_),
            "path"_a,
            "format"_a = Bitmap::FileFormat::auto_,
            "quality"_a = -1
        );
}
