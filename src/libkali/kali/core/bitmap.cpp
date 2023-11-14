// TODO: MITSUBA license

#include "bitmap.h"
#include "kali/core/macros.h"
#include "kali/core/error.h"
#include "kali/core/logger.h"
#include "kali/core/file_stream.h"

#include <png.h>
#include <jpeglib.h>
#include <turbojpeg.h>

KALI_DISABLE_MSVC_WARNING(4611)

namespace kali {

inline size_t get_component_byte_size(Bitmap::ComponentType component_type)
{
    switch (component_type) {
    case Bitmap::ComponentType::u8:
        return 1;
    case Bitmap::ComponentType::u16:
        return 2;
    case Bitmap::ComponentType::u32:
        return 4;
    case Bitmap::ComponentType::f16:
        return 2;
    case Bitmap::ComponentType::f32:
        return 4;
    }
    KALI_THROW("Invalid component type");
}

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
    const void* data
)
    : m_pixel_format(pixel_format)
    , m_component_type(component_type)
    , m_width(width)
    , m_height(height)
    , m_channel_count(channel_count)
    , m_channel_names(channel_names)
{
    KALI_CHECK(
        pixel_format != PixelFormat::multi_channel || channel_count > 0,
        "Expected non-zero channel count for multi-channel pixel format."
    );

    if (pixel_format != PixelFormat::multi_channel) {
        m_channel_names = get_channel_names(pixel_format);
        m_channel_count = static_cast<uint32_t>(m_channel_names.size());
    }

    KALI_UNUSED(data);
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

Bitmap::~Bitmap() { }

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

void Bitmap::write(Stream* stream, FileFormat format, int quality) const
{
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
    return get_component_byte_size(m_component_type) * m_channel_count;
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
        m_component_type = ComponentType::u8;
        break;
    case 16:
        m_component_type = ComponentType::u16;
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
    case ComponentType::u8:
        bit_depth = 8;
        break;
    case ComponentType::u16:
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
    m_component_type = ComponentType::u8;
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

    if (m_component_type != ComponentType::u8)
        KALI_THROW("Unsupported component format {}, expected {}.", m_component_type, ComponentType::u8);

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


} // namespace kali
