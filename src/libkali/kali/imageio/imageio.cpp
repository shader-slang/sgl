#include "imageio.h"

#include "kali/macros.h"
#include "kali/error.h"
#include "kali/string_utils.h"
#include "kali/type_utils.h"
#include "kali/memory_mapped_file.h"

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

// ----------------------------------------------------------------------------
// ImageReader
// ----------------------------------------------------------------------------

class ImageReader {
public:
    virtual ~ImageReader() { }

    virtual bool open(const std::filesystem::path& path, ImageSpec& out_spec)
    {
        m_path = path;
        m_file = std::make_unique<MemoryMappedFile>(path);
        if (!m_file->is_open())
            return false;

        bool success = read_spec(m_file->data(), m_file->size(), m_spec);
        if (success)
            out_spec = m_spec;

        return success;
    }

    virtual void close() { m_file.reset(); }

    virtual bool read_spec(const void* buffer, size_t len, ImageSpec& out_spec) = 0;
    virtual bool read_image(void* out_buffer, size_t out_len) = 0;

protected:
    std::filesystem::path m_path;
    ImageSpec m_spec;
    std::unique_ptr<MemoryMappedFile> m_file;
};

class ImageWriter {
public:
    virtual ~ImageWriter() { }

    virtual bool open(const std::filesystem::path& path, const ImageSpec& spec)
    {
        KALI_ASSERT(!path.empty());
        KALI_ASSERT(spec.width > 0);
        KALI_ASSERT(spec.height > 0);
        KALI_ASSERT(spec.component_count > 0);
        KALI_ASSERT(spec.component_type != ImageComponentType::unknown);

        m_path = path;
        m_spec = spec;
        m_ofs.open(path, std::ios::binary);
        if (!m_ofs.is_open())
            return false;

        return true;
    }

    virtual void close() { m_ofs.close(); }

    virtual bool write_image(const void* buffer, size_t len) = 0;

protected:
    std::filesystem::path m_path;
    ImageSpec m_spec;
    std::ofstream m_ofs;
};

// ----------------------------------------------------------------------------
// StbImageReader
// ----------------------------------------------------------------------------

class StbImageReader : public ImageReader {
public:
    ~StbImageReader() { }

    bool read_spec(const void* buffer_, size_t len_, ImageSpec& out_spec) override
    {
        const stbi_uc* buffer = reinterpret_cast<const stbi_uc*>(buffer_);
        int len = narrow_cast<int>(len_);

        int width, height, component_count;
        if (!stbi_info_from_memory(buffer, len, &width, &height, &component_count))
            return false;

        bool is_hdr = stbi_is_hdr_from_memory(buffer, len);
        bool is_16bit = stbi_is_16_bit_from_memory(buffer, len);

        m_spec = ImageSpec{
            .width = narrow_cast<uint32_t>(width),
            .height = narrow_cast<uint32_t>(height),
            .component_count = narrow_cast<uint32_t>(component_count),
            .component_type
            = is_hdr ? ImageComponentType::f32 : (is_16bit ? ImageComponentType::u16 : ImageComponentType::u8),
        };
        out_spec = m_spec;

        return true;
    }

