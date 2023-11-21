// TODO: MITSUBA license

#include "bitmap.h"
#include "kali/core/config.h"
#include "kali/core/macros.h"
#include "kali/core/error.h"
#include "kali/core/logger.h"
#include "kali/core/file_stream.h"
#include "kali/core/string.h"
#include "kali/core/thread.h"

#if KALI_HAS_PNG
#include <png.h>
#endif

#if KALI_HAS_JPEG
#include <jpeglib.h>
#endif

#if KALI_HAS_OPENEXR
#include <ImfInputFile.h>
#include <ImfStandardAttributes.h>
#include <ImfRgbaYca.h>
#include <ImfOutputFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <ImfStringAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfFloatAttribute.h>
#include <ImfDoubleAttribute.h>
#include <ImfCompressionAttribute.h>
#include <ImfVecAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfVersion.h>
#include <ImfIO.h>
#include <ImathBox.h>
#include <IlmThreadPool.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <algorithm>

KALI_DISABLE_MSVC_WARNING(4611)

namespace kali {

inline std::vector<std::string> get_channel_names(Bitmap::PixelFormat pixel_format)
{
    switch (pixel_format) {
    case Bitmap::PixelFormat::y:
        return {"y"};
    case Bitmap::PixelFormat::ya:
        return {"y", "a"};
    case Bitmap::PixelFormat::rgb:
        return {"r", "g", "b"};
    case Bitmap::PixelFormat::rgba:
        return {"r", "g", "b", "a"};
    case Bitmap::PixelFormat::multi_channel:
        return {};
    }
    KALI_THROW("Invalid pixel format");
}

Bitmap::Bitmap(
    PixelFormat pixel_format,
    ComponentType component_type,
    uint32_t width,
    uint32_t height,
    uint32_t channel_count,
    const std::vector<std::string>& channel_names,
    void* data
)
    : m_pixel_format(pixel_format)
    , m_component_type(component_type)
    , m_width(width)
    , m_height(height)
    , m_channel_count(channel_count)
    , m_channel_names(channel_names)
    , m_data(reinterpret_cast<uint8_t*>(data))
    , m_owns_data(false)
{
    KALI_CHECK(
        pixel_format != PixelFormat::multi_channel || channel_count > 0,
        "Expected non-zero channel count for multi-channel pixel format."
    );

    if (pixel_format != PixelFormat::multi_channel) {
        m_channel_names = get_channel_names(pixel_format);
        m_channel_count = static_cast<uint32_t>(m_channel_names.size());
    }

    // Use sRGB gamma by default for 8-bit and 16-bit images.
    m_srgb_gamma = (m_component_type == ComponentType::uint8) || (m_component_type == ComponentType::uint16);

    if (!m_data) {
        m_data = std::make_unique<uint8_t[]>(buffer_size());
        m_owns_data = true;
    }
}

Bitmap::Bitmap(const Bitmap& other)
    : m_pixel_format(other.m_pixel_format)
    , m_component_type(other.m_component_type)
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_channel_count(other.m_channel_count)
    , m_channel_names(other.m_channel_names)
    , m_srgb_gamma(other.m_srgb_gamma)
    , m_data(new uint8_t[other.buffer_size()])
{
    std::memcpy(m_data.get(), other.m_data.get(), other.buffer_size());
}

Bitmap::Bitmap(Bitmap&& other)
    : m_pixel_format(std::exchange(other.m_pixel_format, PixelFormat(0)))
    , m_component_type(std::exchange(other.m_component_type, ComponentType(0)))
    , m_width(std::exchange(other.m_width, 0))
    , m_height(std::exchange(other.m_height, 0))
    , m_channel_count(std::exchange(other.m_channel_count, 0))
    , m_channel_names(std::move(other.m_channel_names))
    , m_srgb_gamma(std::exchange(other.m_srgb_gamma, false))
    , m_data(std::move(other.m_data))
{
}

Bitmap::Bitmap(Stream* stream, FileFormat format)
{
    KALI_UNUSED(stream);
    KALI_UNUSED(format);
}

Bitmap::Bitmap(const std::filesystem::path& path, FileFormat format)
{
    FileStream stream(path, FileStream::Mode::read);
    read(&stream, format);
}

Bitmap::~Bitmap()
{
    if (!m_owns_data)
        m_data.release();
}

std::vector<ref<Bitmap>> Bitmap::read_multiple(const std::vector<std::filesystem::path>& paths, FileFormat format)
{
    std::vector<std::future<ref<Bitmap>>> futures;
    futures.reserve(paths.size());
    for (const auto& path : paths)
        futures.push_back(thread::do_async(
            [](const std::filesystem::path& path, FileFormat format) { return make_ref<Bitmap>(path, format); },
            path,
            format
        ));
    std::vector<ref<Bitmap>> bitmaps;
    bitmaps.reserve(paths.size());
    for (auto& future : futures) {
        future.wait();
        bitmaps.push_back(future.get());
    }
    return bitmaps;
}

#if 0
ref<Bitmap> Bitmap::read(Stream* stream, FileFormat format)
{
    return make_ref<Bitmap>(stream, format);
}

ref<Bitmap> Bitmap::read(const std::filesystem::path& path, FileFormat format)
{
    return make_ref<Bitmap>(path, format);
}

std::future<ref<Bitmap>> Bitmap::read_async(ref<Stream> stream, FileFormat format)
{
    KALI_UNUSED(stream);
    KALI_UNUSED(format);
    return {};
}

std::future<ref<Bitmap>> Bitmap::read_async(const std::filesystem::path& path, FileFormat format)
{
    auto stream = make_ref<FileStream>(path, FileStream::Mode::read);
    return read_async(stream, format);
}
#endif

void Bitmap::write(Stream* stream, FileFormat format, int quality) const
{
    KALI_UNUSED(quality);

    auto fs = dynamic_cast<FileStream*>(stream);

    if (format == FileFormat::auto_) {
        KALI_CHECK(fs, "Unable to determine image file format without a filename.");
        std::string extension = string::to_lower(fs->path().extension().string());
        if (extension == ".png")
            format = FileFormat::png;
        else if (extension == ".jpg" || extension == ".jpeg")
            format = FileFormat::jpg;
        else if (extension == ".bmp")
            format = FileFormat::bmp;
        else if (extension == ".tga")
            format = FileFormat::tga;
        else if (extension == ".hdr" || extension == ".rgbe")
            format = FileFormat::hdr;
        else if (extension == ".exr")
            format = FileFormat::exr;
        else
            KALI_THROW("Unsupported image file extension \"%s\"", extension);
    }

    log_debug(
        "Writing {} file \"{}\" ({}x{}, {}, {}) ...",
        format,
        fs ? fs->path().string() : "<stream>",
        m_width,
        m_height,
        m_pixel_format,
        m_component_type
    );

    switch (format) {
    case FileFormat::png:
#if KALI_HAS_PNG
        if (quality == -1)
            quality = 5;
        write_png(stream, quality);
#else
        KALI_THROW("PNG support is not available!");
#endif
        break;
    case FileFormat::jpg:
#if KALI_HAS_JPEG
        if (quality == -1)
            quality = 100;
        write_jpg(stream, quality);
#else
        KALI_THROW("JPEG support is not available!");
#endif
        break;
    case FileFormat::bmp:
        write_bmp(stream);
        break;
    case FileFormat::tga:
        write_tga(stream);
        break;
    case FileFormat::hdr:
        write_hdr(stream);
        break;
    case FileFormat::exr:
#if KALI_HAS_OPENEXR
        write_exr(stream, quality);
#else
        KALI_THROW("OpenEXR support is not available!");
#endif
        break;
    default:
        KALI_THROW("Invalid file format!");
    }
}

void Bitmap::write(const std::filesystem::path& path, FileFormat format, int quality) const
{
    auto stream = make_ref<FileStream>(path, FileStream::Mode::write);
    write(stream, format, quality);
}

void Bitmap::write_async(const std::filesystem::path& path, FileFormat format, int quality) const
{
    // Increment reference count to ensure that the bitmap is not destroyed before written.
    this->inc_ref();
    thread::do_async(
        [=, this]()
        {
            this->write(path, format, quality);
            this->dec_ref();
        }
    );
}

size_t Bitmap::bytes_per_pixel() const
{
    return Struct::type_size(m_component_type) * m_channel_count;
}

void Bitmap::clear()
{
    std::memset(m_data.get(), 0, buffer_size());
}

void Bitmap::vflip()
{
    size_t row_size = buffer_size() / m_height;
    size_t half_height = m_height / 2;
    uint8_t* temp = reinterpret_cast<uint8_t*>(alloca(row_size));
    uint8_t* data = uint8_data();
    for (size_t i = 0, j = m_height - 1; i < half_height; ++i) {
        std::memcpy(temp, data + i * row_size, row_size);
        std::memcpy(data + i * row_size, data + j * row_size, row_size);
        std::memcpy(data + j * row_size, temp, row_size);
        j--;
    }
}

ref<Struct> Bitmap::pixel_struct() const
{
    ref<Struct> result = make_ref<Struct>();
    Struct::Flags flags = Struct::Flags::none;
    if (m_component_type == ComponentType::uint8 || m_component_type == ComponentType::uint16)
        flags |= Struct::Flags::normalized;
    if (m_srgb_gamma)
        flags |= Struct::Flags::srgb_gamma;
    for (size_t i = 0; i < m_channel_count; ++i)
        result->append(m_channel_names[i], m_component_type, flags);
    return result;
}

ref<Bitmap> Bitmap::convert(PixelFormat pixel_format, ComponentType component_type, bool srgb_gamma) const
{
    ref<Bitmap> result = make_ref<Bitmap>(pixel_format, component_type, m_width, m_height);
    result->set_srgb_gamma(srgb_gamma);
    convert(result);
    return result;
}

void Bitmap::convert(Bitmap* target) const
{
    if (width() != target->width() || height() != target->height())
        KALI_THROW(
            "Bitmap dimensions do not match (source image is {}x{} vs target image {}x{})!",
            width(),
            height(),
            target->width(),
            target->height()
        );

    bool src_is_rgb = m_pixel_format == PixelFormat::rgb || m_pixel_format == PixelFormat::rgba;
    bool src_is_y = m_pixel_format == PixelFormat::y || m_pixel_format == PixelFormat::ya;

    ref<Struct> src_struct = pixel_struct();
    ref<Struct> dst_struct = target->pixel_struct();

    for (Struct::Field& field : dst_struct->fields()) {
        if (src_struct->has_field(field.name)) {
            continue;
        }
        if (field.name == "r") {
            if (src_is_y) {
                field.name = "y";
                continue;
            }
        }
        if (field.name == "g") {
            if (src_is_y) {
                field.name = "y";
                continue;
            }
        }
        if (field.name == "b") {
            if (src_is_y) {
                field.name = "y";
                continue;
            }
        }
        if (field.name == "a") {
            field.default_value = 1.0;
            field.flags |= Struct::Flags::default_;
            continue;
        }
        KALI_THROW(
            "Unable to convert bitmap: cannot determine how to derive field \"{}\" in target image!",
            field.name
        );
    }

    ref<StructConverter> converter = make_ref<StructConverter>(src_struct, dst_struct);
    converter->convert(data(), target->data(), pixel_count());
}

bool Bitmap::operator==(const Bitmap& other) const
{
    return m_pixel_format == other.m_pixel_format && m_component_type == other.m_component_type
        && m_width == other.m_width && m_height == other.m_height && m_channel_count == other.m_channel_count
        && m_channel_names == other.m_channel_names && m_srgb_gamma == other.m_srgb_gamma
        && std::memcmp(m_data.get(), other.m_data.get(), buffer_size()) == 0;
}

std::string Bitmap::to_string() const
{
    return fmt::format(
        "Bitmap(\n"
        "  pixel_format = {},\n"
        "  component_type = {},\n"
        "  channel_count = {},\n"
        "  width = {},\n"
        "  height = {},\n"
        "  srgb_gamma = {},\n"
        "  data = {},\n"
        ")",
        m_pixel_format,
        m_component_type,
        m_channel_count,
        m_width,
        m_height,
        m_srgb_gamma,
        string::format_byte_size(buffer_size())
    );
}

void Bitmap::static_init()
{
    // IlmThread::ThreadPool::globalThreadPool().setThreadProvider(new EXRThreadPool());
}

void Bitmap::static_shutdown() { }

void Bitmap::read(Stream* stream, FileFormat format)
{
    if (format == FileFormat::auto_)
        format = detect_file_format(stream);

    switch (format) {
    case FileFormat::png:
#if KALI_HAS_PNG
        read_png(stream);
#else
        KALI_THROW("PNG support is not available!");
#endif
        break;
    case FileFormat::jpg:
#if KALI_HAS_JPEG
        read_jpg(stream);
#else
        KALI_THROW("JPEG support is not available!");
#endif
        break;
    case FileFormat::bmp:
        read_bmp(stream);
        break;
    case FileFormat::tga:
        read_tga(stream);
        break;
    case FileFormat::hdr:
        read_hdr(stream);
        break;
    case FileFormat::exr:
#if KALI_HAS_OPENEXR
        read_exr(stream);
#else
        KALI_THROW("OpenEXR support is not available!");
#endif
        break;
    default:
        KALI_THROW("Unknown file format!");
    }
}

Bitmap::FileFormat Bitmap::detect_file_format(Stream* stream)
{
    FileFormat format = FileFormat::unknown;

    size_t pos = stream->tell();
    uint8_t header[8];
    stream->read(header, 8);

    if (header[0] == 'B' && header[1] == 'M') {
        format = FileFormat::bmp;
    } else if (header[0] == '#' && header[1] == '?') {
        format = FileFormat::hdr;
#if KALI_HAS_JPEG
    } else if (header[0] == 0xFF && header[1] == 0xD8) {
        format = FileFormat::jpg;
#endif
#if KALI_HAS_PNG
    } else if (png_sig_cmp(header, 0, 8) == 0) {
        format = FileFormat::png;
#endif
#if KALI_HAS_OPENEXR
    } else if (Imf::isImfMagic(reinterpret_cast<const char*>(header))) {
        format = FileFormat::exr;
#endif
    } else {
        // Check for TGAv1 file
        char spec[10];
        stream->read(spec, 10);
        if ((header[1] == 0 || header[1] == 1)
            && (header[2] == 1 || header[2] == 2 || header[2] == 3 || header[2] == 9 || header[2] == 10
                || header[2] == 11)
            && (spec[8] == 8 || spec[8] == 16 || spec[8] == 24 || spec[8] == 32))
            format = FileFormat::tga;

        // Check for a TGAv2 file
        char footer[18];
        stream->seek(stream->size() - 18);
        stream->read(footer, 18);
        if (footer[17] == 0 && strncmp(footer, "TRUEVISION-XFILE.", 17) == 0)
            format = FileFormat::tga;
    }
    stream->seek(pos);
    return format;
}

void Bitmap::check_required_format(
    std::string_view file_format,
    std::vector<Bitmap::PixelFormat> allowed_pixel_formats,
    std::vector<Bitmap::ComponentType> allowed_component_types
) const
{
    if (std::find(allowed_pixel_formats.begin(), allowed_pixel_formats.end(), m_pixel_format)
        == allowed_pixel_formats.end())
        KALI_THROW(
            "Unsupported pixel format {} for writing {} image, expected one of: {}.",
            m_pixel_format,
            file_format,
            fmt::join(allowed_pixel_formats, ", ")
        );
    if (std::find(allowed_component_types.begin(), allowed_component_types.end(), m_component_type)
        == allowed_component_types.end())
        KALI_THROW(
            "Unsupported component type {} for writing {} image, expected one of: {}.",
            m_component_type,
            file_format,
            fmt::join(allowed_component_types, ", ")
        );
}

// ----------------------------------------------------------------------------
// STB I/O
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// PNG I/O
// ----------------------------------------------------------------------------

#if KALI_HAS_PNG

static void png_flush_data(png_structp png_ptr)
{
    static_cast<Stream*>(png_get_io_ptr(png_ptr))->flush();
}

static void png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    static_cast<Stream*>(png_get_io_ptr(png_ptr))->read(data, length);
}

