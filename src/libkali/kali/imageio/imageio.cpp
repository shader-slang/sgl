#include "imageio.h"

#include "kali/core/macros.h"
#include "kali/core/error.h"
#include "kali/core/string_utils.h"
#include "kali/core/type_utils.h"
#include "kali/core/platform.h"
#include "kali/core/file_stream.h"

#include <cstdio> // must be included before jpeglib.h
// #include <jpeglib.h>
// #include <png.h>
// #include <TinyEXIF.h>

#include <cctype>
#include <cstring>
#include <string>
#include <fstream>

KALI_DIAGNOSTIC_PUSH
KALI_DISABLE_MSVC_WARNING(4244 4996)
KALI_DISABLE_CLANG_WARNING("-Wmissing-field-initializers")
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
KALI_DIAGNOSTIC_POP

KALI_DIAGNOSTIC_PUSH
KALI_DISABLE_MSVC_WARNING(4706)
#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>
KALI_DIAGNOSTIC_POP

namespace kali {

static ImageFileFormat extension_to_format(std::string_view ext)
{
    if (ext == "png")
        return ImageFileFormat::png;
    else if (ext == "jpg" || ext == "jpeg")
        return ImageFileFormat::jpg;
    else if (ext == "bmp")
        return ImageFileFormat::bmp;
    else if (ext == "tga")
        return ImageFileFormat::tga;
    else if (ext == "hdr")
        return ImageFileFormat::hdr;
    else if (ext == "exr")
        return ImageFileFormat::exr;
    else
        KALI_THROW("Unknown image file extension '{}'", ext);
}

// ----------------------------------------------------------------------------
// ImageReader
// ----------------------------------------------------------------------------

class ImageReader {
public:
    ImageReader(ref<Stream> stream, ImageFileFormat file_format)
        : m_stream(stream)
        , m_file_format(file_format)
    {
    }

    virtual ~ImageReader() { }

    virtual ImageSpec read_spec() = 0;
    virtual void read_image(const ImageSpec& spec, void* out_buffer, size_t out_len) = 0;

protected:
    ref<Stream> m_stream;
    ImageFileFormat m_file_format;
};

class ImageWriter {
public:
    ImageWriter(ref<Stream> stream, ImageFileFormat file_format)
        : m_stream(stream)
        , m_file_format(file_format)
    {
    }

    virtual ~ImageWriter() { }

    virtual void write_spec(const ImageSpec& spec) = 0;
    virtual void write_image(const ImageSpec& spec, const void* buffer, size_t len) = 0;

protected:
    ref<Stream> m_stream;
    ImageFileFormat m_file_format;
};

// ----------------------------------------------------------------------------
// StbImageReader
// ----------------------------------------------------------------------------

class StbImageReader : public ImageReader {
public:
    StbImageReader(ref<Stream> stream, ImageFileFormat file_format)
        : ImageReader(stream, file_format)
    {
        m_callbacks.read = [](void* user, char* data, int size) -> int
        {
            StbImageReader* self = reinterpret_cast<StbImageReader*>(user);
            try {
                self->m_stream->read(data, size);
            } catch (const EOFException&) {
                self->m_eof = true;
            }
            return size;
        };
        m_callbacks.skip = [](void* user, int n)
        {
            StbImageReader* self = reinterpret_cast<StbImageReader*>(user);
            self->m_stream->seek(self->m_stream->tell() + n);
        };
        m_callbacks.eof = [](void* user) -> int
        {
            StbImageReader* self = reinterpret_cast<StbImageReader*>(user);
            return self->m_eof;
        };
    }

    ~StbImageReader() { }