    bool read_image(void* out_buffer, size_t out_len) override
    {
        KALI_ASSERT(m_spec.width > 0);
        KALI_ASSERT(m_spec.height > 0);
        KALI_ASSERT(m_spec.component_count > 0);
        KALI_ASSERT(m_spec.component_type != ImageComponentType::unknown);

        const stbi_uc* buffer = reinterpret_cast<const stbi_uc*>(m_file->data());
        int len = narrow_cast<int>(m_file->size());

        int width, height, component_count;
        uint8_t* pixels{nullptr};
        size_t read_len{0};
        switch (m_spec.component_type) {
        case ImageComponentType::u8:
            pixels
                = reinterpret_cast<uint8_t*>(stbi_load_from_memory(buffer, len, &width, &height, &component_count, 0));
            read_len = width * height * component_count;
            break;
        case ImageComponentType::u16:
            pixels
                = reinterpret_cast<uint8_t*>(stbi_load_16_from_memory(buffer, len, &width, &height, &component_count, 0)
                );
            read_len = width * height * component_count * 2;
            break;
        case ImageComponentType::f32:
            pixels
                = reinterpret_cast<uint8_t*>(stbi_loadf_from_memory(buffer, len, &width, &height, &component_count, 0));
            read_len = width * height * component_count * 4;
            break;
        default:
            KALI_ASSERT(false);
            break;
        }

        if (!pixels)
            return false;

        size_t expected_len = m_spec.width * m_spec.height * m_spec.component_count * m_spec.get_component_byte_size();
        KALI_ASSERT_EQ(out_len, expected_len);
        KALI_ASSERT_EQ(read_len, expected_len);

        std::memcpy(out_buffer, pixels, out_len);

        stbi_image_free(pixels);

        return true;
    }
};

// ----------------------------------------------------------------------------
// StbImageWriter
// ----------------------------------------------------------------------------

class StbImageWriter : public ImageWriter {
public:
    ~StbImageWriter() { }

    bool open(const std::filesystem::path& path, const ImageSpec& spec) override
    {
        std::string ext = to_lower(path.extension().string());
        if (ext == ".png") {
            m_file_format = FileFormat::png;
            if (spec.component_type != ImageComponentType::u8)
                return false;
        } else if (ext == ".jpg" || ext == ".jpeg") {
            m_file_format = FileFormat::jpg;
            if (spec.component_type != ImageComponentType::u8)
                return false;
        } else if (ext == ".bmp") {
            m_file_format = FileFormat::bmp;
            if (spec.component_type != ImageComponentType::u8)
                return false;
        } else if (ext == ".tga") {
            m_file_format = FileFormat::tga;
            if (spec.component_type != ImageComponentType::u8)
                return false;
        } else if (ext == ".hdr") {
            m_file_format = FileFormat::hdr;
            if (spec.component_type != ImageComponentType::f32)
                return false;
        } else
            return false;

        return ImageWriter::open(path, spec);
    }

    bool write_image(const void* buffer, size_t len) override
    {
        KALI_UNUSED(len);

        size_t expected_len = m_spec.width * m_spec.height * m_spec.component_count * m_spec.get_component_byte_size();
        KALI_ASSERT_EQ(len, expected_len);

        int width = narrow_cast<int>(m_spec.width);
        int height = narrow_cast<int>(m_spec.height);
        int component_count = narrow_cast<int>(m_spec.component_count);

        auto write_func = [](void* context, void* data, int size)
        {
            std::ofstream* ofs = reinterpret_cast<std::ofstream*>(context);
            ofs->write(reinterpret_cast<char*>(data), size);
        };

        bool success{false};
        switch (m_file_format) {
        case FileFormat::png:
            success = stbi_write_png_to_func(write_func, &m_ofs, width, height, component_count, buffer, 0);
            break;
        case FileFormat::jpg:
            success = stbi_write_jpg_to_func(write_func, &m_ofs, width, height, component_count, buffer, 100);
            break;
        case FileFormat::bmp:
            success = stbi_write_bmp_to_func(write_func, &m_ofs, width, height, component_count, buffer);
            break;
        case FileFormat::tga:
            success = stbi_write_tga_to_func(write_func, &m_ofs, width, height, component_count, buffer);
            break;
        case FileFormat::hdr:
            success = stbi_write_hdr_to_func(
                write_func,
                &m_ofs,
                width,
                height,
                component_count,
                reinterpret_cast<const float*>(buffer)
            );
            break;
        default:
            break;
        }
        return success;
    }

protected:
    enum class FileFormat { unknown, png, jpg, bmp, tga, hdr };
    FileFormat m_file_format{FileFormat::unknown};
};

// ----------------------------------------------------------------------------
// TinyExrImageReader
// ----------------------------------------------------------------------------

