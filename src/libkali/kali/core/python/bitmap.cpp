#include "nanobind.h"

#include "kali/core/bitmap.h"

#include <bit>

KALI_PY_EXPORT(core_bitmap)
{
    using namespace kali;

    nb::class_<Bitmap, Object> bitmap(m, "Bitmap");

    nb::kali_enum<Bitmap::PixelFormat>(bitmap, "PixelFormat");
    bitmap.attr("ComponentType") = m.attr("Struct").attr("Type");
    nb::kali_enum<Bitmap::FileFormat>(bitmap, "FileFormat");

    bitmap //
        .def(
            "__init__",
            [](Bitmap* self,
               Bitmap::PixelFormat pixel_format,
               Bitmap::ComponentType component_type,
               uint32_t width,
               uint32_t height,
               uint32_t channel_count,
               std::vector<std::string> channel_names)
            { new (self) Bitmap(pixel_format, component_type, width, height, channel_count, channel_names); },
            "pixel_format"_a,
            "component_type"_a,
            "width"_a,
            "height"_a,
            "channel_count"_a = 0,
            "channel_names"_a = std::vector<std::string>{}
        )
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
        .def(
            "__init__",
            [](Bitmap* self, const std::filesystem::path& path) { new (self) Bitmap(path); },
            "path"_a
        )
        .def_prop_ro("pixel_format", &Bitmap::pixel_format)
        .def_prop_ro("component_type", &Bitmap::component_type)
        .def_prop_ro("width", &Bitmap::width)
        .def_prop_ro("height", &Bitmap::height)
        .def_prop_ro("pixel_count", &Bitmap::pixel_count)
        .def_prop_ro("channel_count", &Bitmap::channel_count)
        .def_prop_ro("channel_names", &Bitmap::channel_names)
        .def_prop_rw("srgb_gamma", &Bitmap::srgb_gamma, &Bitmap::set_srgb_gamma)
        .def("has_alpha", &Bitmap::has_alpha)
        .def_prop_ro("bytes_per_pixel", &Bitmap::bytes_per_pixel)
        .def_prop_ro("buffer_size", &Bitmap::buffer_size)
        .def("clear", &Bitmap::clear)
        .def("vflip", &Bitmap::vflip)
        .def(
            "convert",
            [](Bitmap& self,
               std::optional<Bitmap::PixelFormat> pixel_format,
               std::optional<Bitmap::ComponentType> component_type,
               std::optional<bool> srgb_gamma) -> ref<Bitmap>
            {
                return self.convert(
                    pixel_format ? *pixel_format : self.pixel_format(),
                    component_type ? *component_type : self.component_type(),
                    srgb_gamma ? *srgb_gamma : self.srgb_gamma()
                );
            },
            "pixel_format"_a = std::optional<Bitmap::PixelFormat>{},
            "component_type"_a = std::optional<Bitmap::ComponentType>{},
            "srgb_gamma"_a = std::optional<bool>{}
        )
        .def(
            "write",
            nb::overload_cast<const std::filesystem::path&, Bitmap::FileFormat, int>(&Bitmap::write, nb::const_),
            "path"_a,
            "format"_a = Bitmap::FileFormat::auto_,
            "quality"_a = -1
        )
        .def("write_async", &Bitmap::write_async, "path"_a, "format"_a = Bitmap::FileFormat::auto_, "quality"_a = -1)
        .def(nb::self == nb::self)
        .def(nb::self != nb::self)
        .def_prop_ro(
            "__array_interface__",
            [](Bitmap& self) -> nb::object
            {
                if (self.empty())
                    return nb::none();

                nb::dict result;
                if (self.channel_count() == 1)
                    result["shape"] = nb::make_tuple(self.height(), self.width());
                else
                    result["shape"] = nb::make_tuple(self.height(), self.width(), self.channel_count());

                std::string format(3, '\0');
                format[0] = std::endian::native == std::endian::little ? '<' : '>';
                format[1] = Struct::is_float(self.component_type())
                    ? 'f'
                    : (Struct::is_unsigned(self.component_type()) ? 'u' : 'i');
                format[2] = '0' + static_cast<char>(Struct::type_size(self.component_type()));
                result["typestr"] = format;

                result["data"] = nb::make_tuple(reinterpret_cast<uintptr_t>(self.data()), false);
                result["version"] = 3;

                return nb::object(result);
            }
        );
}
