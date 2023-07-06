#pragma once

#include "kali/macros.h"
#include "kali/object.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>

namespace kali {

class MemoryMappedFile;
class ImageReader;
class ImageWriter;

#ifdef KALI_HEADER_VALIDATION
class MemoryMappedFile {};
class ImageReader {};
class ImageWriter {};
#endif

/// The type of each component in an image.
enum class ImageComponentType {
    unknown,
    u8,
    u16,
    u32,
    f16,
    f32,
};

struct ImageSpec {
    /// The width of the image in pixels.
    uint32_t width{0};
    /// The height of the image in pixels.
    uint32_t height{0};
    /// The number of components per pixel.
    uint32_t component_count{0};
    /// The type of each component.
    ImageComponentType component_type{ImageComponentType::unknown};

    size_t get_component_byte_size() const
    {
        switch (component_type) {
        case ImageComponentType::unknown:
            return 0;
        case ImageComponentType::u8:
            return 1;
        case ImageComponentType::u16:
        case ImageComponentType::f16:
            return 2;
        case ImageComponentType::u32:
        case ImageComponentType::f32:
            return 4;
        }
        return 0;
    }

    size_t get_image_byte_size() const { return width * height * component_count * get_component_byte_size(); }
};

/// Class for reading images.
class KALI_API ImageInput : public Object {
    KALI_OBJECT(ImageInput)
public:
    using Options = std::map<std::string, std::string>;

    static ref<ImageInput> open(const std::filesystem::path& path, Options options = {});

    ~ImageInput();

    const ImageSpec& get_spec() const { return m_spec; }

    bool read_image(void* buffer, size_t len);

    const std::string& get_error() const { return m_error; }

private:
    ImageSpec m_spec;
    Options m_options;
    std::string m_error;
    std::unique_ptr<ImageReader> m_reader;
    std::unique_ptr<MemoryMappedFile> m_file;
};

/// Class for writing images.
class KALI_API ImageOutput : public Object {
    KALI_OBJECT(ImageOutput)
public:
    using Options = std::map<std::string, std::string>;

    static ref<ImageOutput> open(const std::filesystem::path& path, ImageSpec spec, Options options = {});

    ~ImageOutput();

    bool write_image(const void* buffer, size_t len);

    const std::string& get_error() const { return m_error; }

private:
    ImageSpec m_spec;
    Options m_options;
    std::string m_error;
    std::unique_ptr<ImageWriter> m_writer;
};

} // namespace kali
