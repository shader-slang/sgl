#pragma once

#include "kali/core/memory_stream.h"
#include "kali/core/memory_mapped_file.h"

#include <filesystem>
#include <memory>

namespace kali {

class KALI_API MemoryMappedFileStream : public MemoryStream {
    KALI_OBJECT(MemoryMappedFileStream)
public:
    MemoryMappedFileStream(
        const std::filesystem::path& path,
        size_t mapped_size = MemoryMappedFile::WHOLE_FILE,
        MemoryMappedFile::AccessHint access_hint = MemoryMappedFile::AccessHint::normal
    );
    virtual ~MemoryMappedFileStream();

    const std::filesystem::path& path() const { return m_path; }

    std::string to_string() const override;

public:
    std::filesystem::path m_path;
    std::unique_ptr<MemoryMappedFile> m_file;
};

} // namespace kali