static void png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    static_cast<Stream*>(png_get_io_ptr(png_ptr))->write(data, length);
}

static void png_error_func(png_structp, png_const_charp msg)
{
    KALI_THROW("libpng error: {}\n", msg);
}

static void png_warn_func(png_structp, png_const_charp msg)
{
    if (std::strstr(msg, "iCCP: known incorrect sRGB profile") != nullptr)
        return;
    log_warn("libpng warning: {}\n", msg);
}

void Bitmap::read_png(Stream* stream)
{
    // Create buffers.
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, &png_error_func, &png_warn_func);
    if (png_ptr == nullptr)
        KALI_THROW("Failed to create PNG data structure!");

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr) {
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        KALI_THROW("Failed to create PNG information structure!");
    }

    // Setup error handling.
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        KALI_THROW("Error reading the PNG file!");
    }

    // Setup read callback.
    png_set_read_fn(png_ptr, stream, (png_rw_ptr)png_read_data);

    int bit_depth, color_type, interlace_type, compression_type, filter_type;
    png_read_info(png_ptr, info_ptr);
    png_uint_32 width = 0, height = 0;
    png_get_IHDR(
        png_ptr,
        info_ptr,
        &width,
        &height,
        &bit_depth,
        &color_type,
        &interlace_type,
        &compression_type,
        &filter_type
    );

    // Expand 1-, 2- and 4-bit grayscale.
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);

    // Always expand paletted files.
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    // Expand transparency to a proper alpha channel.
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    // Swap the byte order on little endian machines.
    if constexpr (std::endian::native == std::endian::little) {
        if (bit_depth == 16)
            png_set_swap(png_ptr);
    }

    // Update the information based on the transformations.
    png_read_update_info(png_ptr, info_ptr);
    png_get_IHDR(
        png_ptr,
        info_ptr,
        &width,
        &height,
        &bit_depth,
        &color_type,
        &interlace_type,
        &compression_type,
        &filter_type
    );
    m_width = width;
    m_height = height;

    switch (color_type) {
    case PNG_COLOR_TYPE_GRAY:
        m_pixel_format = PixelFormat::y;
        m_channel_count = 1;
        break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
        m_pixel_format = PixelFormat::ya;
        m_channel_count = 2;
        break;
    case PNG_COLOR_TYPE_RGB:
        m_pixel_format = PixelFormat::rgb;
        m_channel_count = 3;
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        m_pixel_format = PixelFormat::rgba;
        m_channel_count = 4;
        break;
    default:
        KALI_THROW("Unknown color type {}!", color_type);
    }

    m_channel_names = get_channel_names(m_pixel_format);

    switch (bit_depth) {
    case 8:
        m_component_type = ComponentType::uint8;
        break;
    case 16:
        m_component_type = ComponentType::uint16;
        break;
    default:
        KALI_THROW("Unsupported bit depth {}!", bit_depth);
    }

    // TODO should we detect non-srgb pngs?
    m_srgb_gamma = true;

