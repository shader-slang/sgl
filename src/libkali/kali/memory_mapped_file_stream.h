#pragma once

#include "kali/memory_stream.h"
#include "kali/memory_mapped_file.h"

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

public:
    std::filesystem::path m_path;
    std::unique_ptr<MemoryMappedFile> m_file;
};

} // namespace kali
