#include "testing.h"
#include "kali/core/memory_mapped_file.h"

#include <cstring>
#include <fstream>
#include <random>
#include <vector>

using namespace kali;

TEST_SUITE_BEGIN("memory_mapped_file");

TEST_CASE("MemoryMappedFile")
{
    SUBCASE("closed")
    {
        MemoryMappedFile file;
        CHECK_EQ(file.is_open(), false);
        CHECK_EQ(file.size(), 0);
        CHECK_EQ(file.data(), nullptr);
        CHECK_EQ(file.mapped_size(), 0);

        // Allowed to close a closed file.
        file.close();
        CHECK_EQ(file.is_open(), false);
    }

    SUBCASE("non existing")
    {
        {
            MemoryMappedFile file;
            CHECK_EQ(file.open("__file_that_does_not_exist__"), false);
            CHECK_EQ(file.is_open(), false);
        }
        {
            MemoryMappedFile file("__file_that_does_not_exist__");
            CHECK_EQ(file.is_open(), false);
        }
    }

    SUBCASE("read")
    {
        std::vector<uint8_t> random_data(128 * 1024);
        std::mt19937 rng;
        for (size_t i = 0; i < random_data.size(); ++i)
            random_data[i] = rng() & 0xff;

        const std::filesystem::path temp_path = std::filesystem::absolute("test_memory_mapped.bin");

        // Write file with random data.
        std::ofstream ofs(temp_path, std::ios::binary);
        REQUIRE(ofs.good());
        ofs.write(reinterpret_cast<const char*>(random_data.data()), random_data.size());
        ofs.close();

        {
            // Map entire file.
            MemoryMappedFile file(temp_path);
            CHECK_EQ(file.is_open(), true);
            CHECK_EQ(file.size(), random_data.size());
            CHECK_NE(file.data(), nullptr);
            CHECK_GE(file.mapped_size(), random_data.size());
            CHECK(std::memcmp(file.data(), random_data.data(), file.size()) == 0);
        }

        {
            // Map first 1024 bytes.
            MemoryMappedFile file(temp_path, 1024);
            CHECK_EQ(file.is_open(), true);
            CHECK_EQ(file.size(), random_data.size());
            CHECK_NE(file.data(), nullptr);
            CHECK_GE(file.mapped_size(), 1024);
            CHECK(std::memcmp(file.data(), random_data.data(), 1024) == 0);
        }

        {
            // Map first page.
            size_t page_size = MemoryMappedFile::page_size();
            CHECK_GE(page_size, 4096);
            REQUIRE_LE(page_size, random_data.size());
            MemoryMappedFile file(temp_path, page_size);
            CHECK_EQ(file.is_open(), true);
            CHECK_EQ(file.size(), random_data.size());
            CHECK_NE(file.data(), nullptr);
            CHECK_GE(file.mapped_size(), page_size);
            CHECK(std::memcmp(file.data(), random_data.data(), page_size) == 0);
        }

        // Cleanup.
        std::filesystem::remove(temp_path);
    }
}

TEST_SUITE_END();