#if 0
    m_srgb_gamma = true;
    m_premultiplied_alpha = false;

    rebuild_struct();

    // Load any string-valued metadata
    int text_idx = 0;
    png_textp text_ptr;
    png_get_text(png_ptr, info_ptr, &text_ptr, &text_idx);

    for (int i = 0; i < text_idx; ++i, text_ptr++)
        m_metadata.set_string(text_ptr->key, text_ptr->text);
#endif

    auto fs = dynamic_cast<FileStream*>(stream);
    log_debug(
        "Reading PNG file \"{}\" ({}x{}, {}, {}) ...",
        fs ? fs->path().string() : "<stream>",
        m_width,
        m_height,
        m_pixel_format,
        m_component_type
    );

    size_t size = buffer_size();
    m_data = std::unique_ptr<uint8_t[]>(new uint8_t[size]);
    m_owns_data = true;

    size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    KALI_ASSERT(row_bytes == size / m_height);

    png_bytepp rows = static_cast<png_bytepp>(alloca(sizeof(png_bytep) * m_height));
    for (size_t i = 0; i < m_height; i++)
        rows[i] = uint8_data() + i * row_bytes;

    png_read_image(png_ptr, rows);
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
}

void Bitmap::write_png(Stream* stream, int compression) const
{
    check_required_format(
        "PNG",
        {PixelFormat::y, PixelFormat::ya, PixelFormat::rgb, PixelFormat::rgba},
        {ComponentType::uint8, ComponentType::uint16}
    );

    int color_type;
    switch (m_pixel_format) {
    case PixelFormat::y:
        color_type = PNG_COLOR_TYPE_GRAY;
        break;
    case PixelFormat::ya:
        color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
        break;
    case PixelFormat::rgb:
        color_type = PNG_COLOR_TYPE_RGB;
        break;
    case PixelFormat::rgba:
        color_type = PNG_COLOR_TYPE_RGBA;
        break;
    default:
        KALI_THROW("Unsupported pixel format!");
    }

    int bit_depth;
    switch (m_component_type) {
    case ComponentType::uint8:
        bit_depth = 8;
        break;
    case ComponentType::uint16:
        bit_depth = 16;
        break;
    default:
        KALI_THROW("Unsupported component type!");
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, &png_error_func, &png_warn_func);
    if (png_ptr == nullptr)
        KALI_THROW("Error while creating PNG data structure");

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr) {
        png_destroy_write_struct(&png_ptr, nullptr);
        KALI_THROW("Error while creating PNG information structure");
    }

    // Setup error handling.
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        KALI_THROW("Error writing the PNG file");
    }

    png_set_write_fn(png_ptr, stream, &png_write_data, &png_flush_data);
    png_set_compression_level(png_ptr, compression);

