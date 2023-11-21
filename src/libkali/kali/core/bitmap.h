// TODO: MITSUBA license

#pragma once

#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/enum.h"
#include "kali/core/stream.h"
#include "kali/core/struct.h"

#include <filesystem>
#include <future>
#include <memory>
#include <string>
#include <vector>

namespace kali {

class KALI_API Bitmap : public Object {
    KALI_OBJECT(Bitmap)
public:
    enum class FileFormat {
        unknown,
        auto_,
        png,
        jpg,
        bmp,
        tga,
        hdr,
        exr,
    };

    KALI_ENUM_INFO(
        FileFormat,
        {
            {FileFormat::unknown, "unknown"},
            {FileFormat::auto_, "auto"},
            {FileFormat::png, "png"},
            {FileFormat::jpg, "jpg"},
            {FileFormat::bmp, "bmp"},
            {FileFormat::tga, "tga"},
            {FileFormat::hdr, "hdr"},
            {FileFormat::exr, "exr"},
        }
    );

    enum class PixelFormat {
        /// Luminance only.
        y,
        /// Luminance + alpha.
        ya,
        /// RGB.
        rgb,
        /// RGB + alpha.
        rgba,
        /// Arbitrary multi-channel.
        multi_channel,
    };

    KALI_ENUM_INFO(
        PixelFormat,
        {
            {PixelFormat::y, "y"},
            {PixelFormat::ya, "ya"},
            {PixelFormat::rgb, "rgb"},
            {PixelFormat::rgba, "rgba"},
            {PixelFormat::multi_channel, "multi_channel"},
        }
    );

    using ComponentType = Struct::Type;

    Bitmap(
        PixelFormat pixel_format,
        ComponentType component_type,
        uint32_t width,
        uint32_t height,
        uint32_t channel_count = 0,
        const std::vector<std::string>& channel_names = {},
        void* data = nullptr
    );

    Bitmap(Stream* stream, FileFormat format = FileFormat::auto_);

    Bitmap(const std::filesystem::path& path, FileFormat format = FileFormat::auto_);

    /// Copy constructor.
    Bitmap(const Bitmap& other);

    /// Move constructor.
    Bitmap(Bitmap&& other);

    /// Destructor.
    ~Bitmap();

#if 0
    /// Read a bitmap from a stream.
    /// @param stream The stream to read from.
    /// @param format The file format to read (auto-detect by default).
    /// @return The loaded bitmap.
    static ref<Bitmap> read(Stream* stream, FileFormat format = FileFormat::auto_);

    /// Read a bitmap from a file.
    /// @param path The file path to read from.
    /// @param format The file format to read (auto-detect by default).
    /// @return The loaded bitmap.
    static ref<Bitmap> read(const std::filesystem::path& path, FileFormat format = FileFormat::auto_);

    /// Read a bitmap from a stream asynchronously.
    /// @param stream The stream to read from.
    /// @param format The file format to read (auto-detect by default).
    /// @return A future to the loaded bitmap.
    static std::future<ref<Bitmap>> read_async(ref<Stream> stream, FileFormat format = FileFormat::auto_);

    /// Read a bitmap from a file asynchronously.
    /// @param path The file path to read from.
    /// @param format The file format to read (auto-detect by default).
    /// @return A future to the loaded bitmap.
    static std::future<ref<Bitmap>>
    read_async(const std::filesystem::path& path, FileFormat format = FileFormat::auto_);
#endif

    void write(Stream* stream, FileFormat format = FileFormat::auto_, int quality = -1) const;
    void write(const std::filesystem::path& path, FileFormat format = FileFormat::auto_, int quality = -1) const;

    void write_async(const std::filesystem::path& path, FileFormat format = FileFormat::auto_, int quality = -1) const;

    /// The pixel format.
    PixelFormat pixel_format() const { return m_pixel_format; }

    /// The component type.
    ComponentType component_type() const { return m_component_type; }

    /// The width of the bitmap in pixels.
    uint32_t width() const { return m_width; }

    /// The height of the bitmap in pixels.
    uint32_t height() const { return m_height; }

    /// The total number of pixels in the bitmap.
    size_t pixel_count() const { return static_cast<size_t>(m_width) * m_height; }

    /// The number of channels in the bitmap.
    uint32_t channel_count() const { return m_channel_count; }

    /// The names of the channels in the bitmap.
    const std::vector<std::string>& channel_names() const { return m_channel_names; }

    /// True if the bitmap is in sRGB gamma space.
    bool srgb_gamma() const { return m_srgb_gamma; }

    /// Set the sRGB gamma flag.
    /// Note that this does not convert the pixel values, it only sets the flag.
    void set_srgb_gamma(bool srgb_gamma) { m_srgb_gamma = srgb_gamma; }

    /// Returns true if the bitmap has an alpha channel.
    bool has_alpha() const { return m_pixel_format == PixelFormat::ya || m_pixel_format == PixelFormat::rgba; }

    /// The number of bytes per pixel.
    size_t bytes_per_pixel() const;

    /// The total size of the bitmap in bytes.
    size_t buffer_size() const { return pixel_count() * bytes_per_pixel(); }

    /// The raw image data.
    void* data() { return m_data.get(); }
    const void* data() const { return m_data.get(); }

    /// The raw image data as uint8_t.
    uint8_t* uint8_data() { return m_data.get(); }
    const uint8_t* uint8_data() const { return m_data.get(); }

    /// True if bitmap is empty.
    bool empty() const { return m_width == 0 || m_height == 0; }

    /// Clears the bitmap to zeros.
    void clear();

    /// Vertically flip the bitmap.
    void vflip();

    ref<Struct> pixel_struct() const;

    ref<Bitmap> convert(PixelFormat pixel_format, ComponentType component_type, bool srgb_gamma) const;

    void convert(Bitmap* target) const;

    /// Equality operator.
    bool operator==(const Bitmap& other) const;

    /// Inequality operator.
    bool operator!=(const Bitmap& other) const { return !operator==(other); }

    std::string to_string() const override;

    static void static_init();
    static void static_shutdown();

private:
    void read(Stream* stream, FileFormat format);

    static FileFormat detect_file_format(Stream* stream);

    void check_required_format(
        std::string_view file_format,
        std::vector<Bitmap::PixelFormat> allowed_pixel_formats,
        std::vector<Bitmap::ComponentType> allowed_component_types
    ) const;

    void read_png(Stream* stream);
    void write_png(Stream* stream, int compression) const;

    void read_jpg(Stream* stream);
    void write_jpg(Stream* stream, int quality) const;

    void read_bmp(Stream* stream);
    void write_bmp(Stream* stream) const;

    void read_tga(Stream* stream);
    void write_tga(Stream* stream) const;

    void read_hdr(Stream* stream);
    void write_hdr(Stream* stream) const;

    void read_exr(Stream* stream);
    void write_exr(Stream* stream, int quality) const;

    PixelFormat m_pixel_format;
    ComponentType m_component_type;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_channel_count;
    std::vector<std::string> m_channel_names;
    bool m_srgb_gamma;
    std::unique_ptr<uint8_t[]> m_data;
    bool m_owns_data;
};

KALI_ENUM_REGISTER(Bitmap::FileFormat);
KALI_ENUM_REGISTER(Bitmap::PixelFormat);

} // namespace kali
