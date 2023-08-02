#include "testing.h"
#include "kali/core/memory_stream.h"
#include "kali/core/file_stream.h"
#include "kali/core/memory_mapped_file_stream.h"

#include <cstring>
#include <fstream>
#include <random>
#include <vector>

using namespace kali;

TEST_SUITE_BEGIN("stream");

TEST_CASE("MemoryStream")
{
    SUBCASE("unowned_read_only")
    {
        const uint8_t buffer[8]{0, 1, 2, 3, 4, 5, 6, 7};
        MemoryStream stream(buffer, 8);
        CHECK(stream.is_open());
        CHECK(stream.is_readable());
        CHECK_FALSE(stream.is_writable());
        CHECK_EQ(stream.tell(), 0);
        CHECK_EQ(stream.size(), 8);
        CHECK_EQ(stream.capacity(), 8);
        CHECK_FALSE(stream.owns_data());
        CHECK_EQ(stream.data(), buffer);

        uint8_t tmp[8];
        stream.read(tmp, 8);
        CHECK(std::memcmp(tmp, buffer, 8) == 0);
        CHECK_EQ(stream.tell(), 8);

        stream.seek(0);
        CHECK_EQ(stream.tell(), 0);

        CHECK_THROWS(stream.write(tmp, 8));

        stream.seek(4);
        CHECK_EQ(stream.tell(), 4);

        stream.read(tmp, 4);
        CHECK(std::memcmp(tmp, buffer + 4, 4) == 0);

        CHECK_THROWS(stream.truncate(4));
        CHECK_THROWS(stream.truncate(16));

        stream.close();
        CHECK_FALSE(stream.is_open());
    }

    SUBCASE("unowned_read_write")
    {
        uint8_t buffer[8];
        std::memcpy(buffer, "12345678", 8);
        MemoryStream stream(buffer, 8);
        CHECK(stream.is_open());
        CHECK(stream.is_readable());
        CHECK(stream.is_writable());
        CHECK_EQ(stream.tell(), 0);
        CHECK_EQ(stream.size(), 8);
        CHECK_EQ(stream.capacity(), 8);
        CHECK_FALSE(stream.owns_data());
        CHECK_EQ(stream.data(), (void*)buffer);

        uint8_t tmp[8];
        stream.read(tmp, 8);
        CHECK(std::memcmp(tmp, buffer, 8) == 0);
        CHECK_EQ(stream.tell(), 8);

        stream.seek(0);
        CHECK_EQ(stream.tell(), 0);

        std::memcpy(tmp, "abcdefgh", 8);
        stream.write(tmp, 8);
        CHECK(std::memcmp(tmp, buffer, 8) == 0);
        CHECK_EQ(stream.tell(), 8);

        stream.seek(4);
        CHECK_EQ(stream.tell(), 4);

        stream.read(tmp, 4);
        CHECK(std::memcmp(tmp, buffer + 4, 4) == 0);

        CHECK_THROWS(stream.truncate(4));
        CHECK_THROWS(stream.truncate(16));

        stream.close();
        CHECK_FALSE(stream.is_open());
    }

    SUBCASE("owned")
    {
        MemoryStream stream(8);
        CHECK(stream.is_open());
        CHECK(stream.is_readable());
        CHECK(stream.is_writable());
        CHECK_EQ(stream.tell(), 0);
        CHECK_EQ(stream.size(), 0);
        CHECK_EQ(stream.capacity(), 8);
        CHECK(stream.owns_data());
        CHECK_NE(stream.data(), nullptr);

        uint8_t tmp[8];
        CHECK_THROWS(stream.read(tmp, 1));

        stream.seek(8);
        CHECK_EQ(stream.tell(), 8);

        std::memcpy(tmp, "abcdefgh", 8);
        stream.write(tmp, 8);
        CHECK_EQ(stream.tell(), 16);
        CHECK_EQ(stream.size(), 16);
        CHECK_GE(stream.capacity(), 16);

        stream.seek(0);
        CHECK_EQ(stream.tell(), 0);
        stream.read(tmp, 8);
        for (size_t i = 0; i < 8; ++i)
            CHECK_EQ(tmp[i], 0);
        CHECK_EQ(stream.tell(), 8);

        stream.seek(0);
        CHECK_EQ(stream.tell(), 0);

        std::memcpy(tmp, "01234567", 8);
        stream.write(tmp, 8);
        CHECK_EQ(stream.tell(), 8);
        CHECK_EQ(stream.size(), 16);
        CHECK_GE(stream.capacity(), 16);

        stream.seek(4);
        CHECK_EQ(stream.tell(), 4);
        stream.read(tmp, 4);
        for (size_t i = 0; i < 4; ++i)
            CHECK_EQ(tmp[i], '4' + i);

        stream.truncate(4);
        CHECK_EQ(stream.tell(), 4);
        CHECK_EQ(stream.size(), 4);

        stream.close();
        CHECK_FALSE(stream.is_open());
    }
}