#if 0
    png_text* text = nullptr;

    Properties metadata(m_metadata);
    if (!metadata.has_property("generated_by"))
        metadata.set_string("generated_by", "Mitsuba version " MI_VERSION);

    std::vector<std::string> keys = metadata.property_names();
    std::vector<std::string> values(keys.size());

    text = new png_text[keys.size()];
    std::memset(text, 0, sizeof(png_text) * keys.size());

    for (size_t i = 0; i < keys.size(); ++i) {
        values[i] = metadata.as_string(keys[i]);
        text[i].key = const_cast<char*>(keys[i].c_str());
        text[i].text = const_cast<char*>(values[i].c_str());
        text[i].compression = PNG_TEXT_COMPRESSION_NONE;
    }

    png_set_text(png_ptr, info_ptr, text, (int)keys.size());
#endif

    if (m_srgb_gamma)
        png_set_sRGB_gAMA_and_cHRM(png_ptr, info_ptr, PNG_sRGB_INTENT_ABSOLUTE);

    png_set_IHDR(
        png_ptr,
        info_ptr,
        m_width,
        m_height,
        bit_depth,
        color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE
    );

    png_write_info(png_ptr, info_ptr);

    // Swap the byte order on little endian machines.
    if constexpr (std::endian::native == std::endian::little) {
        if (bit_depth == 16)
            png_set_swap(png_ptr);
    }

    size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    KALI_ASSERT(row_bytes == buffer_size() / m_height);

    volatile png_bytepp rows = static_cast<png_bytepp>(alloca(sizeof(png_bytep) * m_height));
    for (size_t i = 0; i < m_height; i++)
        rows[i] = &m_data[row_bytes * i];

    png_write_image(png_ptr, rows);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    // delete[] text;
}

#endif // KALI_HAS_PNG

// ----------------------------------------------------------------------------
// JPEG I/O
// ----------------------------------------------------------------------------

#if KALI_HAS_JPEG

extern "C" {
static const size_t jpeg_buffer_size = 0x8000;

typedef struct {
    struct jpeg_source_mgr mgr;
    JOCTET* buffer;
    Stream* stream;
} jbuf_in_t;

typedef struct {
    struct jpeg_destination_mgr mgr;
    JOCTET* buffer;
    Stream* stream;
} jbuf_out_t;

static void jpeg_init_source(j_decompress_ptr cinfo)
{
    jbuf_in_t* p = (jbuf_in_t*)cinfo->src;
    p->buffer = new JOCTET[jpeg_buffer_size];
}

static boolean jpeg_fill_input_buffer(j_decompress_ptr cinfo)
{
    jbuf_in_t* p = (jbuf_in_t*)cinfo->src;
    size_t bytes_read = 0;

    try {
        p->stream->read(p->buffer, jpeg_buffer_size);
        bytes_read = jpeg_buffer_size;
    } catch (const EOFException& e) {
        bytes_read = e.gcount();
        if (bytes_read == 0) {
            // Insert a fake EOI marker
            p->buffer[0] = (JOCTET)0xFF;
            p->buffer[1] = (JOCTET)JPEG_EOI;
            bytes_read = 2;
        }
    }

    cinfo->src->bytes_in_buffer = bytes_read;
    cinfo->src->next_input_byte = p->buffer;
    return TRUE;
}

static void jpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    if (num_bytes > 0) {
        while (num_bytes > (long)cinfo->src->bytes_in_buffer) {
            num_bytes -= (long)cinfo->src->bytes_in_buffer;
            jpeg_fill_input_buffer(cinfo);
        }
        cinfo->src->next_input_byte += (size_t)num_bytes;
        cinfo->src->bytes_in_buffer -= (size_t)num_bytes;
    }
}

static void jpeg_term_source(j_decompress_ptr cinfo)
{
    jbuf_in_t* p = (jbuf_in_t*)cinfo->src;
    delete[] p->buffer;
}

static void jpeg_init_destination(j_compress_ptr cinfo)
{
    jbuf_out_t* p = (jbuf_out_t*)cinfo->dest;

    p->buffer = new JOCTET[jpeg_buffer_size];
    p->mgr.next_output_byte = p->buffer;
    p->mgr.free_in_buffer = jpeg_buffer_size;
}

static boolean jpeg_empty_output_buffer(j_compress_ptr cinfo)
{
    jbuf_out_t* p = (jbuf_out_t*)cinfo->dest;
    p->stream->write(p->buffer, jpeg_buffer_size);
    p->mgr.next_output_byte = p->buffer;
    p->mgr.free_in_buffer = jpeg_buffer_size;
    return TRUE;
}

static void jpeg_term_destination(j_compress_ptr cinfo)
{
    jbuf_out_t* p = (jbuf_out_t*)cinfo->dest;
    p->stream->write(p->buffer, jpeg_buffer_size - p->mgr.free_in_buffer);
    delete[] p->buffer;
    p->mgr.free_in_buffer = 0;
}

static void jpeg_error_exit(j_common_ptr cinfo)
{
    char msg[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, msg);
    KALI_THROW("libjpeg error: {}", msg);
}

}; // extern "C"

void Bitmap::read_jpg(Stream* stream)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    jbuf_in_t jbuf;

    std::memset(&jbuf, 0, sizeof(jbuf_in_t));

    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = jpeg_error_exit;
    jpeg_create_decompress(&cinfo);
    cinfo.src = (struct jpeg_source_mgr*)&jbuf;
    jbuf.mgr.init_source = jpeg_init_source;
    jbuf.mgr.fill_input_buffer = jpeg_fill_input_buffer;
    jbuf.mgr.skip_input_data = jpeg_skip_input_data;
    jbuf.mgr.term_source = jpeg_term_source;
    jbuf.mgr.resync_to_restart = jpeg_resync_to_restart;
    jbuf.stream = stream;

    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    m_width = cinfo.output_width;
    m_height = cinfo.output_height;
    m_component_type = ComponentType::uint8;
    m_srgb_gamma = true;

    switch (cinfo.output_components) {
    case 1:
        m_pixel_format = PixelFormat::y;
        m_channel_count = 1;
        break;
    case 3:
        m_pixel_format = PixelFormat::rgb;
        m_channel_count = 3;
        break;
    default:
        KALI_THROW("Unsupported number of components!");
    }

    auto fs = dynamic_cast<FileStream*>(stream);
    log_debug(
        "Reading JPEG file \"{}\" ({}x{}, {}, {}) ...",
        fs ? fs->path().string() : "<stream>",
        m_width,
        m_height,
        m_pixel_format,
        m_component_type
    );

    size_t row_stride = static_cast<size_t>(cinfo.output_width) * static_cast<size_t>(cinfo.output_components);

    m_data = std::unique_ptr<uint8_t[]>(new uint8_t[buffer_size()]);
    m_owns_data = true;

    JSAMPARRAY scanlines = reinterpret_cast<JSAMPARRAY>(alloca(sizeof(JSAMPROW) * m_height));
    for (size_t i = 0; i < m_height; ++i)
        scanlines[i] = static_cast<JSAMPROW>(uint8_data() + row_stride * i);

    // Process scanline by scanline.
    int counter = 0;
    while (cinfo.output_scanline < cinfo.output_height)
        counter += jpeg_read_scanlines(
            &cinfo,
            &scanlines[counter],
            static_cast<JDIMENSION>(m_height - cinfo.output_scanline)
        );

    // Release the libjpeg data structures.
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
}