    ImageSpec read_spec() override
    {
        int width, height, component_count;
        if (!stbi_info_from_callbacks(&m_callbacks, this, &width, &height, &component_count))
            KALI_THROW("Failed to read image spec");

        bool is_hdr = stbi_is_hdr_from_callbacks(&m_callbacks, this);
        bool is_16bit = stbi_is_16_bit_from_callbacks(&m_callbacks, this);

        return {
            .width = narrow_cast<uint32_t>(width),
            .height = narrow_cast<uint32_t>(height),
            .component_count = narrow_cast<uint32_t>(component_count),
            .component_type
            = is_hdr ? ImageComponentType::f32 : (is_16bit ? ImageComponentType::u16 : ImageComponentType::u8),
        };
    }

    void read_image(const ImageSpec& spec, void* out_buffer, size_t out_len) override
    {
        KALI_ASSERT(spec.width > 0);
        KALI_ASSERT(spec.height > 0);
        KALI_ASSERT(spec.component_count > 0);
        KALI_ASSERT(spec.component_type != ImageComponentType::unknown);

        m_stream->seek(0);

        int width, height, component_count;
        uint8_t* pixels{nullptr};
        size_t read_len{0};
        switch (spec.component_type) {
        case ImageComponentType::u8:
            pixels = reinterpret_cast<uint8_t*>(
                stbi_load_from_callbacks(&m_callbacks, this, &width, &height, &component_count, 0)
            );
            read_len = width * height * component_count;
            break;
        case ImageComponentType::u16:
            pixels = reinterpret_cast<uint8_t*>(
                stbi_load_16_from_callbacks(&m_callbacks, this, &width, &height, &component_count, 0)
            );
            read_len = width * height * component_count * 2;
            break;
        case ImageComponentType::f32:
            pixels = reinterpret_cast<uint8_t*>(
                stbi_loadf_from_callbacks(&m_callbacks, this, &width, &height, &component_count, 0)
            );
            read_len = width * height * component_count * 4;
            break;
        default:
            KALI_ASSERT(false);
            break;
        }

        if (!pixels)
            KALI_THROW("Failed to read image data");

        size_t expected_len = spec.get_image_byte_size();
        KALI_ASSERT_EQ(out_len, expected_len);
        KALI_ASSERT_EQ(read_len, expected_len);

        std::memcpy(out_buffer, pixels, out_len);

        stbi_image_free(pixels);
    }

private:
    stbi_io_callbacks m_callbacks;
    bool m_eof{false};
};

// ----------------------------------------------------------------------------
// StbImageWriter
// ----------------------------------------------------------------------------

class StbImageWriter : public ImageWriter {
public:
    StbImageWriter(ref<Stream> stream, ImageFileFormat file_format)
        : ImageWriter(stream, file_format)
    {
    }

    ~StbImageWriter() { }

    void write_spec(const ImageSpec& spec) override
    {
        switch (m_file_format) {
        case ImageFileFormat::png:
            if (spec.component_type != ImageComponentType::u8)
                KALI_THROW("Unsupported component type for PNG");
            break;
        case ImageFileFormat::jpg:
            if (spec.component_type != ImageComponentType::u8)
                KALI_THROW("Unsupported component type for JPG");
            break;
        case ImageFileFormat::bmp:
            if (spec.component_type != ImageComponentType::u8)
                KALI_THROW("Unsupported component type for BMP");
            break;
        case ImageFileFormat::tga:
            if (spec.component_type != ImageComponentType::u8 && spec.component_type != ImageComponentType::u16)
                KALI_THROW("Unsupported component type for TGA");
            break;
        case ImageFileFormat::hdr:
            if (spec.component_type != ImageComponentType::f32)
                KALI_THROW("Unsupported component type for HDR");
            break;
        default:
            KALI_THROW("Unknown file extension");
        }
    }

