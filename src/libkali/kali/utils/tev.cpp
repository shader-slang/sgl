#include "tev.h"

#include "kali/core/config.h"
#include "kali/core/bitmap.h"
#include "kali/core/format.h"
#include "kali/core/thread.h"

#include "kali/device/resource.h"

#if KALI_HAS_TEVCLIENT
#include <tevclient.h>
#endif

#include <atomic>
#include <semaphore>

namespace kali::utils {

#if KALI_HAS_TEVCLIENT

class ClientPool {
public:
    ClientPool() { tevclient::initialize(); }

    ~ClientPool()
    {
        m_clients.clear();
        tevclient::shutdown();
    }

    std::unique_ptr<tevclient::Client> acquire_client(std::string host, uint16_t port)
    {
        std::unique_ptr<tevclient::Client> client;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = std::find_if(
                m_clients.begin(),
                m_clients.end(),
                [&](const auto& c) { return c->getHostname() == host && c->getPort() == port; }
            );
            if (it != m_clients.end()) {
                client = std::move(*it);
                m_clients.erase(it);
            }
        }

        if (!client)
            client = std::make_unique<tevclient::Client>(host.c_str(), port);

        return client;
    }

    void release_client(std::unique_ptr<tevclient::Client> client)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_clients.push_back(std::move(client));
    }

    static ClientPool& get()
    {
        static ClientPool instance;
        return instance;
    }

private:
    std::mutex m_mutex;
    std::vector<std::unique_ptr<tevclient::Client>> m_clients;
};

#endif // KALI_HAS_TEVCLIENT

bool show_in_tev(const Bitmap* bitmap, std::string name, std::string host, uint16_t port, uint32_t max_retries)
{
    KALI_CHECK_NOT_NULL(bitmap);

#if KALI_HAS_TEVCLIENT
    ref<Bitmap> converted;
    if (bitmap->component_type() != Bitmap::ComponentType::float32) {
        log_warn_once(
            "tev only supports 32-bit floating point images. Converting {} data to float.",
            bitmap->component_type()
        );
        converted = bitmap->convert(bitmap->pixel_format(), Bitmap::ComponentType::float32, false);
        bitmap = converted;
    }

    static std::atomic<uint32_t> image_counter{0};
    if (name.empty())
        name = fmt::format("image_{}", image_counter++);

    const auto& pixel_struct = *bitmap->pixel_struct();
    std::vector<const char*> channel_names(pixel_struct.field_count());
    std::vector<uint64_t> channel_offsets(pixel_struct.field_count());
    std::vector<uint64_t> channel_strides(pixel_struct.field_count());
    for (size_t i = 0; i < channel_names.size(); ++i) {
        channel_names[i] = pixel_struct[i].name.c_str();
        channel_offsets[i] = pixel_struct[i].offset / sizeof(float);
        channel_strides[i] = pixel_struct.size() / sizeof(float);
    }

    for (uint32_t attempt = 1; attempt <= max_retries; ++attempt) {
        std::unique_ptr<tevclient::Client> client = ClientPool::get().acquire_client(host, port);

        if (!client->isConnected()) {
            if (client->connect() != tevclient::Error::Ok) {
                log_warn(
                    "Failed to connect to tev (attempt {}/{}): {}",
                    attempt,
                    max_retries,
                    client->lastErrorString()
                );
                continue;
            }
        }

        if (client->createImage(
                name.c_str(),
                bitmap->width(),
                bitmap->height(),
                bitmap->channel_count(),
                channel_names.data()
            )
            != tevclient::Error::Ok) {
            log_warn(
                "Failed to create image in tev (attempt {}/{}): {}",
                attempt,
                max_retries,
                client->lastErrorString()
            );
            continue;
        }

        if (client->updateImage(
                name.c_str(),
                0,
                0,
                bitmap->width(),
                bitmap->height(),
                bitmap->channel_count(),
                channel_names.data(),
                channel_offsets.data(),
                channel_strides.data(),
                (const float*)bitmap->data(),
                bitmap->buffer_size() / 4
            )
            != tevclient::Error::Ok) {
            log_warn(
                "Failed to update image in tev (attempt {}/{}): {}",
                attempt,
                max_retries,
                client->lastErrorString()
            );
            continue;
        }

        ClientPool::get().release_client(std::move(client));
    }

    return true;

#else
    KALI_UNUSED(bitmap, name, host, port);
    KALI_THROW("tev support is not enabled.");
#endif
}

bool show_in_tev(const Texture* texture, std::string name, std::string host, uint16_t port, uint32_t max_retries)
{
    KALI_CHECK_NOT_NULL(texture);

    ref<Bitmap> bitmap = texture->to_bitmap();
    return show_in_tev(bitmap, name, host, port, max_retries);
}

void show_in_tev_async(const Bitmap* bitmap, std::string name, std::string host, uint16_t port, uint32_t max_retries)
{
    KALI_CHECK_NOT_NULL(bitmap);

    bitmap->inc_ref();

    thread::do_async(
        [=]()
        {
            static std::counting_semaphore semaphore{8};
            semaphore.acquire();
            show_in_tev(bitmap, name, host, port, max_retries);
            semaphore.release();
            bitmap->dec_ref();
        }
    );
}

void show_in_tev_async(const Texture* texture, std::string name, std::string host, uint16_t port, uint32_t max_retries)
{
    KALI_CHECK_NOT_NULL(texture);

    ref<Bitmap> bitmap = texture->to_bitmap();
    return show_in_tev_async(bitmap, name, host, port, max_retries);
}

} // namespace kali::utils