void Bitmap::write_jpg(Stream* stream, int quality) const
{
    check_required_format("JPEG", {PixelFormat::y, PixelFormat::rgb}, {ComponentType::uint8});

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    jbuf_out_t jbuf;

    int components = 0;
    switch (m_pixel_format) {
    case PixelFormat::y:
        components = 1;
        break;
    case PixelFormat::rgb:
        components = 3;
        break;
    default:
        KALI_THROW("Unsupported pixel format!");
    }

    if (m_component_type != ComponentType::uint8)
        KALI_THROW("Unsupported component format {}, expected {}.", m_component_type, ComponentType::uint8);

    std::memset(&jbuf, 0, sizeof(jbuf_out_t));
    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = jpeg_error_exit;
    jpeg_create_compress(&cinfo);

    cinfo.dest = reinterpret_cast<jpeg_destination_mgr*>(&jbuf);
    jbuf.mgr.init_destination = jpeg_init_destination;
    jbuf.mgr.empty_output_buffer = jpeg_empty_output_buffer;
    jbuf.mgr.term_destination = jpeg_term_destination;
    jbuf.stream = stream;

    cinfo.image_width = static_cast<JDIMENSION>(m_width);
    cinfo.image_height = static_cast<JDIMENSION>(m_height);
    cinfo.input_components = components;
    cinfo.in_color_space = components == 1 ? JCS_GRAYSCALE : JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);

    if (quality == 100) {
        // Disable chroma subsampling.
        cinfo.comp_info[0].v_samp_factor = 1;
        cinfo.comp_info[0].h_samp_factor = 1;
    }

    jpeg_start_compress(&cinfo, TRUE);

    // Write scanline by scanline
    for (size_t i = 0; i < m_height; ++i) {
        const uint8_t* source = m_data.get() + i * m_width * cinfo.input_components;
        jpeg_write_scanlines(&cinfo, const_cast<JSAMPARRAY>(&source), 1);
    }

    // Release the libjpeg data structures.
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
}

#endif // KALI_HAS_JPEG

// ----------------------------------------------------------------------------
// BMP I/O
// ----------------------------------------------------------------------------

static void stbi_write_func(void* context, void* data, int size)
{
    static_cast<Stream*>(context)->write(data, size);
}

struct StreamReader {
    Stream* stream;
    size_t initial_pos;
    bool is_eof{false};
    stbi_io_callbacks callbacks;

    StreamReader(Stream* stream)
        : stream(stream)
        , initial_pos(stream->tell())
        , callbacks({.read = &read, .skip = &skip, .eof = &eof})
    {
    }

    void reset() { stream->seek(initial_pos); }

    static int read(void* user, char* data, int size)
    {
        StreamReader* reader = static_cast<StreamReader*>(user);
        try {
            reader->stream->read(data, size);
            return size;
        } catch (const EOFException& e) {
            reader->is_eof = true;
            return static_cast<int>(e.gcount());
        }
    }

    static void skip(void* user, int n)
    {
        StreamReader* reader = static_cast<StreamReader*>(user);
        reader->stream->seek(reader->stream->tell() + n);
    }

    static int eof(void* user)
    {
        StreamReader* reader = static_cast<StreamReader*>(user);
        return reader->is_eof;
    }
};

void Bitmap::read_bmp(Stream* stream)
{
    StreamReader reader(stream);
    int w, h, c;
    if (!stbi_info_from_callbacks(&reader.callbacks, &reader, &w, &h, &c))
        KALI_THROW("Failed to read BMP file!");
    reader.reset();

    m_width = w;
    m_height = h;
    m_pixel_format = c == 3 ? PixelFormat::rgb : PixelFormat::rgba;
    m_component_type = ComponentType::uint8;
    m_channel_count = c;
    m_channel_names = get_channel_names(m_pixel_format);
    m_srgb_gamma = true;

    auto fs = dynamic_cast<FileStream*>(stream);
    log_debug(
        "Reading BMP file \"{}\" ({}x{}, {}, {}) ...",
        fs ? fs->path().string() : "<stream>",
        m_width,
        m_height,
        m_pixel_format,
        m_component_type
    );

    uint8_t* data = stbi_load_from_callbacks(&reader.callbacks, &reader, &w, &h, &c, c);
    if (!data)
        KALI_THROW("Failed to read BMP file!");

    KALI_ASSERT_EQ(m_width, static_cast<uint32_t>(w));
    KALI_ASSERT_EQ(m_height, static_cast<uint32_t>(h));
    KALI_ASSERT_EQ(m_channel_count, static_cast<uint32_t>(c));

    m_data = std::unique_ptr<uint8_t[]>(data);
    m_owns_data = true;
}

void Bitmap::write_bmp(Stream* stream) const
{
    check_required_format("BMP", {PixelFormat::y, PixelFormat::rgb, PixelFormat::rgba}, {ComponentType::uint8});

    if (!stbi_write_bmp_to_func(&stbi_write_func, stream, m_width, m_height, m_channel_count, data()))
        KALI_THROW("Failed to write BMP file!");
}

// ----------------------------------------------------------------------------
// TGA I/O
// ----------------------------------------------------------------------------

void Bitmap::read_tga(Stream* stream)
{
    StreamReader reader(stream);
    int w, h, c;
    if (!stbi_info_from_callbacks(&reader.callbacks, &reader, &w, &h, &c))
        KALI_THROW("Failed to read TGA file!");
    reader.reset();

    m_width = w;
    m_height = h;
    m_pixel_format = c == 1 ? PixelFormat::y : (c == 3 ? PixelFormat::rgb : PixelFormat::rgba);
    m_component_type = ComponentType::uint8;
    m_channel_count = c;
    m_channel_names = get_channel_names(m_pixel_format);
    m_srgb_gamma = true;

    auto fs = dynamic_cast<FileStream*>(stream);
    log_debug(
        "Reading TGA file \"{}\" ({}x{}, {}, {}) ...",
        fs ? fs->path().string() : "<stream>",
        m_width,
        m_height,
        m_pixel_format,
        m_component_type
    );

    uint8_t* data = stbi_load_from_callbacks(&reader.callbacks, &reader, &w, &h, &c, c);
    if (!data)
        KALI_THROW("Failed to read TGA file!");

    KALI_ASSERT_EQ(m_width, static_cast<uint32_t>(w));
    KALI_ASSERT_EQ(m_height, static_cast<uint32_t>(h));
    KALI_ASSERT_EQ(m_channel_count, static_cast<uint32_t>(c));

    m_data = std::unique_ptr<uint8_t[]>(data);
    m_owns_data = true;
}

void Bitmap::write_tga(Stream* stream) const
{
    check_required_format("TGA", {PixelFormat::y, PixelFormat::rgb, PixelFormat::rgba}, {ComponentType::uint8});

    if (!stbi_write_tga_to_func(&stbi_write_func, stream, m_width, m_height, m_channel_count, data()))
        KALI_THROW("Failed to write BMP file!");
}

// ----------------------------------------------------------------------------
// HDR I/O
// ----------------------------------------------------------------------------

