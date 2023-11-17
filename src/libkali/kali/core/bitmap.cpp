// TODO: MITSUBA license

#include "bitmap.h"
#include "kali/core/macros.h"
#include "kali/core/error.h"
#include "kali/core/logger.h"
#include "kali/core/file_stream.h"
#include "kali/core/string.h"

#include <png.h>
#include <jpeglib.h>
#include <turbojpeg.h>

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
    KALI_UNUSED(path);
    KALI_UNUSED(format);
}

Bitmap::~Bitmap()
{
    if (!m_owns_data)
        m_data.release();
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
        if (quality == -1)
            quality = 5;
        write_png(stream, quality);
        break;
    case FileFormat::jpg:
        if (quality == -1)
            quality = 100;
        write_jpg(stream, quality);
        break;
    case FileFormat::bmp:
        write_bmp(stream);
        break;
    case FileFormat::tga:
        // write_tga(stream);
        break;
    case FileFormat::hdr:
        // write_hdr(stream);
        break;
    case FileFormat::exr:
        // write_exr(stream, quality);
        break;
    default:
        KALI_THROW("Invalid file format!");
    }

    KALI_UNUSED(stream);
    KALI_UNUSED(format);
    KALI_UNUSED(quality);
}

void Bitmap::write(const std::filesystem::path& path, FileFormat format, int quality) const
{
    auto stream = make_ref<FileStream>(path, FileStream::Mode::write);
    write(stream, format, quality);
}

std::future<void> Bitmap::write_async(ref<Stream> stream, FileFormat format, int quality) const
{
    KALI_UNUSED(stream);
    KALI_UNUSED(format);
    KALI_UNUSED(quality);
    // FileStream* fs = dynamic_cast<FileStream*>(stream.get());
    // if (fs) {
    //     fs->path().extension();
    // }
    return {};
}

std::future<void> Bitmap::write_async(const std::filesystem::path& path, FileFormat format, int quality) const
{
    auto stream = make_ref<FileStream>(path, FileStream::Mode::write);
    return write_async(stream, format, quality);
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

bool Bitmap::operator==(const Bitmap& bitmap) const
{
    return m_pixel_format == bitmap.m_pixel_format && m_component_type == bitmap.m_component_type
        && m_width == bitmap.m_width && m_height == bitmap.m_height && m_channel_count == bitmap.m_channel_count
        && m_channel_names == bitmap.m_channel_names
        && std::memcmp(m_data.get(), bitmap.m_data.get(), buffer_size()) == 0;
}

std::string Bitmap::to_string() const
{
    return fmt::format(
        "Bitmap(\n"
        "  pixel_format = {},\n"
        "  component_type = {},\n"
        "  width = {},\n"
        "  height = {},\n"
        "  data = {},\n"
        ")",
        m_pixel_format,
        m_component_type,
        m_width,
        m_height,
        string::format_byte_size(buffer_size())
    );
}

void Bitmap::static_init() { }

void Bitmap::static_shutdown() { }

void Bitmap::read(Stream* stream, FileFormat format)
{
    if (format == FileFormat::auto_)
        format = detect_file_format(stream);

    switch (format) {
    case FileFormat::png:
        read_png(stream);
        break;
    case FileFormat::jpg:
        read_jpg(stream);
        break;
    case FileFormat::bmp:
        read_bmp(stream);
        break;
    case FileFormat::tga:
        // read_tga(stream);
        // break;
    case FileFormat::hdr:
        // read_hdr(stream);
        // break;
    case FileFormat::exr:
        // read_exr(stream);
        break;
    }
    KALI_THROW("Unknown file format!");
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
        // } else if (header[0] == 'P' && (header[1] == 'F' || header[1] == 'f')) {
        //     format = FileFormat::PFM;
        // } else if (header[0] == 'P' && header[1] == '6') {
        //     format = FileFormat::PPM;
    } else if (header[0] == 0xFF && header[1] == 0xD8) {
        format = FileFormat::jpg;
    } else if (png_sig_cmp(header, 0, 8) == 0) {
        format = FileFormat::png;
        // } else if (Imf::isImfMagic((const char*)header)) {
        //     format = FileFormat::OpenEXR;
    } else {
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
    FileFormat file_format,
    std::vector<Bitmap::PixelFormat> allowed_pixel_formats,
    std::vector<Bitmap::ComponentType> allowed_component_types
) const
{
    if (std::find(allowed_pixel_formats.begin(), allowed_pixel_formats.end(), m_pixel_format)
        == allowed_pixel_formats.end())
        KALI_THROW(
            "Unsupported pixel format \"{}\" for writing \"{}\" image, expected one of: {}.",
            m_pixel_format,
            file_format,
            fmt::join(allowed_pixel_formats, ", ")
        );
    if (std::find(allowed_component_types.begin(), allowed_component_types.end(), m_component_type)
        == allowed_component_types.end())
        KALI_THROW(
            "Unsupported component type \"{}\" for writing \"{}\" image, expected one of: {}.",
            m_component_type,
            file_format,
            fmt::join(allowed_component_types, ", ")
        );
}

//
// PNG I/O
//

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

#if defined(LITTLE_ENDIAN)
    // Swap the byte order on little endian machines.
    if (bit_depth == 16)
        png_set_swap(png_ptr);
#endif

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
        break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
        m_pixel_format = PixelFormat::ya;
        break;
    case PNG_COLOR_TYPE_RGB:
        m_pixel_format = PixelFormat::rgb;
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        m_pixel_format = PixelFormat::rgba;
        break;
    default:
        KALI_THROW("Unknown color type {}!", color_type);
    }

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
#if 0
    m_owns_data = true;
