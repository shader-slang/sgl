#include "kali/core/file_stream.h"

#include "kali/core/error.h"

#include <fstream>

namespace kali {

inline std::ios::openmode get_openmode(FileStream::Mode mode)
{
    switch (mode) {
    case FileStream::Mode::read:
        return std::ios::in | std::ios::binary;
    case FileStream::Mode::write:
        return std::ios::out | std::ios::binary;
    case FileStream::Mode::read_write:
        return std::ios::in | std::ios::out | std::ios::binary;
    default:
        KALI_UNREACHABLE();
    }
}

inline std::string strerror_safe(int errnum)
{
    char buf[1024];
#if KALI_WINDOWS
    strerror_s(buf, sizeof(buf), errnum);
    return buf;
#else
    strerror_r(errnum, buf, sizeof(buf));
    return buf;
#endif
}

FileStream::FileStream(const std::filesystem::path& path, Mode mode)
    : m_path(path)
    , m_mode(mode)
{
    m_stream = std::make_unique<std::fstream>(m_path, get_openmode(m_mode));

    if (!m_stream->good())
        KALI_THROW("{}: I/O error while attempting to open file: {}", m_path, strerror_safe(errno));
}

FileStream::~FileStream()
{
    close();
}

bool FileStream::is_open() const
{
    return m_stream->is_open();
}

void FileStream::close()
{
    m_stream->close();
}

void FileStream::read(void* p, size_t size)
{
    m_stream->read((char*)p, size);

    if (!m_stream->good()) {
        bool eof = m_stream->eof();
        size_t gcount = m_stream->gcount();
        m_stream->clear();
        if (eof)
            throw EOFException(fmt::format("{}: read {} out of {} bytes", m_path, gcount, size), gcount);
        else
            KALI_THROW("{}: I/O error while attempting to read {} bytes: {}", m_path, size, strerror_safe(errno));
    }
}

void FileStream::write(const void* p, size_t size)
{
    m_stream->write((char*)p, size);

    if (!m_stream->good()) {
        m_stream->clear();
        KALI_THROW("{}: I/O error while attempting to write {} bytes: {}", m_path, size, strerror_safe(errno));
    }
}

void FileStream::seek(size_t pos)
{
    m_stream->seekg(static_cast<std::ios::pos_type>(pos));
    if (!m_stream->good())
        KALI_THROW("{}: I/O error while attempting to seek in file", m_path);
}

void FileStream::truncate(size_t size)
{
    if (m_mode == Mode::read)
        KALI_THROW("{}: attempting to truncate a read-only file", m_path);

    flush();
    const size_t prev_pos = tell();
#if KALI_WINDOWS
    // Need to close the file in order to resize it.
    close();
#else
    seek(0);
#endif

    std::filesystem::resize_file(m_path, size);

#if KALI_WINDOWS
    m_stream->open(m_path, get_openmode(Mode::read_write));
    if (!m_stream->good())
        KALI_THROW("{}: I/O error while attempting to open file: {}", m_path, strerror_safe(errno));
#endif

    seek(std::min(prev_pos, size));
}

size_t FileStream::tell() const
{
    std::ios::pos_type pos = m_stream->tellg();
    if (pos == std::ios::pos_type(-1))
        KALI_THROW("{}: I/O error while attempting to determine position in file", m_path);
    return static_cast<size_t>(pos);
}

size_t FileStream::size() const
{
    return std::filesystem::file_size(m_path);
}

void FileStream::flush()
{
    m_stream->flush();
    if (!m_stream->good()) {
        m_stream->clear();
        KALI_THROW("{}: I/O error while attempting flush file stream: {}", m_path, strerror_safe(errno));
    }
}

std::string FileStream::to_string() const
{
    return fmt::format(
        "FileStream(\n"
        "  path = \"{}\",\n"
        "  mode = {}\n"
        ")",
        m_path,
        m_mode
    );
}

} // namespace kali
