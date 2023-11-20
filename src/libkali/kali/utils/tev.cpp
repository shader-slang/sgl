#include "tev.h"

#include "kali/core/bitmap.h"
#include "kali/core/format.h"

#include <tevclient.h>

#include <atomic>

namespace kali::utils {

bool show_in_tev(const Bitmap* bitmap, std::optional<std::string> name, const std::string& host, uint16_t port)
{
    KALI_CHECK(bitmap, "Bitmap cannot be null.");

    ref<Bitmap> converted;
    if (bitmap->component_type() != Bitmap::ComponentType::float32) {
        log_warn(
            "tev only supports 32-bit floating point images. Converting {} data to float.",
            bitmap->component_type()
        );
        converted = bitmap->convert(bitmap->pixel_format(), Bitmap::ComponentType::float32, false);
        bitmap = converted;
    }

    static std::atomic<uint32_t> image_counter{0};
    if (!name)
        name = fmt::format("image_{}", image_counter++);

    tevclient::Client client(host.c_str(), port);
    if (client.connect() != tevclient::Error::Ok) {
        log_warn("Failed to connect to tev viewer: {}", client.lastErrorString());
        return false;
    }

    std::vector<const char*> channel_names(bitmap->channel_names().size());
    for (size_t i = 0; i < channel_names.size(); ++i)
        channel_names[i] = bitmap->channel_names()[i].c_str();

    client.createImage(name->c_str(), bitmap->width(), bitmap->height(), bitmap->channel_count(), channel_names.data());
    client.updateImage(
        name->c_str(),
        0,
        0,
        bitmap->width(),
        bitmap->height(),
        bitmap->channel_count(),
        channel_names.data(),
        nullptr,
        nullptr,
        (const float*)bitmap->data(),
        bitmap->buffer_size() / 4
    );

    return true;
}

} // namespace kali::utils