    void write_image(const ImageSpec& spec, const void* buffer, size_t len) override
    {
        KALI_UNUSED(len);

        size_t expected_len = spec.get_image_byte_size();
        KALI_ASSERT_EQ(len, expected_len);

        int width = narrow_cast<int>(spec.width);
        int height = narrow_cast<int>(spec.height);
        int component_count = narrow_cast<int>(spec.component_count);

        auto write_func = [](void* context, void* data, int size)
        {
            Stream* stream = reinterpret_cast<Stream*>(context);
            stream->write(reinterpret_cast<char*>(data), size);
        };

        bool success{false};
        switch (m_file_format) {
        case ImageFileFormat::png:
            success = stbi_write_png_to_func(write_func, m_stream.get(), width, height, component_count, buffer, 0);
            break;
        case ImageFileFormat::jpg:
            success = stbi_write_jpg_to_func(write_func, m_stream.get(), width, height, component_count, buffer, 100);
            break;
        case ImageFileFormat::bmp:
            success = stbi_write_bmp_to_func(write_func, m_stream.get(), width, height, component_count, buffer);
            break;
        case ImageFileFormat::tga:
            success = stbi_write_tga_to_func(write_func, m_stream.get(), width, height, component_count, buffer);
            break;
        case ImageFileFormat::hdr:
            success = stbi_write_hdr_to_func(
                write_func,
                &m_stream,
                width,
                height,
                component_count,
                reinterpret_cast<const float*>(buffer)
            );
            break;
        default:
            KALI_UNREACHABLE();
        }
    }
};

#if 0

// ----------------------------------------------------------------------------
// TinyExrImageReader
// ----------------------------------------------------------------------------

class TinyExrImageReader : public ImageReader {
public:
    ~TinyExrImageReader() { }

    ImageSpec read_spec(const void* buffer, size_t len) override
    {
        const uint8_t* memory = reinterpret_cast<const uint8_t*>(buffer);

        EXRVersion exr_version;
        if (ParseEXRVersionFromMemory(&exr_version, memory, len) != TINYEXR_SUCCESS)
            KALI_THROW("Failed to parse EXR version");

        if (exr_version.multipart)
            KALI_THROW("Multipart EXR is not supported");

        EXRHeader exr_header;
        const char* err;
        if (ParseEXRHeaderFromMemory(&exr_header, &exr_version, memory, len, &err) != TINYEXR_SUCCESS) {
            std::string err_copy = err;
            FreeEXRErrorMessage(err);
            KALI_THROW("Failed to parse EXR header: {}", err_copy);
        }

        EXRImage exr_image;
        InitEXRImage(&exr_image);

        if (LoadEXRImageFromMemory(&exr_image, &exr_header, memory, len, &err) != TINYEXR_SUCCESS) {
            std::string err_copy = err;
            FreeEXRHeader(&exr_header);
            FreeEXRErrorMessage(err);
            KALI_THROW("Failed to read EXR image: {}", err_copy);
        }

        // exr_image.

        FreeEXRImage(&exr_image);
        FreeEXRHeader(&exr_header);

        // TODO: implement
        return {};
    }

    void read_image(const ImageSpec& spec, void* out_buffer, size_t out_len) override
    {
        KALI_UNUSED(spec);
        KALI_UNUSED(out_buffer);
        KALI_UNUSED(out_len);
        KALI_UNIMPLEMENTED();
    }

private:
};

// ----------------------------------------------------------------------------
// TinyExrImageWriter
// ----------------------------------------------------------------------------

class TinyExrImageWriter : public ImageWriter {
public:
    ~TinyExrImageWriter() { }

    void open(const std::filesystem::path& path, const ImageSpec& spec) override
    {
        if (spec.component_count > 4)
            KALI_THROW("Unsupported component count for EXR");
        if (spec.component_type != ImageComponentType::u32 && spec.component_type != ImageComponentType::f32
            && spec.component_type != ImageComponentType::f16)
            KALI_THROW("Unsupported component type for EXR");

        return ImageWriter::open(path, spec);
    }