#endif

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
        FileFormat::png,
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
    memset(text, 0, sizeof(png_text) * keys.size());

    for (size_t i = 0; i < keys.size(); ++i) {
        values[i] = metadata.as_string(keys[i]);
        text[i].key = const_cast<char*>(keys[i].c_str());
        text[i].text = const_cast<char*>(values[i].c_str());
        text[i].compression = PNG_TEXT_COMPRESSION_NONE;
    }

    png_set_text(png_ptr, info_ptr, text, (int)keys.size());
#endif

#if 0
    if (m_srgb_gamma)
        png_set_sRGB_gAMA_and_cHRM(png_ptr, info_ptr, PNG_sRGB_INTENT_ABSOLUTE);
#endif

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

#if defined(LITTLE_ENDIAN)
    // Swap the byte order on little endian machines.
    if (bit_depth == 16)
        png_set_swap(png_ptr);
#endif

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

//
// JPG I/O
//

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

    memset(&jbuf, 0, sizeof(jbuf_in_t));

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
#if 0
    m_srgb_gamma = true;
    m_premultiplied_alpha = false;
#endif

    switch (cinfo.output_components) {
    case 1:
        m_pixel_format = PixelFormat::y;
        break;
    case 3:
        m_pixel_format = PixelFormat::rgb;
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
    // m_owns_data = true;

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
    check_required_format(FileFormat::jpg, {PixelFormat::y, PixelFormat::rgb}, {ComponentType::uint8});

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

//
// BMP I/O
//

static void stbi_write_func(void* context, void* data, int size)
{
    static_cast<Stream*>(context)->write(data, size);
}

void Bitmap::read_bmp(Stream* stream)
{
    KALI_UNUSED(stream);
    KALI_UNIMPLEMENTED();
}

void Bitmap::write_bmp(Stream* stream) const
{
    check_required_format(
        FileFormat::bmp,
        {PixelFormat::y, PixelFormat::rgb, PixelFormat::rgba},
        {ComponentType::uint8}
    );

    if (!stbi_write_bmp_to_func(&stbi_write_func, stream, m_width, m_height, channel_count(), data()))
        KALI_THROW("Failed to write BMP file!");
}

void Bitmap::read_tga(Stream* stream)
{
    KALI_UNUSED(stream);
    KALI_UNIMPLEMENTED();
}

void Bitmap::write_tga(Stream* stream) const
{
    check_required_format(
        FileFormat::tga,
        {PixelFormat::y, PixelFormat::rgb, PixelFormat::rgba},
        {ComponentType::uint8}
    );

    if (!stbi_write_bmp_to_func(&stbi_write_func, stream, m_width, m_height, channel_count(), data()))
        KALI_THROW("Failed to write BMP file!");
}


} // namespace kali