void Bitmap::read_hdr(Stream* stream)
{
    StreamReader reader(stream);
    int w, h, c;
    if (!stbi_info_from_callbacks(&reader.callbacks, &reader, &w, &h, &c))
        KALI_THROW("Failed to read HDR file!");
    reader.reset();

    m_width = w;
    m_height = h;
    m_pixel_format = PixelFormat::rgb;
    m_component_type = ComponentType::float32;
    m_channel_count = 3;
    m_channel_names = get_channel_names(m_pixel_format);
    m_srgb_gamma = false;

    auto fs = dynamic_cast<FileStream*>(stream);
    log_debug(
        "Reading HDR file \"{}\" ({}x{}, {}, {}) ...",
        fs ? fs->path().string() : "<stream>",
        m_width,
        m_height,
        m_pixel_format,
        m_component_type
    );

    float* data = stbi_loadf_from_callbacks(&reader.callbacks, &reader, &w, &h, &c, c);
    if (!data)
        KALI_THROW("Failed to read HDR file!");

    KALI_ASSERT_EQ(m_width, static_cast<uint32_t>(w));
    KALI_ASSERT_EQ(m_height, static_cast<uint32_t>(h));
    KALI_ASSERT_EQ(m_channel_count, static_cast<uint32_t>(c));

    m_data = std::unique_ptr<uint8_t[]>(reinterpret_cast<uint8_t*>(data));
    m_owns_data = true;
}

void Bitmap::write_hdr(Stream* stream) const
{
    check_required_format("HDR", {PixelFormat::rgb}, {ComponentType::float32});

    if (!stbi_write_hdr_to_func(
            &stbi_write_func,
            stream,
            m_width,
            m_height,
            m_channel_count,
            reinterpret_cast<const float*>(data())
        ))
        KALI_THROW("Failed to write HDR file!");
}

// ----------------------------------------------------------------------------
// OpenEXR I/O
// ----------------------------------------------------------------------------

#if KALI_HAS_OPENEXR

class EXRIStream : public Imf::IStream {
public:
    EXRIStream(Stream* stream)
        : IStream(stream->to_string().c_str())
        , m_stream(stream)
    {
        m_offset = stream->tell();
        m_size = stream->size();
    }

    bool isMemoryMapped() const override
    {
        // TODO add support for memory mapped streams
        return false;
    }

    bool read(char* c, int n) override
    {
        m_stream->read(c, n);
        return m_stream->tell() == m_size;
    }

    char* readMemoryMapped(int n) override
    {
        // TODO add support for memory mapped streams
        KALI_UNUSED(n);
        return nullptr;
    }

    uint64_t tellg() override { return m_stream->tell() - m_offset; }

    void seekg(uint64_t pos) override { m_stream->seek((size_t)pos + m_offset); }

    void clear() override { }

private:
    ref<Stream> m_stream;
    size_t m_offset, m_size;
};

class EXROStream : public Imf::OStream {
public:
    EXROStream(Stream* stream)
        : OStream(stream->to_string().c_str())
        , m_stream(stream)
    {
    }

    void write(const char* c, int n) override { m_stream->write(c, n); }

    uint64_t tellp() override { return m_stream->tell(); }

    void seekp(uint64_t pos) override { m_stream->seek((size_t)pos); }

private:
    ref<Stream> m_stream;
};

