#include "testing.h"
#include "imageio/imageio.h"

#include <memory>

using namespace kali;

struct rgb8 {
    uint8_t r, g, b;
};

template<typename T>
struct TestImage {
    uint32_t w;
    uint32_t h;
    std::unique_ptr<T[]> pixels;

    T* data() { return pixels.get(); }
    const T* data() const { return pixels.get(); }
    size_t size() const { return w * h * sizeof(T); }
};

template<typename T>
TestImage<T> create_image(uint32_t w, uint32_t h)
{
    return {w, h, std::make_unique<T[]>(w * h)};
}

template<typename T>
TestImage<T> create_checkerboard(uint32_t w, uint32_t h, T color0, T color1, uint32_t cell_size = 16)
{
    TestImage<T> img{w, h, std::make_unique<T[]>(w * h)};

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            bool on = ((x / cell_size) ^ (y / cell_size)) & 1;
            img.pixels[y * w + x] = on ? color1 : color0;
        }
    }

    return img;
}

inline double pixel_diff(rgb8 p0, rgb8 p1)
{
    double r = std::max(p0.r, p1.r) - std::min(p0.r, p1.r);
    double g = std::max(p0.g, p1.g) - std::min(p0.g, p1.g);
    double b = std::max(p0.b, p1.b) - std::min(p0.b, p1.b);
    return std::max(r, std::max(g, b));
}

template<typename T>
double image_diff(const TestImage<T>& img0, const TestImage<T>& img1)
{
    double diff = 0.0;
    REQUIRE_EQ(img0.w, img1.w);
    REQUIRE_EQ(img0.h, img1.h);
    REQUIRE(img0.pixels);
    REQUIRE(img1.pixels);
    for (uint32_t i = 0; i < img0.w * img0.h; ++i)
        diff = std::max(diff, pixel_diff(img0.pixels.get()[i], img1.pixels.get()[i]));
    return diff;
}

TEST_SUITE_BEGIN("imageio");

TEST_CASE("jpeg")
{
    auto img = create_checkerboard(128, 64, rgb8{25, 75, 125}, rgb8{125, 175, 225});

    {
        auto output = ImageOutput::open(
            "test.jpg",
            {.width = img.w, .height = img.h, .component_count = 3, .component_type = ComponentType::u8}
        );
        REQUIRE(output);
        CHECK(output->write_image(img.data(), img.size()));
    }

    {
        auto input = ImageInput::open("test.jpg");
        REQUIRE(input);

        const ImageSpec& spec = input->get_spec();
        CHECK_EQ(spec.width, 128);
        CHECK_EQ(spec.height, 64);
        CHECK_EQ(spec.component_count, 3);
        CHECK_EQ(spec.component_type, ComponentType::u8);

        auto img2 = create_image<rgb8>(spec.width, spec.height);
        CHECK(input->read_image(img2.data(), img.size()));

        double diff = image_diff(img, img2);
        CHECK_LE(diff, 1.0);
    }
}

TEST_SUITE_END();