    void write_image(const ImageSpec& spec, const void* buffer, size_t len) override
    {
        KALI_ASSERT(spec.component_count <= 4);

        int pixel_type;
        switch (spec.component_type) {
        case ImageComponentType::u32:
            pixel_type = TINYEXR_PIXELTYPE_UINT;
            break;
        case ImageComponentType::f32:
            pixel_type = TINYEXR_PIXELTYPE_FLOAT;
            break;
        case ImageComponentType::f16:
            pixel_type = TINYEXR_PIXELTYPE_HALF;
            break;
        default:
            KALI_THROW("Unsupported component type");
        }

        KALI_UNUSED(buffer);
        KALI_UNUSED(len);

        // Split image into channels.
        std::vector<std::unique_ptr<uint8_t[]>> channel_data(spec.component_count);
        std::vector<uint8_t*> channel_ptrs(spec.component_count);
        std::vector<int> pixel_types(spec.component_count);
        std::vector<int> requested_pixel_types(spec.component_count);
        for (uint32_t i = 0; i < spec.component_count; ++i) {
            channel_data[i].reset(new uint8_t[spec.width * spec.height * spec.get_component_byte_size()]);
            channel_ptrs[spec.component_count - i - 1] = channel_data[i].get();
            pixel_types[i] = pixel_type;
            requested_pixel_types[i] = pixel_type;
        }

        // Convert interleaved input data into channel data.
        switch (spec.component_type) {
        case ImageComponentType::u32:
        case ImageComponentType::f32:
            for (uint32_t y = 0; y < spec.height; ++y) {
                for (uint32_t x = 0; x < spec.width; ++x) {
                    const uint32_t* pixel
                        = reinterpret_cast<const uint32_t*>(buffer) + (y * spec.width + x) * spec.component_count;
                    for (uint32_t i = 0; i < spec.component_count; ++i) {
                        uint32_t* channel = reinterpret_cast<uint32_t*>(channel_data[i].get());
                        channel[y * spec.width + x] = pixel[i];
                    }
                }
            }
            break;
        case ImageComponentType::f16:
            for (uint32_t y = 0; y < spec.height; ++y) {
                for (uint32_t x = 0; x < spec.width; ++x) {
                    const uint16_t* pixel
                        = reinterpret_cast<const uint16_t*>(buffer) + (y * spec.width + x) * spec.component_count;
                    for (uint32_t i = 0; i < spec.component_count; ++i) {
                        uint16_t* channel = reinterpret_cast<uint16_t*>(channel_data[i].get());
                        channel[y * spec.width + x] = pixel[i];
                    }
                }
            }
            break;
        default:
            KALI_THROW("Unsupported component type");
        }

        // Setup channel infos.
        std::vector<EXRChannelInfo> channel_infos(spec.component_count);
        const char channel_names[] = "RGBA";
        for (uint32_t i = 0; i < spec.component_count; ++i) {
            auto& info = channel_infos[i];
            info.name[0] = channel_names[spec.component_count - i - 1];
            info.name[1] = '\0';
        }

        EXRHeader exr_header;
        InitEXRHeader(&exr_header);

        exr_header.num_channels = spec.component_count;
        exr_header.channels = channel_infos.data();
        exr_header.pixel_types = pixel_types.data();
        exr_header.requested_pixel_types = pixel_types.data();

        EXRImage exr_image;
        InitEXRImage(&exr_image);

        exr_image.width = spec.width;
        exr_image.height = spec.height;
        exr_image.num_channels = spec.component_count;
        exr_image.images = channel_ptrs.data();

        uint8_t* exr_buffer;
        const char* err;
        size_t exr_size = SaveEXRImageToMemory(&exr_image, &exr_header, &exr_buffer, &err);
        if (exr_size == 0) {
            FreeEXRErrorMessage(err);
            KALI_THROW("Failed to save EXR image");
        }

        m_ofs.write(reinterpret_cast<const char*>(exr_buffer), exr_size);
        free(exr_buffer);
    }
};

#endif

#if 0

// ----------------------------------------------------------------------------
// JPEGReader
// ----------------------------------------------------------------------------

class JPEGReader : public ImageReader {
public:
    JPEGReader()
    {
        m_info.err = jpeg_std_error(&m_err);
        m_err.error_exit = [](j_common_ptr info) { throw info->err; };
        jpeg_create_decompress(&m_info);
    }