void Bitmap::read_exr(Stream* stream)
{
    EXRIStream is(stream);
    Imf::InputFile file(is);

    const Imf::Header& header = file.header();
    const Imf::ChannelList& channels = header.channels();

    if (channels.begin() == channels.end())
        KALI_THROW("EXR image does not contain any channels!");

#if 0
    // Load meta data
    for (auto it = header.begin(); it != header.end(); ++it) {
        std::string name = it.name();
        const Imf::Attribute* attr = &it.attribute();
        std::string type_name = attr->typeName();

        if (type_name == "string") {
            auto v = static_cast<const Imf::StringAttribute*>(attr);
            m_metadata.set_string(name, v->value());
        } else if (type_name == "int") {
            auto v = static_cast<const Imf::IntAttribute*>(attr);
            m_metadata.set_long(name, v->value());
        } else if (type_name == "float") {
            auto v = static_cast<const Imf::FloatAttribute*>(attr);
            m_metadata.set_float(name, Properties::Float(v->value()));
        } else if (type_name == "double") {
            auto v = static_cast<const Imf::DoubleAttribute*>(attr);
            m_metadata.set_float(name, Properties::Float(v->value()));
        } else if (type_name == "v3f") {
            auto v = static_cast<const Imf::V3fAttribute*>(attr);
            Imath::V3f vec = v->value();
            m_metadata.set_array3f(name, Vector3f(vec.x, vec.y, vec.z));
        } else if (type_name == "m44f") {
            auto v = static_cast<const Imf::M44fAttribute*>(attr);
            Matrix4f M;
            for (size_t i = 0; i < 4; ++i)
                for (size_t j = 0; j < 4; ++j)
                    M(i, j) = v->value().x[i][j];
            m_metadata.set_transform(name, Transform4f(M));
        } else if (type_name == "m33f") {
            auto v = static_cast<const Imf::M33fAttribute*>(attr);
            Matrix3f M;
            for (size_t i = 0; i < 3; ++i)
                for (size_t j = 0; j < 3; ++j)
                    M(i, j) = v->value().x[i][j];
            m_metadata.set_transform3f(name, Transform3f(M));
        }
    }
#endif

    // bool process_colors = false;
    // m_srgb_gamma = false;
    // m_premultiplied_alpha = true;
    // m_pixel_format = PixelFormat::MultiChannel;
    // m_struct = new Struct();
    Imf::PixelType pixel_type = channels.begin().channel().type;

    switch (pixel_type) {
    case Imf::HALF:
        m_component_type = ComponentType::float16;
        break;
    case Imf::FLOAT:
        m_component_type = ComponentType::float32;
        break;
    case Imf::UINT:
        m_component_type = ComponentType::uint32;
        break;
    default:
        KALI_THROW("EXR image contains invalid component type (must be float16, float32 or uint32)");
    }

    enum { Unknown, R, G, B, X, Y, Z, A, RY, BY, NumClasses };

    // Classification scheme for color channels
    auto channel_class = [](std::string name) -> uint8_t
    {
        auto it = name.rfind(".");
        if (it != std::string::npos)
            name = name.substr(it + 1);
        name = string::to_lower(name);
        if (name == "r")
            return R;
        if (name == "g")
            return G;
        if (name == "b")
            return B;
        if (name == "x")
            return X;
        if (name == "y")
            return Y;
        if (name == "z")
            return Z;
        if (name == "ry")
            return RY;
        if (name == "by")
            return BY;
        if (name == "a")
            return A;
        return Unknown;
    };

    // Assign a sorting key to color channels
    auto channel_key = [&](std::string name) -> std::string
    {
        uint8_t class_ = channel_class(name);
        if (class_ == Unknown)
            return name;
        auto it = name.rfind(".");
        char suffix('0' + class_);
        if (it != std::string::npos)
            name = name.substr(0, it) + "." + suffix;
        else
            name = suffix;
        return name;
    };

#if 0

    bool found[NumClasses] = {false};
    std::vector<std::string> channels_sorted;
    for (auto it = channels.begin(); it != channels.end(); ++it) {
        std::string name(it.name());
        found[channel_class(name)] = true;
        channels_sorted.push_back(name);
    }

    std::sort(
        channels_sorted.begin(),
        channels_sorted.end(),
        [&](auto const& v0, auto const& v1) { return channel_key(v0) < channel_key(v1); }
    );

    // Order channel names based on RGB/XYZ[A] suffix
    for (auto const& name : channels_sorted) {
        uint32_t flags = +Struct::Flags::Empty;
        // Tag alpha channels to be able to perform operations depending on alpha
        auto c_class = channel_class(name);
        if (c_class == A)
            flags |= +Struct::Flags::Alpha;

        // Currently we don't support alpha transformations for multi-layer images
        // So it is okay to just set this for all non-alpha channels
        if (c_class != A)
            flags |= +Struct::Flags::PremultipliedAlpha;
        m_struct->append(name, m_component_format, flags);
    }

    // Attempt to detect a standard combination of color channels
    m_pixel_format = PixelFormat::MultiChannel;
    bool luminance_chroma_format = false;
    if (found[R] && found[G] && found[B]) {
        if (m_struct->field_count() == 3)
            m_pixel_format = PixelFormat::RGB;
        else if (found[A] && m_struct->field_count() == 4)
            m_pixel_format = PixelFormat::RGBA;
    } else if (found[X] && found[Y] && found[Z]) {
        if (m_struct->field_count() == 3)
            m_pixel_format = PixelFormat::XYZ;
        else if (found[A] && m_struct->field_count() == 4)
            m_pixel_format = PixelFormat::XYZA;
    } else if (found[Y] && found[RY] && found[BY]) {
        if (m_struct->field_count() == 3)
            m_pixel_format = PixelFormat::RGB;
        else if (found[A] && m_struct->field_count() == 4)
            m_pixel_format = PixelFormat::RGBA;
        luminance_chroma_format = true;
    } else if (found[Y]) {
        if (m_struct->field_count() == 1)
            m_pixel_format = PixelFormat::Y;
        else if (found[A] && m_struct->field_count() == 2)
            m_pixel_format = PixelFormat::YA;
    }

    // Check if there is a chromaticity header entry
    Imf::Chromaticities file_chroma;
    if (Imf::hasChromaticities(file.header()))
        file_chroma = Imf::chromaticities(file.header());

    auto chroma_eq = [](const Imf::Chromaticities& a, const Imf::Chromaticities& b)
    {
        return (a.red - b.red).length2() + (a.green - b.green).length2() + (a.blue - b.blue).length2()
            + (a.white - b.white).length2()
            < 1e-6f;
    };

    auto set_suffix = [](std::string name, const std::string& suffix)
    {
        auto it = name.rfind(".");
        if (it != std::string::npos)
            name = name.substr(0, it) + "." + suffix;
        else
            name = suffix;
        return name;
    };

    Imath::Box2i data_window = file.header().dataWindow();
    m_size = Vector2u(data_window.max.x - data_window.min.x + 1, data_window.max.y - data_window.min.y + 1);

    // Compute pixel / row strides
    size_t pixel_stride = bytes_per_pixel(), row_stride = pixel_stride * m_size.x(), pixel_count = this->pixel_count();

    // Finally, allocate memory for it
    m_data = std::unique_ptr<uint8_t[]>(new uint8_t[row_stride * m_size.y()]);
    m_owns_data = true;

    using ResampleBuffer = std::pair<std::string, ref<Bitmap>>;
    std::vector<ResampleBuffer> resample_buffers;

    uint8_t* ptr = m_data.get() - (data_window.min.x + data_window.min.y * m_size.x()) * pixel_stride;

    // Tell OpenEXR where the image data should be put
    Imf::FrameBuffer framebuffer;
    for (auto const& field : *m_struct) {
        const Imf::Channel& channel = channels[field.name];
        Vector2i sampling(channel.xSampling, channel.ySampling);
        Imf::Slice slice;

        if (sampling == Vector2i(1)) {
            // This is a full resolution channel. Load the ordinary way
            slice = Imf::Slice(pixel_type, (char*)(ptr + field.offset), pixel_stride, row_stride);
        } else {
            // Uh oh, this is a sub-sampled channel. We will need to scale it
            Vector2u channel_size = m_size / sampling;

            ref<Bitmap> bitmap = new Bitmap(PixelFormat::Y, m_component_format, channel_size);

            uint8_t* ptr_nested = bitmap->uint8_data()
                - (data_window.min.x / sampling.x() + data_window.min.y / sampling.y() * (size_t)channel_size.x())
                    * field.size;

            slice = Imf::Slice(
                pixel_type,
                (char*)ptr_nested,
                field.size,
                field.size * channel_size.x(),
                sampling.x(),
                sampling.y()
            );

            resample_buffers.emplace_back(field.name, std::move(bitmap));
        }

        framebuffer.insert(field.name, slice);
    }

    auto fs = dynamic_cast<FileStream*>(stream);
    Log(Debug,
        "Loading OpenEXR file \"%s\" (%ix%i, %s, %s) ..",
        fs ? fs->path().string() : "<stream>",
        m_size.x(),
        m_size.y(),
        m_pixel_format,
        m_component_format);

    file.setFrameBuffer(framebuffer);
    file.readPixels(data_window.min.y, data_window.max.y);

    for (auto& buf : resample_buffers) {
        Log(Debug,
            "Upsampling layer \"%s\" from %ix%i to %ix%i pixels",
            buf.first,
            buf.second->width(),
            buf.second->height(),
            m_size.x(),
            m_size.y());

        buf.second = buf.second->resample(m_size);
        const Struct::Field& field = m_struct->field(buf.first);

        size_t comp_size = field.size;
        uint8_t* dst = uint8_data() + field.offset;
        uint8_t* src = buf.second->uint8_data();

        for (size_t j = 0; j < pixel_count; ++j) {
            std::memcpy(dst, src, comp_size);
            src += comp_size;
            dst += pixel_stride;
        }

        buf.second = nullptr;
    }

    if (luminance_chroma_format) {
        Log(Debug, "Converting from Luminance-Chroma to RGB format ..");
        Imath::V3f yw = Imf::RgbaYca::computeYw(file_chroma);

        auto convert = [&](auto* data)
        {
            using T = std::decay_t<decltype(*data)>;

            for (size_t j = 0; j < pixel_count; ++j) {
                Float Y = (Float)data[0], RY = (Float)data[1], BY = (Float)data[2];

                if (std::is_integral<T>::value) {
                    Float scale = Float(1) / Float(std::numeric_limits<T>::max());
                    Y *= scale;
                    RY *= scale;
                    BY *= scale;
                }

                Float R = (RY + 1.f) * Y, B = (BY + 1.f) * Y, G = ((Y - R * yw.x - B * yw.z) / yw.y);

                if (std::is_integral<T>::value) {
                    Float scale = Float(std::numeric_limits<T>::max());
                    R *= R * scale + .5f;
                    G *= G * scale + .5f;
                    B *= B * scale + .5f;
                }

                data[0] = T(R);
                data[1] = T(G);
                data[2] = T(B);
                data += channel_count();
            }
        };

        switch (m_component_format) {
        case Struct::Type::Float16:
            convert((dr::half*)m_data.get());
            break;
        case Struct::Type::Float32:
            convert((float*)m_data.get());
            break;
        case Struct::Type::UInt32:
            convert((uint32_t*)m_data.get());
            break;
        default:
            Throw("Internal error!");
        }

        for (int i = 0; i < 3; ++i) {
            std::string& name = m_struct->operator[](i).name;
            name = set_suffix(name, std::string(1, "RGB"[i]));
        }
    }

    if (Imf::hasChromaticities(file.header())
        && (m_pixel_format == PixelFormat::RGB || m_pixel_format == PixelFormat::RGBA)) {

        Imf::Chromaticities itu_rec_b_709;
        Imf::Chromaticities
            xyz(Imath::V2f(1.f, 0.f), Imath::V2f(0.f, 1.f), Imath::V2f(0.f, 0.f), Imath::V2f(1.f / 3.f, 1.f / 3.f));

        if (chroma_eq(file_chroma, itu_rec_b_709)) {
            // Already in the right space -- do nothing.
        } else if (chroma_eq(file_chroma, xyz)) {
            // This is an XYZ image
            m_pixel_format = m_pixel_format == PixelFormat::RGB ? PixelFormat::XYZ : PixelFormat::XYZA;
            for (int i = 0; i < 3; ++i) {
                std::string& name = m_struct->operator[](i).name;
                name = set_suffix(name, std::string(1, "XYZ"[i]));
            }
        } else {
            // Non-standard chromaticities. Special processing is required..
            process_colors = true;
        }
    }

    if (process_colors) {
        // Convert ITU-R Rec. BT.709 linear RGB
        Imath::M44f M = Imf::RGBtoXYZ(file_chroma, 1) * Imf::XYZtoRGB(Imf::Chromaticities(), 1);

        Log(Debug, "Converting to sRGB color space ..");

        auto convert = [&](auto* data)
        {
            using T = std::decay_t<decltype(*data)>;

            for (size_t j = 0; j < pixel_count; ++j) {
                Float R = (Float)data[0], G = (Float)data[1], B = (Float)data[2];

                if (std::is_integral<T>::value) {
                    Float scale = Float(1) / Float(std::numeric_limits<T>::max());
                    R *= scale;
                    G *= scale;
                    B *= scale;
                }

                Imath::V3f rgb = Imath::V3f(float(R), float(G), float(B)) * M;
                R = Float(rgb[0]);
                G = Float(rgb[1]);
                B = Float(rgb[2]);

                if (std::is_integral<T>::value) {
                    Float scale = Float(std::numeric_limits<T>::max());
                    R *= R * scale + 0.5f;
                    G *= G * scale + 0.5f;
                    B *= B * scale + 0.5f;
                }

                data[0] = T(R);
                data[1] = T(G);
                data[2] = T(B);
                data += channel_count();
            }
        };

        switch (m_component_format) {
        case Struct::Type::Float16:
            convert((dr::half*)m_data.get());
            break;
        case Struct::Type::Float32:
            convert((float*)m_data.get());
            break;
        case Struct::Type::UInt32:
            convert((uint32_t*)m_data.get());
            break;
        default:
            Throw("Internal error!");
        }
    }
#endif
}

