// TODO: MITSUBA license

#pragma once

#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/enum.h"
#include "kali/core/stream.h"

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
        png,
        jpg,
        bmp,
        tga,
        hdr,
        exr,
        auto_,
    };

    KALI_ENUM_INFO(
        FileFormat,
        {
            {FileFormat::png, "png"},
            {FileFormat::jpg, "jpg"},
            {FileFormat::bmp, "bmp"},
            {FileFormat::tga, "tga"},
            {FileFormat::hdr, "hdr"},
            {FileFormat::exr, "exr"},
            {FileFormat::auto_, "auto"},
        }
    );

    enum class PixelFormat {
        y,
        ya,
        rgb,
        rgba,
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

    enum class ComponentType {
        u8,
        u16,
        u32,
        f16,
        f32,
    };

    KALI_ENUM_INFO(
        ComponentType,
        {
            {ComponentType::u8, "u8"},
            {ComponentType::u16, "u16"},
            {ComponentType::u32, "u32"},
            {ComponentType::f16, "f16"},
            {ComponentType::f32, "f32"},
        }
    );

    Bitmap(
        PixelFormat pixel_format,
        ComponentType component_type,
        uint32_t width,
        uint32_t height,
        uint32_t channel_count = 0,
        const std::vector<std::string>& channel_names = {},
        const void* data = nullptr
    );

    Bitmap(Stream* stream, FileFormat format = FileFormat::auto_);

    Bitmap(const std::filesystem::path& path, FileFormat format = FileFormat::auto_);

    Bitmap(const Bitmap& other);

    Bitmap(Bitmap&& other);

    ~Bitmap();

    static ref<Bitmap> read(Stream* stream, FileFormat format = FileFormat::auto_)
    {
        return make_ref<Bitmap>(stream, format);
    }

    static ref<Bitmap> read(const std::filesystem::path& path, FileFormat format = FileFormat::auto_)
    {
        return make_ref<Bitmap>(path, format);
    }

    static std::future<ref<Bitmap>> read_async(ref<Stream> stream, FileFormat format = FileFormat::auto_);
    static std::future<ref<Bitmap>>
    read_async(const std::filesystem::path& path, FileFormat format = FileFormat::auto_);

    void write(Stream* stream, FileFormat format = FileFormat::auto_, int quality = -1) const;
    void write(const std::filesystem::path& path, FileFormat format = FileFormat::auto_, int quality = -1) const;

    std::future<void> write_async(ref<Stream> stream, FileFormat format = FileFormat::auto_, int quality = -1) const;
    std::future<void>
    write_async(const std::filesystem::path& path, FileFormat format = FileFormat::auto_, int quality = -1) const;

    PixelFormat pixel_format() const { return m_pixel_format; }
    ComponentType component_type() const { return m_component_type; }

    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }
    size_t pixel_count() const { return static_cast<size_t>(m_width) * m_height; }
    uint32_t channel_count() const { return m_channel_count; }
    bool has_alpha() const { return m_pixel_format == PixelFormat::ya || m_pixel_format == PixelFormat::rgba; }

    size_t bytes_per_pixel() const;
    size_t buffer_size() const { return pixel_count() * bytes_per_pixel(); }

    void* data() { return m_data.get(); }
    const void* data() const { return m_data.get(); }

    uint8_t* uint8_data() { return m_data.get(); }
    const uint8_t* uint8_data() const { return m_data.get(); }

    void clear();
    void vflip();

    bool operator==(const Bitmap& bitmap) const;

    bool operator!=(const Bitmap& bitmap) const { return !operator==(bitmap); }

private:
    void read_png(Stream* stream);
    void write_png(Stream* stream, int compression) const;

    void read_jpg(Stream* stream);
    void write_jpg(Stream* stream, int quality) const;

    PixelFormat m_pixel_format;
    ComponentType m_component_type;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_channel_count;
    std::vector<std::string> m_channel_names;
    std::unique_ptr<uint8_t[]> m_data;
};

KALI_ENUM_REGISTER(Bitmap::FileFormat);
KALI_ENUM_REGISTER(Bitmap::PixelFormat);
KALI_ENUM_REGISTER(Bitmap::ComponentType);

} // namespace kali