TEST_CASE("FileStream")
{
    char buffer[16];

    SUBCASE("open non-exist file")
    {
        CHECK_THROWS(FileStream("__file_that_does_not_exist__", FileStream::Mode::Read));
    }

    SUBCASE("write")
    {
        FileStream stream("file_stream_write.bin", FileStream::Mode::Write);
        CHECK_EQ(stream.path(), "file_stream_write.bin");
        CHECK_EQ(stream.mode(), FileStream::Mode::Write);
        CHECK(stream.is_open());
        CHECK_FALSE(stream.is_readable());
        CHECK(stream.is_writable());
        CHECK_EQ(stream.tell(), 0);
        CHECK_EQ(stream.size(), 0);

        CHECK_THROWS(stream.read(buffer, 1));

        stream.write("12345678", 8);
        CHECK_EQ(stream.tell(), 8);
        stream.flush();
        CHECK_EQ(stream.size(), 8);

        stream.seek(4);
        CHECK_EQ(stream.tell(), 4);
        stream.write("1234", 4);
        CHECK_EQ(stream.tell(), 8);
        stream.flush();
        CHECK_EQ(stream.size(), 8);

        stream.write("abcdefgh", 8);
        CHECK_EQ(stream.tell(), 16);
        stream.flush();
        CHECK_EQ(stream.size(), 16);

        stream.truncate(12);
        CHECK_EQ(stream.tell(), 12);
        stream.flush();
        CHECK_EQ(stream.size(), 12);

        stream.close();
        CHECK_FALSE(stream.is_open());

        std::ifstream file("file_stream_write.bin", std::ios::binary);
        CHECK(file.is_open());
        file.read(buffer, 12);
        CHECK(std::memcmp(buffer, "12341234abcd", 12) == 0);
        file.close();
    }

    SUBCASE("read")
    {
        std::ofstream file("file_stream_read.bin", std::ios::binary);
        file.write("12345678abcdefgh", 16);
        file.close();

        FileStream stream("file_stream_read.bin", FileStream::Mode::Read);
        CHECK_EQ(stream.path(), "file_stream_read.bin");
        CHECK_EQ(stream.mode(), FileStream::Mode::Read);
        CHECK(stream.is_open());
        CHECK(stream.is_readable());
        CHECK_FALSE(stream.is_writable());
        CHECK_EQ(stream.tell(), 0);
        CHECK_EQ(stream.size(), 16);

        CHECK_THROWS(stream.write("12345678", 8));
        CHECK_THROWS(stream.truncate(1));

        char buffer[16];
        stream.read(buffer, 8);
        CHECK(std::memcmp(buffer, "12345678", 8) == 0);
        CHECK_EQ(stream.tell(), 8);

        stream.seek(0);
        CHECK_EQ(stream.tell(), 0);

        stream.read(buffer, 8);
        CHECK(std::memcmp(buffer, "12345678", 8) == 0);
        CHECK_EQ(stream.tell(), 8);

        stream.seek(4);
        CHECK_EQ(stream.tell(), 4);

        CHECK_THROWS_AS(stream.read(buffer, 16), EOFException);
        CHECK(std::memcmp(buffer, "5678abcdefgh", 12) == 0);

        CHECK_EQ(stream.tell(), 16);

        stream.close();
        CHECK_FALSE(stream.is_open());
    }
}

TEST_CASE("MemoryMappedFileStream")
{
    char buffer[16];

    SUBCASE("read")
    {
        std::ofstream file("memory_mapped_file_stream_read.bin", std::ios::binary);
        file.write("12345678abcdefgh", 16);
        file.close();

        MemoryMappedFileStream stream("memory_mapped_file_stream_read.bin");
        CHECK_EQ(stream.path(), "memory_mapped_file_stream_read.bin");
        CHECK(stream.is_open());
        CHECK(stream.is_readable());
        CHECK_FALSE(stream.is_writable());
        CHECK_EQ(stream.tell(), 0);
        CHECK_EQ(stream.size(), 16);

        CHECK_THROWS(stream.write("12345678", 8));
        CHECK_THROWS(stream.truncate(1));

        char buffer[16];
        stream.read(buffer, 8);
        CHECK(std::memcmp(buffer, "12345678", 8) == 0);
        CHECK_EQ(stream.tell(), 8);

        stream.seek(0);
        CHECK_EQ(stream.tell(), 0);

        stream.read(buffer, 8);
        CHECK(std::memcmp(buffer, "12345678", 8) == 0);
        CHECK_EQ(stream.tell(), 8);

        stream.seek(4);
        CHECK_EQ(stream.tell(), 4);

        stream.read(buffer, 12);
        CHECK(std::memcmp(buffer, "5678abcdefgh", 12) == 0);

        CHECK_EQ(stream.tell(), 16);

        CHECK_THROWS(stream.read(buffer, 1));

        stream.close();
        CHECK_FALSE(stream.is_open());
    }
}

TEST_SUITE_END();
