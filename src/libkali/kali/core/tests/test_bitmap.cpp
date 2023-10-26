#include "testing.h"
#include "kali/core/bitmap.h"

#include <cstdint>
#include <thread>
#include <chrono>

using namespace kali;

TEST_SUITE_BEGIN("bitmap");

TEST_CASE("constructor")
{
    // Check pixel formats.
    for (auto [pixel_format, channel_count] :
         {std::pair{Bitmap::PixelFormat::y, 1},
          std::pair{Bitmap::PixelFormat::ya, 2},
          std::pair{Bitmap::PixelFormat::rgb, 3},
          std::pair{Bitmap::PixelFormat::rgba, 4}}) {
        Bitmap b(pixel_format, Bitmap::ComponentType::f32, 1, 1);
        CHECK_EQ(b.pixel_format(), pixel_format);
        CHECK_EQ(b.channel_count(), channel_count);
        CHECK_EQ(b.bytes_per_pixel(), channel_count * 4);
    };

    // Check component formats.
    for (auto [component_type, bytes_per_pixel] :
         {std::pair{Bitmap::ComponentType::u8, 1},
          std::pair{Bitmap::ComponentType::u16, 2},
          std::pair{Bitmap::ComponentType::u32, 4},
          std::pair{Bitmap::ComponentType::f16, 2},
          std::pair{Bitmap::ComponentType::f32, 4}}) {
        Bitmap b(Bitmap::PixelFormat::y, component_type, 1, 1);
        CHECK_EQ(b.component_type(), component_type);
        CHECK_EQ(b.bytes_per_pixel(), bytes_per_pixel);
    }

    CHECK_THROWS(Bitmap(Bitmap::PixelFormat::multi_channel, Bitmap::ComponentType::f32, 1, 1));
}

TEST_CASE("create")
{
    {
        Bitmap b(Bitmap::PixelFormat::y, Bitmap::ComponentType::u8, 16, 16);
        CHECK_EQ(b.pixel_format(), Bitmap::PixelFormat::y);
        CHECK_EQ(b.component_type(), Bitmap::ComponentType::u8);
        CHECK_EQ(b.width(), 16);
        CHECK_EQ(b.height(), 16);
        CHECK_EQ(b.pixel_count(), 16 * 16);
        CHECK_EQ(b.channel_count(), 1);
        CHECK_FALSE(b.has_alpha());
        CHECK_EQ(b.bytes_per_pixel(), 1);
        CHECK_EQ(b.buffer_size(), 16 * 16);
    }

    {
        Bitmap b(Bitmap::PixelFormat::y, Bitmap::ComponentType::u8, 16, 16);
        CHECK_EQ(b.pixel_format(), Bitmap::PixelFormat::y);
        CHECK_EQ(b.component_type(), Bitmap::ComponentType::u8);
        CHECK_EQ(b.width(), 16);
        CHECK_EQ(b.height(), 16);
        CHECK_EQ(b.pixel_count(), 16 * 16);
        CHECK_EQ(b.channel_count(), 1);
        CHECK_FALSE(b.has_alpha());
        CHECK_EQ(b.bytes_per_pixel(), 1);
        CHECK_EQ(b.buffer_size(), 16 * 16);
    }
}

TEST_SUITE_END();
