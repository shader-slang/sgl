#pragma once

#include "kali/core/stream.h"

#include <exception>
#include <filesystem>
#include <iosfwd>
#include <memory>

namespace kali {

class KALI_API EOFException : public std::runtime_error {
public:
    EOFException(const std::string& what, size_t gcount)
        : std::runtime_error(what)
        , m_gcount(gcount)
    {
    }

    size_t gcount() const { return m_gcount; }

private:
    size_t m_gcount;
};

class KALI_API FileStream : public Stream {
    KALI_OBJECT(FileStream)
public:
    enum class Mode {
        Read,
        Write,
        ReadWrite,
    };

    FileStream(const std::filesystem::path& path, Mode mode);
    virtual ~FileStream();

    const std::filesystem::path& path() const { return m_path; }
    Mode mode() const { return m_mode; }

    bool is_open() const override;
    bool is_readable() const override { return m_mode == Mode::Read || m_mode == Mode::ReadWrite; }
    bool is_writable() const override { return m_mode == Mode::Write || m_mode == Mode::ReadWrite; }

    void close() override;

    void read(void* p, size_t size) override;
    void write(const void* p, size_t size) override;
    void seek(size_t pos) override;
    void truncate(size_t size) override;
    size_t tell() const override;
    size_t size() const override;
    void flush() override;

private:
    std::filesystem::path m_path;
    Mode m_mode;
    std::unique_ptr<std::fstream> m_stream;
};

} // namespace kali