class TinyExrImageReader : public ImageReader {
public:
    ~TinyExrImageReader() { }

    bool read_spec(const void* buffer, size_t len, ImageSpec& out_spec) override
    {
        const uint8_t* memory = reinterpret_cast<const uint8_t*>(buffer);

        EXRVersion exr_version;
        if (ParseEXRVersionFromMemory(&exr_version, memory, len) != TINYEXR_SUCCESS)
            return false;

        if (exr_version.multipart)
            return false;

        EXRHeader exr_header;
        const char* err;
        if (ParseEXRHeaderFromMemory(&exr_header, &exr_version, memory, len, &err) != TINYEXR_SUCCESS) {
            FreeEXRErrorMessage(err);
            return false;
        }

        EXRImage exr_image;
        InitEXRImage(&exr_image);

        if (LoadEXRImageFromMemory(&exr_image, &exr_header, memory, len, &err) != TINYEXR_SUCCESS) {
            FreeEXRHeader(&exr_header);
            FreeEXRErrorMessage(err);
            return false;
        }

        // exr_image.

        FreeEXRImage(&exr_image);
        FreeEXRHeader(&exr_header);

        KALI_UNUSED(out_spec);
        return false;
    }

    bool read_image(void* out_buffer, size_t out_len) override
    {
        KALI_UNUSED(out_buffer);
        KALI_UNUSED(out_len);
        return false;
    }

private:
};

// ----------------------------------------------------------------------------
// TinyExrImageWriter
// ----------------------------------------------------------------------------

class TinyExrImageWriter : public ImageWriter {
public:
    ~TinyExrImageWriter() { }

    bool open(const std::filesystem::path& path, const ImageSpec& spec) override
    {
        if (spec.component_count > 4)
            return false;
        if (spec.component_type != ImageComponentType::u32 && spec.component_type != ImageComponentType::f32
            && spec.component_type != ImageComponentType::f16)
            return false;

        return ImageWriter::open(path, spec);
    }