void Bitmap::write_exr(Stream* stream, int quality) const
{
    check_required_format(
        "EXR",
        {PixelFormat::y, PixelFormat::ya, PixelFormat::rgb, PixelFormat::rgba, PixelFormat::multi_channel},
        {ComponentType::uint32, ComponentType::float16, ComponentType::float32}
    );

    Imf::Header header(
        static_cast<int>(m_width),                                  // width
        static_cast<int>(m_height),                                 // height,
        1.f,                                                        // pixelAspectRatio
        Imath::V2f(0, 0),                                           // screenWindowCenter,
        1.f,                                                        // screenWindowWidth
        Imf::INCREASING_Y,                                          // lineOrder
        quality <= 0 ? Imf::PIZ_COMPRESSION : Imf::DWAB_COMPRESSION // compression
    );

    if (quality > 0)
        header.dwaCompressionLevel() = static_cast<float>(quality);

#if 0
    Properties metadata(m_metadata);
    if (!metadata.has_property("generatedBy"))
        metadata.set_string("generatedBy", "Mitsuba version " MI_VERSION);

    std::vector<std::string> keys = metadata.property_names();
#endif

#if 0
    for (auto it = keys.begin(); it != keys.end(); ++it) {
        using Type = Properties::Type;

        Type type = metadata.type(*it);
        if (*it == "pixelAspectRatio" || *it == "screenWindowWidth" || *it == "screenWindowCenter")
            continue;

        switch (type) {
        case Type::String:
            header.insert(it->c_str(), Imf::StringAttribute(metadata.string(*it)));
            break;
        case Type::Long:
            header.insert(it->c_str(), Imf::IntAttribute(metadata.get<int>(*it)));
            break;
        case Type::Float:
            header.insert(it->c_str(), Imf::DoubleAttribute(metadata.get<double>(*it)));
            break;
        case Type::Array3f: {
            Vector3f val = metadata.get<Vector3f>(*it);
            header.insert(it->c_str(), Imf::V3fAttribute(Imath::V3f((float)val.x(), (float)val.y(), (float)val.z())));
        } break;
        case Type::Transform3f: {
            Matrix3f val = metadata.get<ScalarTransform3f>(*it).matrix;
            header.insert(
                it->c_str(),
                Imf::M33fAttribute(Imath::M33f(
                    (float)val(0, 0),
                    (float)val(0, 1),
                    (float)val(0, 2),
                    (float)val(1, 0),
                    (float)val(1, 1),
                    (float)val(1, 2),
                    (float)val(2, 0),
                    (float)val(2, 1),
                    (float)val(2, 2)
                ))
            );
        } break;
        case Type::Transform4f: {
            Matrix4f val = metadata.get<ScalarTransform4f>(*it).matrix;
            header.insert(
                it->c_str(),
                Imf::M44fAttribute(Imath::M44f(
                    (float)val(0, 0),
                    (float)val(0, 1),
                    (float)val(0, 2),
                    (float)val(0, 3),
                    (float)val(1, 0),
                    (float)val(1, 1),
                    (float)val(1, 2),
                    (float)val(1, 3),
                    (float)val(2, 0),
                    (float)val(2, 1),
                    (float)val(2, 2),
                    (float)val(2, 3),
                    (float)val(3, 0),
                    (float)val(3, 1),
                    (float)val(3, 2),
                    (float)val(3, 3)
                ))
            );
        } break;
        default:
            header.insert(it->c_str(), Imf::StringAttribute(metadata.as_string(*it)));
            break;
        }
    }
#endif

#if 0
    if (m_pixel_format == PixelFormat::XYZ || m_pixel_format == PixelFormat::XYZA) {
        Imf::addChromaticities(
            header,
            Imf::Chromaticities(
                Imath::V2f(1.f, 0.f),
                Imath::V2f(0.f, 1.f),
                Imath::V2f(0.f, 0.f),
                Imath::V2f(1.f / 3.f, 1.f / 3.f)
            )
        );
    }
#endif

    Imf::PixelType pixel_type;
    switch (m_component_type) {
    case ComponentType::uint32:
        pixel_type = Imf::UINT;
        break;
    case ComponentType::float16:
        pixel_type = Imf::HALF;
        break;
    case ComponentType::float32:
        pixel_type = Imf::FLOAT;
        break;
    default:
        KALI_THROW("Unsupported component type!");
    }

    size_t component_size = Struct::type_size(m_component_type);
    size_t pixel_stride = component_size * m_channel_count;
    size_t row_stride = pixel_stride * m_width;

    Imf::ChannelList& channels = header.channels();
    Imf::FrameBuffer framebuffer;
    const uint8_t* ptr = uint8_data();
    KALI_ASSERT(m_channel_count == m_channel_names.size());
    for (uint32_t i = 0; i < m_channel_count; ++i) {
        const std::string& channel_name = m_channel_names[i];
        Imf::Slice slice(pixel_type, (char*)(ptr + i * component_size), pixel_stride, row_stride);
        channels.insert(channel_name, Imf::Channel(pixel_type));
        framebuffer.insert(channel_name, slice);
    }

    EXROStream os(stream);
    Imf::OutputFile file(os, header);
    file.setFrameBuffer(framebuffer);
    file.writePixels(static_cast<int>(m_height));
}

#endif // KALI_HAS_OPENEXR

} // namespace kali
