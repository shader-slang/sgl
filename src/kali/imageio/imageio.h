#pragma once

#include "core/platform.h"
#include "core/object.h"

#include <cstdint>
#include <filesystem>
#include <memory>

namespace kali {

class MemoryMappedFile;
class ImageReader;
class ImageWriter;

/// The type of each component in an image.
enum class ComponentType {
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
    ComponentType component_type{ComponentType::unknown};

    size_t get_component_size() const
    {
        switch (component_type) {
        case ComponentType::unknown:
            return 0;
        case ComponentType::u8:
            return 1;
        case ComponentType::u16:
            return 2;
        case ComponentType::u32:
            return 4;
        case ComponentType::f16:
            return 2;
        case ComponentType::f32:
            return 4;
        }
        return 0;
    }
};

/// Class for reading images.
class KALI_API ImageInput : public Object {
public:
    static ref<ImageInput> open(const std::filesystem::path& path);

    ~ImageInput();

    const ImageSpec& get_spec() const { return m_spec; }

    bool read_image(void* buffer, size_t len);

private:
    ImageSpec m_spec;
    std::unique_ptr<ImageReader> m_reader;
    std::unique_ptr<MemoryMappedFile> m_file;
};

/// Class for writing images.
class KALI_API ImageOutput : public Object {
public:
    static ref<ImageOutput> open(const std::filesystem::path& path, ImageSpec spec);

    ~ImageOutput();

    bool write_image(const void* buffer, size_t len);

private:
    ImageSpec m_spec;
    std::unique_ptr<ImageWriter> m_writer;
};

} // namespace kali