    bool write_image(const void* buffer, size_t len) override
    {
        KALI_ASSERT(m_spec.component_count <= 4);

        int pixel_type;
        switch (m_spec.component_type) {
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
            KALI_ASSERT(false);
            break;
        }

        KALI_UNUSED(buffer);
        KALI_UNUSED(len);

        // Split image into channels.
        std::vector<std::unique_ptr<uint8_t[]>> channel_data(m_spec.component_count);
        std::vector<uint8_t*> channel_ptrs(m_spec.component_count);
        std::vector<int> pixel_types(m_spec.component_count);
        std::vector<int> requested_pixel_types(m_spec.component_count);
        for (uint32_t i = 0; i < m_spec.component_count; ++i) {
            channel_data[i].reset(new uint8_t[m_spec.width * m_spec.height * m_spec.get_component_byte_size()]);
            channel_ptrs[m_spec.component_count - i - 1] = channel_data[i].get();
            pixel_types[i] = pixel_type;
            requested_pixel_types[i] = pixel_type;
        }

        // Convert interleaved input data into channel data.
        switch (m_spec.component_type) {
        case ImageComponentType::u32:
        case ImageComponentType::f32:
            for (uint32_t y = 0; y < m_spec.height; ++y) {
                for (uint32_t x = 0; x < m_spec.width; ++x) {
                    const uint32_t* pixel
                        = reinterpret_cast<const uint32_t*>(buffer) + (y * m_spec.width + x) * m_spec.component_count;
                    for (uint32_t i = 0; i < m_spec.component_count; ++i) {
                        uint32_t* channel = reinterpret_cast<uint32_t*>(channel_data[i].get());
                        channel[y * m_spec.width + x] = pixel[i];
                    }
                }
            }
            break;
        case ImageComponentType::f16:
            for (uint32_t y = 0; y < m_spec.height; ++y) {
                for (uint32_t x = 0; x < m_spec.width; ++x) {
                    const uint16_t* pixel
                        = reinterpret_cast<const uint16_t*>(buffer) + (y * m_spec.width + x) * m_spec.component_count;
                    for (uint32_t i = 0; i < m_spec.component_count; ++i) {
                        uint16_t* channel = reinterpret_cast<uint16_t*>(channel_data[i].get());
                        channel[y * m_spec.width + x] = pixel[i];
                    }
                }
            }
            break;
        default:
            KALI_ASSERT(false);
            break;
        }

        // Setup channel infos.
        std::vector<EXRChannelInfo> channel_infos(m_spec.component_count);
        const char channel_names[] = "RGBA";
        for (uint32_t i = 0; i < m_spec.component_count; ++i) {
            auto& info = channel_infos[i];
            info.name[0] = channel_names[m_spec.component_count - i - 1];
            info.name[1] = '\0';
        }

        EXRHeader exr_header;
        InitEXRHeader(&exr_header);

        exr_header.num_channels = m_spec.component_count;
        exr_header.channels = channel_infos.data();
        exr_header.pixel_types = pixel_types.data();
        exr_header.requested_pixel_types = pixel_types.data();

        EXRImage exr_image;
        InitEXRImage(&exr_image);

        exr_image.width = m_spec.width;
        exr_image.height = m_spec.height;
        exr_image.num_channels = m_spec.component_count;
        exr_image.images = channel_ptrs.data();

        uint8_t* exr_buffer;
        const char* err;
        size_t exr_size = SaveEXRImageToMemory(&exr_image, &exr_header, &exr_buffer, &err);
        if (exr_size == 0) {
            FreeEXRErrorMessage(err);
            return false;
        }

        m_ofs.write(reinterpret_cast<const char*>(exr_buffer), exr_size);
        free(exr_buffer);

        return true;
    }
};


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

    bool read_image(void* buffer, size_t len) override
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
    if (!std::filesystem::exists(path))
        return nullptr;

    std::string ext = to_lower(path.extension().string());

    std::unique_ptr<ImageReader> reader;

    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga" || ext == ".hdr") {
        reader = std::make_unique<StbImageReader>();
    } else if (ext == ".exr") {
        reader = std::make_unique<TinyExrImageReader>();
    }

    if (!reader)
        return nullptr;

    std::unique_ptr<MemoryMappedFile> file = std::make_unique<MemoryMappedFile>(path);
    if (!file->is_open())
        return nullptr;

    ImageSpec spec;
    if (!reader->open(path, spec))
        return nullptr;

    ref<ImageInput> image_input = make_ref<ImageInput>();
    image_input->m_spec = std::move(spec);
    image_input->m_options = std::move(options);
    image_input->m_reader = std::move(reader);
    image_input->m_file = std::move(file);

    return image_input;
}

ImageInput::~ImageInput() = default;

bool ImageInput::read_image(void* buffer, size_t len)
{
    KALI_ASSERT(m_reader);
    return m_reader->read_image(buffer, len);
}

// ----------------------------------------------------------------------------
// ImageOutput
// ----------------------------------------------------------------------------

ref<ImageOutput> ImageOutput::open(const std::filesystem::path& path, ImageSpec spec, Options options)
{
    std::string ext = to_lower(path.extension().string());

    std::unique_ptr<ImageWriter> writer;

    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga" || ext == ".hdr") {
        writer = std::make_unique<StbImageWriter>();
    } else if (ext == ".exr") {
        writer = std::make_unique<TinyExrImageWriter>();
    }

    if (!writer)
        return nullptr;

    if (!writer->open(path, spec))
        return nullptr;

    ref<ImageOutput> image_output = make_ref<ImageOutput>();
    image_output->m_spec = std::move(spec);
    image_output->m_options = std::move(options);
    image_output->m_writer = std::move(writer);

    return image_output;
}

ImageOutput::~ImageOutput() = default;

bool ImageOutput::write_image(const void* buffer, size_t len)
{
    KALI_ASSERT(m_writer);
    return m_writer->write_image(buffer, len);
}

} // namespace kali
