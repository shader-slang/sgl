#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

namespace kali {

class MemoryMappedFile;
class ImageReader;
class ImageWriter;

enum class ComponentType {
    Unknown,
    U8,
};

struct ImageSpec {
    uint32_t width{0};
    uint32_t height{0};
    ComponentType component_type{ComponentType::Unknown};
    uint32_t component_count{0};
};

class ImageInput {
public:
    static std::unique_ptr<ImageInput> open(const std::filesystem::path& path);

    ~ImageInput();

    const ImageSpec& spec() const { return m_spec; }

    bool read_image(void* buffer, size_t len);

private:
    ImageSpec m_spec;
    std::unique_ptr<ImageReader> m_reader;
    std::unique_ptr<MemoryMappedFile> m_file;
};

class ImageOutput {
public:
    static std::unique_ptr<ImageOutput> open(const std::filesystem::path& path, ImageSpec spec);

    ~ImageOutput();

    bool write_image(const void* buffer, size_t len);

private:
    ImageSpec m_spec;
    std::unique_ptr<ImageWriter> m_writer;
};

} // namespace kali