    ~JPEGReader()
    {
        jpeg_destroy_decompress(&m_info);
        if (m_file)
            fclose(m_file);
    }

    bool open(const std::filesystem::path& path, ImageSpec& out_spec) override
    {
        m_file = fopen(path.string().c_str(), "rb");
        if (m_file == NULL)
            return false;

        jpeg_stdio_src(&m_info, m_file);

        try {
            jpeg_read_header(&m_info, TRUE);
            return true;
        } catch (jpeg_error_mgr*) {
            return false;
        }
    }

    bool open(const void* buffer, size_t len, ImageSpec& out_spec) override
    {
        jpeg_mem_src(&m_info, reinterpret_cast<const uint8_t*>(buffer), static_cast<unsigned long>(len));

        try {
            jpeg_read_header(&m_info, TRUE);

            out_spec.width = m_info.image_width;
            out_spec.height = m_info.image_height;
            out_spec.component_type = ImageComponentType::U8;
            out_spec.component_count = m_info.num_components;
            // TODO read exif

            return true;
        } catch (jpeg_error_mgr*) {
            return false;
        }
    }

    void read_image(const ImageSpec& spec, void* buffer, size_t len) override
    {
        uint8_t* dst = reinterpret_cast<uint8_t*>(buffer);
        try {
            jpeg_start_decompress(&m_info);

            int row_stride = m_info.output_width * m_info.output_components;
            JSAMPROW row[1];

            while (m_info.output_scanline < m_info.output_height) {
                row[0] = reinterpret_cast<JSAMPROW>(dst);
                jpeg_read_scanlines(&m_info, row, 1);
                dst += row_stride;
                // TODO bounds checking
            }

            jpeg_finish_decompress(&m_info);
        } catch (jpeg_error_mgr*) {
            return false;
        }

        return true;
    }

private:
    jpeg_decompress_struct m_info;
    jpeg_error_mgr m_err;
    FILE* m_file{NULL};
};

// ----------------------------------------------------------------------------
// JPEGWriter
// ----------------------------------------------------------------------------

class JPEGWriter : public ImageWriter {
public:
    JPEGWriter()
    {
        m_info.err = jpeg_std_error(&m_err);
        m_err.error_exit = [](j_common_ptr info) { throw info->err; };
        jpeg_create_compress(&m_info);
    }

    ~JPEGWriter()
    {
        jpeg_destroy_compress(&m_info);
        if (m_file)
            fclose(m_file);
    }

    bool open(const std::filesystem::path& path, const ImageSpec& spec) override
    {
        m_file = fopen(path.string().c_str(), "wb");
        if (m_file == NULL)
            return false;

        jpeg_stdio_dest(&m_info, m_file);

        m_info.image_width = spec.width;
        m_info.image_height = spec.height;
        m_info.input_components = spec.component_count;
        m_info.in_color_space = JCS_RGB;

        jpeg_set_defaults(&m_info);
        int quality = 80; // TODO make configurable
        jpeg_set_quality(&m_info, quality, TRUE);

        return true;
    }

