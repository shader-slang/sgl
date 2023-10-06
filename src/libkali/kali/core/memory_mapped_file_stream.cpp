#include "kali/core/memory_mapped_file_stream.h"

#include "kali/core/error.h"

namespace kali {

MemoryMappedFileStream::MemoryMappedFileStream(
    const std::filesystem::path& path,
    size_t mapped_size,
    MemoryMappedFile::AccessHint access_hint
)
    : MemoryStream(static_cast<const void*>(nullptr), 0)
    , m_path(path)
{
    m_file = std::make_unique<MemoryMappedFile>(path, mapped_size, access_hint);
    if (!m_file->is_open())
        KALI_THROW("{}: I/O error while attempting to open file", m_path);

    m_data = (uint8_t*)m_file->data();
    m_size = m_file->size();
}

MemoryMappedFileStream::~MemoryMappedFileStream()
{
    close();
}

std::string MemoryMappedFileStream::to_string() const
{
    return fmt::format("MemoryMappedFileStream(path={})", m_path);
}

} // namespace kali
