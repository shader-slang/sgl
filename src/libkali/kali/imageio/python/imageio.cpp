#include "nanobind.h"

#include "kali/imageio/imageio.h"

#include "kali/core/error.h"
#include "kali/core/type_utils.h"

namespace kali {

inline ImageComponentType dtype_to_image_component_type(nb::dlpack::dtype dtype)
{
    switch (nb::dlpack::dtype_code(dtype.code)) {
    case nb::dlpack::dtype_code::UInt:
        switch (dtype.bits) {
        case 8:
            return ImageComponentType::u8;
        case 16:
            return ImageComponentType::u16;
        case 32:
            return ImageComponentType::u32;
            break;
        }
        break;
    case nb::dlpack::dtype_code::Float:
        switch (dtype.bits) {
        case 16:
            return ImageComponentType::f16;
        case 32:
            return ImageComponentType::f32;
            break;
        }
        break;
    default:
        break;
    }

    return ImageComponentType::unknown;
}

inline nanobind::dlpack::dtype image_component_type_to_dtype(ImageComponentType type)
{
    switch (type) {
    case ImageComponentType::unknown:
        return {};
    case ImageComponentType::u8:
        return nb::dlpack::dtype{uint8_t(nb::dlpack::dtype_code::UInt), 8, 1};
    case ImageComponentType::u16:
        return nb::dlpack::dtype{uint8_t(nb::dlpack::dtype_code::UInt), 16, 1};
    case ImageComponentType::u32:
        return nb::dlpack::dtype{uint8_t(nb::dlpack::dtype_code::UInt), 32, 1};
    case ImageComponentType::f16:
        return nb::dlpack::dtype{uint8_t(nb::dlpack::dtype_code::Float), 16, 1};
    case ImageComponentType::f32:
        return nb::dlpack::dtype{uint8_t(nb::dlpack::dtype_code::Float), 32, 1};
    }
    return {};
}

void register_kali_imageio(nb::module_& m)
{
    nb::enum_<ImageComponentType>(m, "ImageComponentType")
        .value("unknown", ImageComponentType::unknown)
        .value("u8", ImageComponentType::u8)
        .value("u16", ImageComponentType::u16)
        .value("u32", ImageComponentType::u32)
        .value("f16", ImageComponentType::f16)
        .value("f32", ImageComponentType::f32);

    nb::class_<ImageSpec>(m, "ImageSpec")
        .def_rw("width", &ImageSpec::width)
        .def_rw("height", &ImageSpec::height)
        .def_rw("component_count", &ImageSpec::component_count)
        .def_rw("component_type", &ImageSpec::component_type);

    nb::class_<ImageInput, Object>(m, "ImageInput")
        .def_static(
            "open",
            nb::overload_cast<const std::filesystem::path&, ImageInput::Options>(&ImageInput::open),
            "path"_a,
            "options"_a
        )
        .def_prop_ro("spec", &ImageInput::get_spec);

    nb::class_<ImageOutput, Object>(m, "ImageOutput")
        .def_static(
            "open",
            nb::overload_cast<const std::filesystem::path&, ImageSpec, ImageOutput::Options>(&ImageOutput::open),
            "path"_a,
            "spec"_a,
            "options"_a
        );

    m.def(
        "read_image",
        [](const std::filesystem::path& path) -> nb::ndarray<nb::numpy>
        {
            ref<ImageInput> input = ImageInput::open(path);

            const ImageSpec& spec = input->get_spec();

            size_t image_size = spec.get_image_byte_size();
            std::unique_ptr<uint8_t[]> image_data(new uint8_t[image_size]);
            if (!image_data)
                KALI_THROW("Failed to allocate memory for image");

            input->read_image(image_data.get(), image_size);

            size_t ndim = spec.component_count > 1 ? 3 : 2;
            size_t shape[3] = {spec.height, spec.width, spec.component_count};
            void* owned_data = image_data.release();
            nb::capsule owner(owned_data, [](void* ptr) noexcept { delete[] static_cast<uint8_t*>(ptr); });
            nb::dlpack::dtype dtype = image_component_type_to_dtype(spec.component_type);
            return nb::ndarray<nb::numpy>(owned_data, ndim, shape, owner, nullptr, dtype);
        },
        "path"_a
    );

    m.def(
        "write_image",
        [](const std::filesystem::path& path, const nb::ndarray<nb::numpy>& image) -> void
        {
            if (image.ndim() != 2 && image.ndim() != 3)
                KALI_THROW("Image dimensions must be 2 or 3");
            if (image.shape(0) == 0 || image.shape(1) == 0 || (image.ndim() > 2 && image.shape(2) == 0))
                KALI_THROW("Image dimensions must be non-zero");

            ImageSpec spec;
            spec.width = narrow_cast<uint32_t>(image.shape(1));
            spec.height = narrow_cast<uint32_t>(image.shape(0));
            spec.component_count = image.ndim() > 2 ? narrow_cast<uint32_t>(image.shape(2)) : 1;
            spec.component_type = dtype_to_image_component_type(image.dtype());

            if (spec.component_type == ImageComponentType::unknown)
                KALI_THROW("Image dtype is not supported");

            ref<ImageOutput> output = ImageOutput::open(path, spec);

            // TODO convert image layout if not column-major

            output->write_image(image.data(), image.nbytes());
        },
        "path"_a,
        "image"_a
    );
}

} // namespace kali