    bool write_image(const void* buffer, size_t len) override
    {
        const uint8_t* src = reinterpret_cast<const uint8_t*>(buffer);
        try {
            jpeg_start_compress(&m_info, TRUE);

            int row_stride = m_info.image_width * m_info.input_components;
            JSAMPROW row[1];

            while (m_info.next_scanline < m_info.image_height) {
                row[0] = const_cast<JSAMPROW>(src);
                jpeg_write_scanlines(&m_info, row, 1);
                src += row_stride;
                // TODO bounds checking
            }

            jpeg_finish_compress(&m_info);
        } catch (jpeg_error_mgr*) {
            return false;
        }

        return true;
    }

private:
    jpeg_compress_struct m_info;
    jpeg_error_mgr m_err;
    FILE* m_file{NULL};
};

// ----------------------------------------------------------------------------
// PNGReader
// ----------------------------------------------------------------------------

class PNGReader : public ImageReader { };

#endif

// ----------------------------------------------------------------------------
// ImageInput
// ----------------------------------------------------------------------------

ref<ImageInput> ImageInput::open(const std::filesystem::path& path, Options options)
{
    ImageFileFormat file_format = extension_to_format(get_extension_from_path(path));

    ref<Stream> stream = make_ref<FileStream>(path, FileStream::Mode::Read);

    return open(stream, file_format, options);
}

ref<ImageInput> ImageInput::open(ref<Stream> stream, ImageFileFormat file_format, Options options)
{
    std::unique_ptr<ImageReader> reader;

    switch (file_format) {
    case ImageFileFormat::png:
    case ImageFileFormat::jpg:
    case ImageFileFormat::bmp:
    case ImageFileFormat::tga:
    case ImageFileFormat::hdr:
        reader = std::make_unique<StbImageReader>(stream, file_format);
        break;
    case ImageFileFormat::exr:
        // reader = std::make_unique<TinyExrImageReader>(stream, file_format);
        break;
    }

    if (!reader)
        KALI_THROW("Unsupported image file format");

    ImageSpec spec = reader->read_spec();

    ref<ImageInput> image_input = make_ref<ImageInput>();
    image_input->m_spec = std::move(spec);
    image_input->m_options = std::move(options);
    image_input->m_reader = std::move(reader);
    return image_input;
}

ImageInput::~ImageInput() = default;

void ImageInput::read_image(void* buffer, size_t len)
{
    KALI_ASSERT(m_reader);
    m_reader->read_image(m_spec, buffer, len);
}

// ----------------------------------------------------------------------------
// ImageOutput
// ----------------------------------------------------------------------------

ref<ImageOutput> ImageOutput::open(const std::filesystem::path& path, ImageSpec spec, Options options)
{
    ImageFileFormat file_format = extension_to_format(get_extension_from_path(path));

    ref<Stream> stream = make_ref<FileStream>(path, FileStream::Mode::Write);

    return open(stream, file_format, spec, options);
}

ref<ImageOutput> ImageOutput::open(ref<Stream> stream, ImageFileFormat file_format, ImageSpec spec, Options options)
{
    std::unique_ptr<ImageWriter> writer;

    KALI_ASSERT(spec.width > 0);
    KALI_ASSERT(spec.height > 0);
    KALI_ASSERT(spec.component_count > 0);
    KALI_ASSERT(spec.component_type != ImageComponentType::unknown);

    switch (file_format) {
    case ImageFileFormat::png:
    case ImageFileFormat::jpg:
    case ImageFileFormat::bmp:
    case ImageFileFormat::tga:
    case ImageFileFormat::hdr:
        writer = std::make_unique<StbImageWriter>(stream, file_format);
        break;
    case ImageFileFormat::exr:
        // writer = std::make_unique<TinyExrImageWriter>(stream, file_format);
        break;
    }

    if (!writer)
        KALI_THROW("Unknown image file format");

    writer->write_spec(spec);

    ref<ImageOutput> image_output = make_ref<ImageOutput>();
    image_output->m_spec = std::move(spec);
    image_output->m_options = std::move(options);
    image_output->m_writer = std::move(writer);
    return image_output;
}

ImageOutput::~ImageOutput() = default;

void ImageOutput::write_image(const void* buffer, size_t len)
{
    KALI_ASSERT(m_writer);
    m_writer->write_image(m_spec, buffer, len);
}

} // namespace kali
